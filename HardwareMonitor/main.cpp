#include <Windows.h>
#include <map>
#include <string>

#include <cliext/adapter>
#include <cliext/algorithm>
#include <cliext/map>

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace TemperatureChecker;

HINSTANCE hInst;
LPCTSTR szWindowClass = "MyClass";
LPCTSTR szTitle = "Title";
HWND hWnd;
std::map<std::string, float> tmpDictionary;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_SIZE);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = (LPCSTR)szWindowClass;
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass, szTitle, WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

cliext::map <std::string, float> ^ MyConvert(System::Collections::Generic::IReadOnlyDictionary<System::String ^, float> ^myMap)
{
	cliext::map <std::string, float> ^h_result = gcnew cliext::map <std::string, float>(myMap);
	//cliext::collection_adapter<System::Collections::Generic::IReadOnlyDictionary<System::String ^, float>> dict();
	// iterate the outer dictionary
	//for each (std::pair<System::String ^, float> ^kvp1 in dict)
	//{
	//	std::string stringKey = marshal_as<std::string>(kvp1->Key);
	//	float mapValue = marshal_as<float>(kvp1->Value);
	//	// insert in outer map
	//	h_result.insert(std::make_pair(stringKey, mapValue));
	//}

	return h_result;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT rt;

	switch (message)
	{
	case WM_CREATE:
		Temperature temperature;
		System::Collections::Generic::Dictionary<System::String ^, float> ^buff = 
			gcnew System::Collections::Generic::Dictionary<System::String ^, float>(6);
		MyConvert(buff);	
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		GetClientRect(hWnd, &rt);
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}