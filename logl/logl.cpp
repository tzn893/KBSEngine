// logl.cpp : 定义应用程序的入口点。
//
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif

#include "framework.h"
#include "logl.h"
#include "../loglcore/graphic.h"
#include "Application.h"
#include "InputBuffer.h"
#include "Timer.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

int height = 600, width = 800;
//int height = 960, width = 1280;
bool Quit = false;
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LOGL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOGL));

    MSG msg;

    // 主消息循环:
    while (!Quit)
    {
        if (PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
		}else{
			gTimer.tick();

			gGraphic.begin();
			gApp.update();
			gGraphic.end();

			gInput.update();
		}
    }
	gApp.finalize();
	gGraphic.finalize();

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LOGL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, L"Linux 作业", WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME),
	0,0, width, height, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	if (!gGraphic.initialize(hWnd,width,height)) {
		return FALSE;
	}
	if (!gApp.initialize()) {
		return FALSE;
	}
	if (!gTimer.initialize()) {
		return FALSE;
	}
	gInput.initialize();
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
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
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
		Quit = true;
        break;
#ifndef GET_X_FROM_LPARAM
#define GET_X_FROM_LPARAM(lParam) lParam & 0xffff
#endif
#ifndef GET_Y_FROM_LPARAM
#define GET_Y_FROM_LPARAM(lParam) lParam >> 16
#endif
	case WM_LBUTTONDOWN:
		gInput.BufferWriteKeyDown(InputBuffer::KeyCode::MOUSE_LEFT);
		break;
	case WM_RBUTTONDOWN:
		gInput.BufferWriteKeyDown(InputBuffer::KeyCode::MOUSE_RIGHT);
		break;
	case WM_MBUTTONDOWN:
		gInput.BufferWriteKeyDown(InputBuffer::KeyCode::MOUSE_MIDDLE);
		break;
	case WM_MBUTTONUP:
		gInput.BufferWriteKeyUp(InputBuffer::KeyCode::MOUSE_MIDDLE);
		break;
	case WM_LBUTTONUP:
		gInput.BufferWriteKeyUp(InputBuffer::KeyCode::MOUSE_LEFT);
		break;
	case WM_RBUTTONUP:
		gInput.BufferWriteKeyUp(InputBuffer::KeyCode::MOUSE_RIGHT);
		break;
	case WM_MOUSEMOVE:
		gInput.BufferWriteMousePosition(GET_X_FROM_LPARAM(lParam), GET_Y_FROM_LPARAM(lParam));
		break;
	case WM_KEYDOWN: 
		{
			int keycode = wParam - 0x41;
			if (keycode >= 0 && keycode < 26) {
				gInput.BufferWriteKeyDown((InputBuffer::KeyCode)keycode);
			}
		}
		break;
	case WM_KEYUP:
		{
			int keycode = wParam - 0x41;
			if (keycode >= 0 && keycode < 26) {
				gInput.BufferWriteKeyUp((InputBuffer::KeyCode)keycode);
			}
		}
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
