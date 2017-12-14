// dx11.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "dx11.h"
#include <dxgi.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

#include "d3dx11.h"

#include <atlstr.h>


/////////////////////////////////////
IDXGIFactory1 *g_pDXGIFactory = NULL;
ID3D11Device *g_pd3d11Device = NULL;
ID3D11DeviceContext *g_pd3dImmediateContext = NULL;
IDXGISwapChain *g_pSwapChain = NULL;
ID3D11VertexShader *g_pVSRenderUI11 = NULL;
ID3D11PixelShader *g_pPSRenderUI11 = NULL;
ID3D11PixelShader *g_pPSRenderUIUntex11 = NULL;
ID3D11DepthStencilState *g_pDepthStencilStateUI11 = NULL;
ID3D11RasterizerState *g_pRasterizerStateUI11 = NULL;
ID3D11BlendState *g_pBlendStateUI11 = NULL;
ID3D11SamplerState *g_pSamplerStateUI11 = NULL;
/////////////////////////////////////

CHAR g_strUIEffectFile [] = \
"Texture2D g_Texture;"\
""\
"SamplerState Sampler"\
"{"\
"    Filter = MIN_MAG_MIP_LINEAR;"\
"    AddressU = Wrap;"\
"    AddressV = Wrap;"\
"};"\
""\
"BlendState UIBlend"\
"{"\
"    AlphaToCoverageEnable = FALSE;"\
"    BlendEnable[0] = TRUE;"\
"    SrcBlend = SRC_ALPHA;"\
"    DestBlend = INV_SRC_ALPHA;"\
"    BlendOp = ADD;"\
"    SrcBlendAlpha = ONE;"\
"    DestBlendAlpha = ZERO;"\
"    BlendOpAlpha = ADD;"\
"    RenderTargetWriteMask[0] = 0x0F;"\
"};"\
""\
"BlendState NoBlending"\
"{"\
"    BlendEnable[0] = FALSE;"\
"    RenderTargetWriteMask[0] = 0x0F;"\
"};"\
""\
"DepthStencilState DisableDepth"\
"{"\
"    DepthEnable = false;"\
"};"\
"DepthStencilState EnableDepth"\
"{"\
"    DepthEnable = true;"\
"};"\
"struct VS_OUTPUT"\
"{"\
"    float4 Pos : POSITION;"\
"    float4 Dif : COLOR;"\
"    float2 Tex : TEXCOORD;"\
"};"\
""\
"VS_OUTPUT VS( float3 vPos : POSITION,"\
"              float4 Dif : COLOR,"\
"              float2 vTexCoord0 : TEXCOORD )"\
"{"\
"    VS_OUTPUT Output;"\
""\
"    Output.Pos = float4( vPos, 1.0f );"\
"    Output.Dif = Dif;"\
"    Output.Tex = vTexCoord0;"\
""\
"    return Output;"\
"}"\
""\
"float4 PS( VS_OUTPUT In ) : SV_Target"\
"{"\
"    return g_Texture.Sample( Sampler, In.Tex ) * In.Dif;"\
"}"\
""\
"float4 PSUntex( VS_OUTPUT In ) : SV_Target"\
"{"\
"    return In.Dif;"\
"}"\
""\
"technique10 RenderUI"\
"{"\
"    pass P0"\
"    {"\
"        SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
"        SetGeometryShader( NULL );"\
"        SetPixelShader( CompileShader( ps_4_0, PS() ) );"\
"        SetDepthStencilState( DisableDepth, 0 );"\
"        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
"    }"\
"}"\
"technique10 RenderUIUntex"\
"{"\
"    pass P0"\
"    {"\
"        SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
"        SetGeometryShader( NULL );"\
"        SetPixelShader( CompileShader( ps_4_0, PSUntex() ) );"\
"        SetDepthStencilState( DisableDepth, 0 );"\
"        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
"    }"\
"}"\
"technique10 RestoreState"\
"{"\
"    pass P0"\
"    {"\
"        SetDepthStencilState( EnableDepth, 0 );"\
"        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
"    }"\
"}";
const UINT              g_uUIEffectFileSize = sizeof(g_strUIEffectFile);
////////////////////////////
void Init(HWND hwnd)
{
	/////////////////////////////////
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (LPVOID*) &g_pDXGIFactory);
	/////////////////////////////////
	D3D_FEATURE_LEVEL DeviceFeatureLevel = D3D_FEATURE_LEVEL_11_0, FeatureLevel;
	hr = D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		(HMODULE) 0,
		0,
		&DeviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&g_pd3d11Device,
		&FeatureLevel,
		&g_pd3dImmediateContext
		);
	///////////////////////////////////////
	D3D11_RASTERIZER_DESC drd = {
		D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
		D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
		FALSE, //BOOL FrontCounterClockwise;
		0, //INT DepthBias;
		0.0f,//FLOAT DepthBiasClamp;
		0.0f,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;        
	};
	ID3D11RasterizerState* pRS = NULL;
	hr = g_pd3d11Device->CreateRasterizerState(&drd, &pRS);
	g_pd3dImmediateContext->RSSetState(pRS);
	//////////////////////////////////////////
	//后备缓冲个数
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	desc.BufferCount = 1;
	//后备缓冲大小
	desc.BufferDesc.Width = 800;
	desc.BufferDesc.Height = 600;
	//像素格式 归一化
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//立即刷新
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	//扫描线设置
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//设为渲染目标缓冲
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//窗口模式
	desc.Windowed = TRUE;
	desc.OutputWindow = hwnd;
	//不采用多重采样
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	//后备缓冲内容呈现屏幕后，放弃其内容
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//不设置标志
	desc.Flags = 0;
	hr = g_pDXGIFactory->CreateSwapChain(g_pd3d11Device, &desc, &g_pSwapChain);
	///////////////////////////////////////////////////////
	// Compile Shaders
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pPSUntexBlob = NULL;
	hr = D3DCompile(g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "VS", "vs_4_0_level_9_1", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pVSBlob, NULL);
	hr = D3DCompile(g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "PS", "ps_4_0_level_9_1", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pPSBlob, NULL);
	hr = D3DCompile(g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "PSUntex", "ps_4_0_level_9_1", D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pPSUntexBlob, NULL);
	// Create Shaders
	hr = g_pd3d11Device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVSRenderUI11);
	hr = g_pd3d11Device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPSRenderUI11);
	hr = g_pd3d11Device->CreatePixelShader(pPSUntexBlob->GetBufferPointer(), pPSUntexBlob->GetBufferSize(), NULL, &g_pPSRenderUIUntex11);
	// States
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = FALSE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	hr = g_pd3d11Device->CreateDepthStencilState(&DSDesc, &g_pDepthStencilStateUI11);
	//
	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.AntialiasedLineEnable = FALSE;
	RSDesc.CullMode = D3D11_CULL_BACK;
	RSDesc.DepthBias = 0;
	RSDesc.DepthBiasClamp = 0.0f;
	RSDesc.DepthClipEnable = TRUE;
	RSDesc.FillMode = D3D11_FILL_SOLID;
	RSDesc.FrontCounterClockwise = FALSE;
	RSDesc.MultisampleEnable = TRUE;
	RSDesc.ScissorEnable = FALSE;
	RSDesc.SlopeScaledDepthBias = 0.0f;
	hr = g_pd3d11Device->CreateRasterizerState(&RSDesc, &g_pRasterizerStateUI11);
	//
	D3D11_BLEND_DESC BSDesc;
	ZeroMemory(&BSDesc, sizeof(D3D11_BLEND_DESC));
	BSDesc.RenderTarget[0].BlendEnable = TRUE;
	BSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	hr = g_pd3d11Device->CreateBlendState(&BSDesc, &g_pBlendStateUI11);
	//
	D3D11_SAMPLER_DESC SSDesc;
	ZeroMemory(&SSDesc, sizeof(D3D11_SAMPLER_DESC));
	SSDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	SSDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SSDesc.MaxAnisotropy = 16;
	SSDesc.MinLOD = 0;
	SSDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (g_pd3d11Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_9_3) 
	{
		SSDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SSDesc.MaxAnisotropy = 0;
	}
	hr = g_pd3d11Device->CreateSamplerState(&SSDesc, &g_pSamplerStateUI11);
	// 加载一张图片
	ID3D11Resource *pRes = NULL;
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	loadInfo.Width = D3DX11_DEFAULT;
	loadInfo.Height = D3DX11_DEFAULT;
	loadInfo.Depth = D3DX11_DEFAULT;
	loadInfo.FirstMipLevel = 0;
	loadInfo.MipLevels = 1;
	loadInfo.Usage = D3D11_USAGE_DEFAULT;
	loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	loadInfo.CpuAccessFlags = 0;
	loadInfo.MiscFlags = 0;
	loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	loadInfo.Filter = D3DX11_FILTER_NONE;
	loadInfo.MipFilter = D3DX11_FILTER_NONE;
	loadInfo.pSrcInfo = NULL;

	// 图片路径
	CString path;
	::GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH), MAX_PATH);
	path = path.Left(path.ReverseFind(L'\\')) + L"\\aa.png";
	//
	hr = D3DX11CreateTextureFromFile(g_pd3d11Device, path, &loadInfo, NULL, &pRes, NULL);
	hr = pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*) ppTexture);
}
/////////////////////////////////////

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[100];					// 标题栏文本
TCHAR szWindowClass[100];			// 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。

	////////////////////////////////
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadString(hInstance, IDC_DX11, szWindowClass, 100);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DX11));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DX11));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DX11);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		Init(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
