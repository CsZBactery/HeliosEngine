#include "../include/BaseApp.h"
#include <vector>

// ================= HLSL embebido (checker procedural) =================
// Nota: shaders VS/PS en string para compilar en runtime (sin archivos .hlsl).
static const char* kHlslSource = R"(
cbuffer CBNeverChanges      : register(b0) { float4x4 gView; }
cbuffer CBChangeOnResize    : register(b1) { float4x4 gProj; }
cbuffer CBChangesEveryFrame : register(b2) { float4x4 gWorld; float4 vMeshColor; }

Texture2D gTxDiffuse : register(t0);
SamplerState gSamLinear : register(s0);

struct VS_IN { float3 Pos:POSITION; float2 Tex:TEXCOORD0; };
struct VS_OUT{ float4 Pos:SV_POSITION; float2 Tex:TEXCOORD0; };

VS_OUT VS(VS_IN i){
    VS_OUT o;
    float4 w = mul(float4(i.Pos,1), gWorld);
    float4 v = mul(w, gView);
    o.Pos    = mul(v, gProj);
    o.Tex    = i.Tex;
    return o;
}

float4 PS(VS_OUT i):SV_Target{
    // Checker procedural por si no hay textura ligada
    float2 uv = i.Tex * 8.0;
    float  c  = (fmod(floor(uv.x)+floor(uv.y),2.0) < 1.0) ? 0.2 : 1.0;
    float4 checker = float4(c,c,c,1);

    // Intento de samplear textura (si no hay SRV, el checker manda)
    float4 texel = gTxDiffuse.Sample(gSamLinear, i.Tex);
    float useChecker = step(texel.r+texel.g+texel.b+texel.a, 0.0001);

    return lerp(texel, checker, useChecker) * vMeshColor;
}
)";
// ======================================================================

// Ctor: se reciben hInst/nCmdShow (por ahora sin trabajo; init ocurre en run()).
BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {}

// Loop principal de la app (estilo wWinMain pero como método de clase).
int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // Crear ventana Win32 + WndProc
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
        return 0;
    }
    // Inicializar pipeline y recursos DX11
    if (FAILED(init()))
        return 0;

    // Bucle de mensajes + timing de frame (deltaTime vía QPC)
    MSG msg = { };
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (WM_QUIT != msg.message)
    {
        // Procesar mensajes pendentes (no bloquear)
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Calcular deltaTime y avanzar frame
            LARGE_INTEGER curr;
            QueryPerformanceCounter(&curr);
            float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;
            update(deltaTime);
            render();
        }
    }

    return (int)msg.wParam;
}

// Inicialización del motor gráfico y recursos (equivalente a InitDevice).
HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // 1) Crear SwapChain (esto crea Device + Context y obtiene backBuffer).
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize SwapChain.");
        return hr;
    }

    // 2) RTV para el back-buffer (detecta MSAA y usa TEXTURE2DMS si aplica).
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize RenderTargetView.");
        return hr;
    }

    // Tomar descripción del back-buffer para igualar MSAA en el depth.
    D3D11_TEXTURE2D_DESC bbDesc{};
    m_backBuffer.m_texture->GetDesc(&bbDesc);

    // 3) Crear textura de Depth/Stencil con mismo SampleDesc que el back-buffer.
    hr = m_depthStencil.init(
        m_device,
        m_window.m_width,
        m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        bbDesc.SampleDesc.Count,
        bbDesc.SampleDesc.Quality
    );
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize DepthStencil Texture.");
        return hr;
    }

    // 4) Crear DSV (elige TEXTURE2DMS si Count>1).
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize DepthStencilView.");
        return hr;
    }

    // 5) Configurar viewport según tamaño de ventana.
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize Viewport.");
        return hr;
    }

    // 6) Descripción de InputLayout (POSITION + TEXCOORD).
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    {
        D3D11_INPUT_ELEMENT_DESC position{};
        position.SemanticName = "POSITION";
        position.SemanticIndex = 0;
        position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        position.InputSlot = 0;
        position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        position.InstanceDataStepRate = 0;
        Layout.push_back(position);

        D3D11_INPUT_ELEMENT_DESC texcoord{};
        texcoord.SemanticName = "TEXCOORD";
        texcoord.SemanticIndex = 0;
        texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
        texcoord.InputSlot = 0;
        texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        texcoord.InstanceDataStepRate = 0;
        Layout.push_back(texcoord);
    }

    // 7) Compilar y crear shaders desde el HLSL embebido.
    hr = m_shaderProgram.initFromSource(m_device, kHlslSource, Layout);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 8) Geometría: cubo (24 vértices, 36 índices).
    {
        SimpleVertex vertices[] =
        {
            { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
        };

        unsigned int indices[] =
        {
            3,1,0,  2,1,3,
            6,4,5,  7,4,6,
            11,9,8, 10,9,11,
            14,12,13, 15,12,14,
            19,17,16, 18,17,19,
            22,20,21, 23,20,22
        };

        // Copiar datos al MeshComponent (arreglos -> vectores)
        m_mesh.m_vertex.clear();
        m_mesh.m_index.clear();
        for (unsigned int i = 0; i < 24; ++i) m_mesh.m_vertex.push_back(vertices[i]);
        m_mesh.m_numVertex = 24;
        for (unsigned int i = 0; i < 36; ++i) m_mesh.m_index.push_back(indices[i]);
        m_mesh.m_numIndex = 36;
    }

    // 9) Crear VertexBuffer e IndexBuffer desde MeshComponent.
    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize VertexBuffer.");
        return hr;
    }

    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize IndexBuffer.");
        return hr;
    }

    // Configurar topología de primitivas (triángulos).
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 10) Constant Buffers (view, proj, world + color).
    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBNeverChanges."); return hr; }

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBChangeOnResize."); return hr; }

    hr = m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBChangesEveryFrame."); return hr; }

    // 11) Sampler lineal (aunque sin textura: el PS usa checker por defecto).
    hr = m_samplerState.init(m_device);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize SamplerState.");
        return hr;
    }

    // 12) Inicializar matrices (World, View, Proj)
    m_World = XMMatrixIdentity();

    XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // Cargar CBs iniciales
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    return S_OK;
}

// Lógica por frame: animación, matrices y actualización de constant buffers.
void BaseApp::update(float deltaTime) {
    static float t = 0.0f;
    // Avance de tiempo (más lento en driver de referencia)
    if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    }
    else {
        static DWORD t0 = 0;
        DWORD tc = GetTickCount();
        if (t0 == 0) t0 = tc;
        t = (tc - t0) / 1000.0f;
    }

    // Actualizar CB de vista y proyección
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Color animado (RGB con distintas frecuencias)
    m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
    m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
    m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

    // World rotando en Y
    m_World = XMMatrixRotationY(t);
    cb.mWorld = XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor;
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

// Render por frame: clear, set states, draw y present.
void BaseApp::render() {
    // Limpiar RTV y setear DSV
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // Aplicar viewport y limpiar depth/stencil
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Configurar shaders e input layout
    m_shaderProgram.render(m_deviceContext);

    // Enlazar buffers de geometría
    m_vertexBuffer.render(m_deviceContext, 0, 1);
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // Enviar constant buffers (VS y PS)
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true); // PS

    // Sampler (si hubiera SRV, el PS la usaría; si no, checker)
    m_samplerState.render(m_deviceContext, 0, 1);

    // Dibujar malla indexada
    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

    // Presentar en pantalla
    m_swapChain.present();
}

// Liberación ordenada de recursos (inverso a la creación).
void BaseApp::destroy() {
    // Limpia estados de la device context para soltar referencias internas
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_samplerState.destroy();
    // m_textureCube.destroy(); // No se usa textura en este ejemplo

    m_cbNeverChanges.destroy();
    m_cbChangeOnResize.destroy();
    m_cbChangesEveryFrame.destroy();

    m_vertexBuffer.destroy();
    m_indexBuffer.destroy();
    m_shaderProgram.destroy();

    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();

    m_swapChain.destroy();
    m_backBuffer.destroy();
    m_deviceContext.destroy();
    m_device.destroy();
}

// WndProc: gestiona mensajes básicos de la ventana (create/paint/destroy).
LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_CREATE:
    {
        // Guardar puntero de usuario si fuera necesario (no se usa después).
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    }
    return 0;
    case WM_PAINT:
    {
        // Pintado de ventana (no hacemos GDI; DX se encarga).
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_DESTROY:
        // Salir del loop principal
        PostQuitMessage(0);
        return 0;
    }
    // Mensajes no manejados: por defecto Win32
    return DefWindowProc(hWnd, message, wParam, lParam);
}
