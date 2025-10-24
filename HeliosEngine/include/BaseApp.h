#pragma once
// ../include/
#include "../include/Prerequisites.h"
#include "Types.h"
#include "../include/Window.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/SwapChain.h"
#include "../include/Texture.h"
#include "../include/RenderTargetView.h"
#include "../include/DepthStencilView.h"
#include "../include/Viewport.h"
#include "../include/ShaderProgram.h"
#include "../include/MeshComponent.h"
#include "../include/Buffer.h"
#include "../include/SamplerState.h"

class BaseApp {
public:
	BaseApp(HINSTANCE hInst, int nCmdShow);
	~BaseApp() { destroy(); }

	int     run(HINSTANCE hInst, int nCmdShow);
	HRESULT init();
	void    update(float deltaTime);
	void    render();
	void    destroy();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Window           m_window;
	Device           m_device;
	DeviceContext    m_deviceContext;
	SwapChain        m_swapChain;
	Texture          m_backBuffer;
	RenderTargetView m_renderTargetView;
	Texture          m_depthStencil;
	DepthStencilView m_depthStencilView;
	Viewport         m_viewport;
	ShaderProgram    m_shaderProgram;
	MeshComponent    m_mesh;
	Buffer           m_vertexBuffer;
	Buffer           m_indexBuffer;
	Buffer           m_cbNeverChanges;
	Buffer           m_cbChangeOnResize;
	Buffer           m_cbChangesEveryFrame;
	Texture          m_textureCube;
	SamplerState     m_samplerState;

	DirectX::XMMATRIX m_World;
	DirectX::XMMATRIX m_View;
	DirectX::XMMATRIX m_Projection;
	DirectX::XMFLOAT4 m_vMeshColor;

	CBNeverChanges     cbNeverChanges;
	CBChangeOnResize   cbChangesOnResize;
	CBChangesEveryFrame cb;
};
