#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Camera3D.h"
#include "Camera2D.h"
#include "..\\Timer.h"
#include "ImGUI\\imgui.h"
#include "ImGUI\\imgui_impl_win32.h"
#include "ImGUI\\imgui_impl_dx11.h"
#include "RenderableGameObject.h"
#include "Light.h"
#include "Sprite.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void RenderFrame();
	Camera3D camera3D;
	Camera2D camera2D;
	RenderableGameObject floor;
	RenderableGameObject monkeySmooth;
	Light pointLightOne;
	Sprite sprite;

private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
	void RenderShadowMap();

	std::vector<IDXGIAdapter*> EnumerateAdapters();
	
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

	VertexShader vertexshader;
	VertexShader vertexshader_2d;

	PixelShader pixelshader;
	PixelShader pixelshader_2d;
	PixelShader pixelshader_2d_discard;
	PixelShader pixelshader_nolight;

	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_VS_vertexshader_2d> cb_vs_vertexshader_2d;

	ConstantBuffer<CB_PS_light> cb_ps_light;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_DrawMask;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_ApplyMask;
	
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullBack;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStateWrap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DefaultTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GrassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TileTexture;

	int windowWidth = 0;
	int windowHeight = 0;

	Timer fpsTimer;
};