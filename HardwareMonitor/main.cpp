#define ID_TIMER1 666

#include <Windows.h>
#include <string>
#include <cliext/adapter>
#include <cliext/algorithm>
#include <cliext/map>
#include <msclr\marshal_cppstd.h>
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")
#include <gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")
using namespace Gdiplus;
using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace TemperatureChecker;
using namespace OpenHardwareMonitor;

HINSTANCE hInst;
LPCTSTR szWindowClass = "MyClass";
LPCTSTR szTitle = "Title";
HWND hWnd;
NOTIFYICONDATA TrayIconData;

ref class ManagedGlobals {
public:
	static Temperature ^temperature = gcnew Temperature();
	System::Collections::Generic::IReadOnlyDictionary<System::String ^, float> ^buffTemperature = gcnew System::Collections::Generic::Dictionary<System::String ^, float>();
	System::Collections::Generic::IReadOnlyDictionary<System::String ^, float> ^buffLoad = gcnew System::Collections::Generic::Dictionary<System::String ^, float>();
};

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
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
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
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

	ShowWindow(hWnd, SW_MINIMIZE);
	UpdateWindow(hWnd);

	return TRUE;
}

HICON CreateSmallIcon(HWND hWnd, HDC hdc, std::string text)
{
	HDC hdcMem;
	HBITMAP hBitmap = NULL;
	HBITMAP hOldBitMap = NULL;
	HBITMAP hBitmapMask = NULL;
	ICONINFO iconInfo;
	HFONT hFont;
	HICON hIcon;

	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, 128, 128);
	hBitmapMask = CreateCompatibleBitmap(hdc, 128, 128);
	ReleaseDC(hWnd, hdc);
	hOldBitMap = (HBITMAP)SelectObject(hdcMem, hBitmap);
	PatBlt(hdcMem, 0, 0, 128, 128, WHITENESS);

	Graphics graphics(hdcMem);
	FontFamily  fontFamily(L"Bahnschrift SemiBold SemiConden");
	Font        font(&fontFamily, 120, FontStyleBold, UnitPixel);
	SolidBrush  solidBrush(Color(0, 0, 0));
	Pen pen(Color(255, 0, 0, 0));
	PointF pointF = { -16, 10 };
	std::wstring wtext = std::wstring(text.begin(), text.end());
	graphics.DrawString(wtext.c_str(), text.size(), &font, pointF, &solidBrush);

	SelectObject(hdc, hOldBitMap);
	hOldBitMap = NULL;

	iconInfo.fIcon = TRUE;
	iconInfo.hbmMask = hBitmapMask;
	iconInfo.hbmColor = hBitmap;

	hIcon = CreateIconIndirect(&iconInfo);

	DeleteDC(hdcMem);
	DeleteDC(hdc);
	DeleteObject(hBitmap);
	DeleteObject(hBitmapMask);

	return hIcon;
}

HRESULT SaveIcon(HICON hIcon, std::string path) {
	// Create the IPicture intrface
	PICTDESC desc = { sizeof(PICTDESC) };
	desc.picType = PICTYPE_ICON;
	desc.icon.hicon = hIcon;
	IPicture* pPicture = 0;
	HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
	if (FAILED(hr)) return hr;

	// Create a stream and save the image
	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);
	LONG cbSize = 0;
	hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

	// Write the stream content to the file
	if (!FAILED(hr)) {
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);
		void* buffer = GlobalLock(hBuf);
		HANDLE hFile = CreateFile((LPCSTR)path.data(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
		else {
			DWORD written = 0;
			WriteFile(hFile, buffer, cbSize, &written, 0);
			CloseHandle(hFile);
		}
		GlobalUnlock(buffer);
	}
	// Cleanup
	pStream->Release();
	pPicture->Release();
	return hr;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT rt;

	switch (message)
	{
	case WM_CREATE: {
		SetTimer(hWnd, ID_TIMER1, 1000, NULL);
		TrayIconData.hWnd = hWnd;
		TrayIconData.uFlags = NIF_ICON | NIF_REALTIME | NIF_MESSAGE;
		TrayIconData.uCallbackMessage = WM_APP;
		TrayIconData.hIcon = CreateSmallIcon(hWnd, GetDC(hWnd), "66");
		Shell_NotifyIcon(NIM_ADD, &TrayIconData);
		//ShowWindow(hWnd, SW_MINIMIZE);
	}	
		break;

	case WM_TIMER: {
		case ID_TIMER1: {
			ManagedGlobals mg;
			mg.buffTemperature = gcnew System::Collections::Generic::Dictionary<System::String ^, float>();
			mg.buffLoad = gcnew System::Collections::Generic::Dictionary<System::String ^, float>();
			mg.buffTemperature = ManagedGlobals::temperature->GetTemperaturesInCelsius();
			mg.buffLoad = ManagedGlobals::temperature->GetLoadInPercents();
			TrayIconData.hIcon = CreateSmallIcon(hWnd, GetDC(hWnd), 
				msclr::interop::marshal_as<std::string>(mg.buffTemperature["CPU Package"].ToString())); //CPU Total
			Shell_NotifyIcon(NIM_MODIFY, &TrayIconData);
		}
		break;
	}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		GetClientRect(hWnd, &rt);
		hdc = BeginPaint(hWnd, &ps);
		//HICON myHicon = CreateSmallIcon(hWnd, hdc);
		//TrayIconData.hIcon = myHicon;
		//Shell_NotifyIcon(NIM_MODIFY, &TrayIconData);
		//SaveIcon(myHicon, "E:\\MY_ICO.ico");
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		KillTimer(hWnd, ID_TIMER1);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}