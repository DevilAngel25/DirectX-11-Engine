#include "Graphics.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	fpsTimer.Start();

	if (!InitializeDirectX(hwnd)) { return false; }
	if (!InitializeShaders()) { return false; }
	if (!InitializeScene()) { return false; }

	//InIt ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderFrame()
{
	//render final scene
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), bgcolor);

	cb_ps_light.data.lightPos = pointLightOne.GetPositionFloat3();
	cb_ps_light.ApplyChanges();
	cb_vs_vertexshader.ApplyChanges();

	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf());

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetState(rasterizerState_CullBack.Get());
	deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	deviceContext->PSSetSamplers(0, 1, samplerStateWrap.GetAddressOf());

	deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);
	deviceContext->IASetInputLayout(vertexshader.GetInputLayout());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);

#pragma region Draw Models

	//draw Nanosuit one
	{
		monkeySmooth.Draw(camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix());
	}

	//draw Terrain
	{
		floor.Draw(camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix());
	}

#pragma endregion

#pragma region Draw Lights
	//Draw Lights
	{
		deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		pointLightOne.Draw(camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix());
	}
#pragma endregion

#pragma region FPS Counter
	//draw fps text
	static int fpsCounter = 0;
	static std::string fpsString = "FPS: 0";

	fpsCounter += 1;
	if (fpsTimer.GetMillisecondsElapsed() > 1000.0)
	{
		fpsString = "FPS: " + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}

	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), StringHelper::StringToWide(fpsString).c_str(), XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, XMFLOAT2(0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();
#pragma endregion

#pragma region ImGui
	//Start ImGui Frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Window
	ImGui::Begin("Lighting Controls");
	ImGui::DragFloat3("Ambient Color", &cb_ps_light.data.ambientColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Strength", &cb_ps_light.data.ambientStrength, 0.01f, 0.0f, 1.0f);
	ImGui::NewLine();
	ImGui::DragFloat3("Light Color", &cb_ps_light.data.diffuseColor.x, 0.01f, 0.0f, 1.0f); //pointLightOne.lightColor.x
	ImGui::DragFloat("Light Strength", &cb_ps_light.data.lightStrength, 0.01f, 1.0f, 10.0f); //pointLightOne.lightStrength
	ImGui::DragFloat("Light Attenuation A", &cb_ps_light.data.lightAttenuation_a, 0.01f, 0.1f, 1.0f); //pointLightOne.attenuation_a
	ImGui::DragFloat("Light Attenuation B", &cb_ps_light.data.lightAttenuation_b, 0.01f, 0.1f, 1.0f); //pointLightOne.attenuation_b
	ImGui::DragFloat("Light Attenuation C", &cb_ps_light.data.lightAttenuation_c, 0.01f, 0.1f, 1.0f); //pointLightOne.attenuation_c
	ImGui::End();
	//Assamble Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#pragma endregion
	swapchain->Present(1, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	try
	{
		HRESULT hr;
#pragma region GetAdapters

		std::vector<IDXGIAdapter*> pAdapter = EnumerateAdapters();

		if (pAdapter.size() < 1)
		{
			ErrorLogger::Log("No DXGI AAdapters found");
			return false;
		}
#pragma endregion

#pragma region GetDisplayModes

		IDXGIOutput* pOutput = NULL;

		hr = pAdapter[0]->EnumOutputs(0, &pOutput);

		UINT numModes = 0;
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		UINT flags = DXGI_ENUM_MODES_INTERLACED;

		// Get the number of elements
		hr = pOutput->GetDisplayModeList(format, flags, &numModes, NULL);
		COM_ERROR_IF_FAILED(hr, "Failed to get number of display modes.");

		DXGI_MODE_DESC* displayModesDesc = new DXGI_MODE_DESC[numModes];

		hr = pOutput->GetDisplayModeList(format, flags, &numModes, displayModesDesc);
		COM_ERROR_IF_FAILED(hr, "Failed to create displaymode list.");
#pragma endregion

		DXGI_SWAP_CHAIN_DESC swapchainDesc = { 0 };

		swapchainDesc.BufferCount = 1; //double buffering
		swapchainDesc.BufferDesc.Width = windowWidth;
		swapchainDesc.BufferDesc.Height = windowHeight;
		swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.OutputWindow = hwnd;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.Windowed = TRUE;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL FeatureLevel;

		hr = D3D11CreateDeviceAndSwapChain(pAdapter[0], D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &FeatureLevels, 1, D3D11_SDK_VERSION, &swapchainDesc, swapchain.GetAddressOf(), device.GetAddressOf(), &FeatureLevel, deviceContext.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain.");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed.");

		hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

		//describe depth and stencil buffer
		CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, windowWidth, windowHeight);
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = device->CreateTexture2D(&depthStencilDesc, NULL, depthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");

		hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		//create depth stencil state 
		CD3D11_DEPTH_STENCIL_DESC depthStencilStateDesc(D3D11_DEFAULT);
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = device->CreateDepthStencilState(&depthStencilStateDesc, depthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		//create depth stencil state draw mask
		CD3D11_DEPTH_STENCIL_DESC depthStencilStateDesc_DrawMask(D3D11_DEFAULT);
		depthStencilStateDesc_DrawMask.DepthEnable = FALSE;
		depthStencilStateDesc_DrawMask.StencilEnable = TRUE;

		depthStencilStateDesc_DrawMask.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		depthStencilStateDesc_DrawMask.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_DrawMask.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_DrawMask.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		depthStencilStateDesc_DrawMask.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		depthStencilStateDesc_DrawMask.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_DrawMask.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_DrawMask.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR_SAT;

		hr = device->CreateDepthStencilState(&depthStencilStateDesc_DrawMask, depthStencilState_DrawMask.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state for drawing mask.");

		//create stencil state apply mask
		CD3D11_DEPTH_STENCIL_DESC depthStencilStateDesc_ApplyMask(D3D11_DEFAULT);
		depthStencilStateDesc_ApplyMask.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		depthStencilStateDesc_ApplyMask.StencilEnable = TRUE;

		depthStencilStateDesc_ApplyMask.BackFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		depthStencilStateDesc_ApplyMask.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_ApplyMask.BackFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_ApplyMask.BackFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		depthStencilStateDesc_ApplyMask.FrontFace.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		depthStencilStateDesc_ApplyMask.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_ApplyMask.FrontFace.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc_ApplyMask.FrontFace.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;

		hr = device->CreateDepthStencilState(&depthStencilStateDesc_ApplyMask, depthStencilState_ApplyMask.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state for Applying the mask.");

		//create and set viewport
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
		deviceContext->RSSetViewports(1, &viewport);

		//create rasterizer state Cull_Back
		CD3D11_RASTERIZER_DESC rasterizerDesc_CullBack(D3D11_DEFAULT);
		hr = device->CreateRasterizerState(&rasterizerDesc_CullBack, rasterizerState_CullBack.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state Cull_Back.");

		//create rasterizer state Cull_Front
		CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
		rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		hr = device->CreateRasterizerState(&rasterizerDesc_CullFront, rasterizerState_CullFront.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state Cull_Front.");

		//Create blend state
		D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendState = { 0 };
		renderTargetBlendState.BlendEnable = true;
		renderTargetBlendState.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		renderTargetBlendState.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		renderTargetBlendState.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		renderTargetBlendState.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		renderTargetBlendState.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		renderTargetBlendState.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		renderTargetBlendState.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC blendDesc = { 0 };
		blendDesc.RenderTarget[0] = renderTargetBlendState;

		hr = device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		//initialize fonts
		spriteBatch = std::make_unique<SpriteBatch>(deviceContext.Get());
		spriteFont = std::make_unique<SpriteFont>(device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

		//set up sampler state / create sampler desciption for textures
		CD3D11_SAMPLER_DESC samplerDesc(D3D11_DEFAULT);
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;

		hr = device->CreateSamplerState(&samplerDesc, samplerStateWrap.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state Wrap.");

	}
	catch(COMException exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

bool Graphics::InitializeShaders()
{
	std::wstring shaderfolder = L"";

#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG
	#ifdef _WIN64
		shaderfolder = L"..\\x64\\Debug\\";
	#else
		shaderfolder = L"..\\Debug\\";
	#endif
#else
	#ifdef _WIN64
			shaderfolder = L"..\\x64\\Release\\";
	#else
			shaderfolder = L"..\\Release\\";
	#endif
#endif
	}
#pragma endregion

	//3D shaders
	D3D11_INPUT_ELEMENT_DESC layout3D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElements3D = ARRAYSIZE(layout3D);

	if (!vertexshader.Initialize(device, shaderfolder + L"vertexshader.cso", layout3D, numElements3D)) { return false; }
	if (!pixelshader.Initialize(device, shaderfolder + L"pixelshader.cso")) { return false; }
	if (!pixelshader_nolight.Initialize(device, shaderfolder + L"pixelshader_nolight.cso")) { return false; }

	//2D shaders
	D3D11_INPUT_ELEMENT_DESC layout2D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElements2D = ARRAYSIZE(layout2D);

	if (!vertexshader_2d.Initialize(device, shaderfolder + L"vertexshader_2d.cso", layout2D, numElements2D)) { return false; }
	if (!pixelshader_2d.Initialize(device, shaderfolder + L"pixelshader_2d.cso")) { return false; }
	if (!pixelshader_2d_discard.Initialize(device, shaderfolder + L"pixelshader_2d_discard.cso")) { return false; }


	return true;
}

bool Graphics::InitializeScene()
{
	try
	{
		HRESULT hr;

		//load default texture
		/*hr = CreateWICTextureFromFile(device.Get(), L"Data\\Textures\\Default.jpg", nullptr, DefaultTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create default texture from file.");

		//load grass texture
		hr = CreateWICTextureFromFile(device.Get(), L"Data\\Textures\\Grass.jpg", nullptr, GrassTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create grass texture from file.");

		//load tile texture
		hr = CreateWICTextureFromFile(device.Get(), L"Data\\Textures\\Tile.jpg", nullptr, TileTexture.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create tile texture from file.");*/

		//create constant buffer for vertexshader
		hr = cb_vs_vertexshader.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to init vertexshader constant buffer.");

		//create constant buffer for vertexshader_2d
		hr = cb_vs_vertexshader_2d.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to init vertexshader_2d constant buffer.");

#pragma region Init Ambient Light
		//create constant buffer for light one
		hr = cb_ps_light.Initialize(device.Get(), deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to init pixelshader constant buffer for lightone.");

		cb_ps_light.data.ambientColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		cb_ps_light.data.ambientStrength = 1.0f;

		cb_ps_light.data.diffuseColor = pointLightOne.lightColor;
		cb_ps_light.data.lightStrength = pointLightOne.lightStrength;
		cb_ps_light.data.lightAttenuation_a = pointLightOne.attenuation_a;
		cb_ps_light.data.lightAttenuation_b = pointLightOne.attenuation_b;
		cb_ps_light.data.lightAttenuation_c = pointLightOne.attenuation_c;

#pragma endregion

#pragma region Init Models

		if (!floor.Initialize("Data\\Objects\\Samples\\Floor.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
		{
			ErrorLogger::Log("GameObject Not Found!");
			return false;
		}
		floor.SetPosition(0.0f, -1.0f, 0.0f);

		if (!monkeySmooth.Initialize("Data\\Objects\\Samples\\MonkeySmooth.fbx", device.Get(), deviceContext.Get(), cb_vs_vertexshader))
		{
			ErrorLogger::Log("GameObject Not Found!");
			return false;
		}
		monkeySmooth.SetPosition(0.0f, 0.0f, 0.0f);

#pragma endregion

#pragma region Init Lights
		//init light one
		if (!pointLightOne.Initialize(device.Get(), deviceContext.Get(), cb_vs_vertexshader))
		{
			ErrorLogger::Log("Light Not Found!");
			return false;
		}
		pointLightOne.SetPosition(10.0f, 5.0f, 0.0f);

#pragma endregion

		if (!sprite.Initialize(device.Get(), deviceContext.Get(), 256, 256, "Data\\Textures\\circle.png", cb_vs_vertexshader_2d))
		{
			ErrorLogger::Log("Sprite Not Found!");
			return false;
		}

		sprite.SetPosition(XMFLOAT3(windowWidth / 2 - sprite.GetWidth() / 2, windowHeight / 2 - sprite.GetHeight() / 2, 0.0f));

		//set 2d camera valuse
		camera2D.SetProjectionValues(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 1.0f);

		//set 3d camera valuse
		camera3D.SetPosition(0.0f, 10.0f, -20.0f);
		camera3D.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 3000.0f);
		//set projection matrix for light, for shadow mapping
		pointLightOne.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 3000.0f);

	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Graphics::RenderShadowMap()
{

	// Render all the objects in the scene that can cast shadows onto themselves or onto other objects.

	// Only bind the ID3D11DepthStencilView for output.
	/*deviceContext->OMSetRenderTargets(0, nullptr, m_shadowDepthView.Get());

	// Note that starting with the second frame, the previous call will display
	// warnings in VS debug output about forcing an unbind of the pixel shader
	// resource. This warning can be safely ignored when using shadow buffers
	// as demonstrated in this sample.

	// Set rendering state.
	deviceContext->RSSetState(m_shadowRenderState.Get());
	deviceContext->RSSetViewports(1, &m_shadowViewport);

	// Each vertex is one instance of the VertexPositionTexNormColor struct.
	UINT stride = sizeof(VertexPositionTexNormColor);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), 
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	deviceContext->VSSetShader(m_simpleVertexShader.Get(), nullptr, 0);

	// Send the constant buffers to the Graphics device.
	deviceContext->VSSetConstantBuffers(0, 1, m_lightViewProjectionBuffer.GetAddressOf());

	deviceContext->VSSetConstantBuffers(1, 1, m_rotatedModelBuffer.GetAddressOf());

	// In some configurations, it's possible to avoid setting a pixel shader
	// (or set PS to nullptr). Not all drivers are tolerant of this, so to be
	// safe set a minimal shader here.
	//
	// Direct3D will discard output from this shader because the render target
	// view is unbound.
	deviceContext->PSSetShader(m_textureShader.Get(), nullptr, 0 );

	// Draw the objects.
	deviceContext->DrawIndexed(m_indexCountCube, 0, 0);*/
}

std::vector<IDXGIAdapter*> Graphics::EnumerateAdapters()
{
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	IDXGIFactory* pFactory = NULL;

	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
	{
		return vAdapters;
	}

	for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		vAdapters.push_back(pAdapter);
	}

	if (pFactory)
	{
		pFactory->Release();
	}

	return vAdapters;
}
