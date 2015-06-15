//
// win32_multitouch_test.cpp - Win32でマルチタッチの検出テスト
//
// 参考 :
//    Windows タッチ メッセージ - はじめに
//    https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd371581(v=vs.85).aspx
//
//    複数の接触点の検出と追跡
//    https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd744775(v=vs.85).aspx
//
//    TOUCHINPUT 構造体
//    https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd317334(v=vs.85).aspx
//
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"

#include <vector>
 
HWND hwnd = NULL;
BOOL enableMultiTouch = FALSE;

class TouchInput {
public:
	int x, y, w, h;

	TouchInput() : x(0), y(0), w(x), h(h) {}
	TouchInput(const int &x, const int &y, const int &w, const int &h) : x(x), y(y), w(w), h(h) {};
	TouchInput(const TouchInput &t) : x(t.x), y(t.y), w(t.w), h(t.h) {};
};

std::vector<TouchInput> touch_inputs;

void UpdateTouchStatus(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	touch_inputs.clear();

	//
	// point3 : GetTouchInputInfo()を使ってマルチタッチ情報取得。
	//          終わったらCloseTouchInputHandle()でハンドルを閉じること。
	//
	UINT touchNum = LOWORD(wParam);
	if (touchNum > 0) {
		PTOUCHINPUT pTouchInput = new TOUCHINPUT[touchNum];
		HTOUCHINPUT	hTouchInput = (HTOUCHINPUT)lParam;

		GetTouchInputInfo(hTouchInput, touchNum, pTouchInput, sizeof(TOUCHINPUT));

		for (UINT i = 0; i < touchNum; i++){
			TOUCHINPUT ti = pTouchInput[i];

			// 
			// point4: デジタイザの座標からスクリーン座標に変換すること。
			//
			POINT pt;
			pt.x = TOUCH_COORD_TO_PIXEL(ti.x); // 単位の変換。TOUCHINPUTで用いられている単位はピクセル座標の1/100の値
			pt.y = TOUCH_COORD_TO_PIXEL(ti.y);
			ScreenToClient(hwnd, &pt);  // デスクトップ座標->ウインドウローカル座標

			int	w = TOUCH_COORD_TO_PIXEL(ti.cxContact);
			int h = TOUCH_COORD_TO_PIXEL(ti.cyContact);

			//
			// point5 : イベントの種類はdwFlagsに格納されている
			//
			if (ti.dwFlags & TOUCHEVENTF_UP) {
				// noting to do...
			}
			else {
				touch_inputs.push_back(TouchInput(pt.x, pt.y, w, h));
			}
		}

		delete[] pTouchInput;
		pTouchInput = NULL;
		CloseTouchInputHandle(hTouchInput);
	}
}

void MyDraw(HDC hdc)
{
	HPEN pen, oldpen;
	HBRUSH brush, oldbrush;

	if (enableMultiTouch == FALSE) {
		char *msg = "This environment does not support multi-touch...";
		TextOut(hdc, 20, 20, msg, strlen(msg));
		return;
	}

	// prepare pen & brush
	pen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
	brush = CreateSolidBrush(RGB(255, 0, 255));

	oldpen = (HPEN)SelectObject(hdc, pen);
	oldbrush = (HBRUSH)SelectObject(hdc, brush);

	std::vector<TouchInput>::const_iterator it;
	for (it = touch_inputs.begin(); it != touch_inputs.end(); ++it) {
		int l = it->x - it->w;
		int t = it->y - it->h;
		int r = it->x + it->w;
		int b = it->y + it->h;
		Ellipse(hdc, l, t, r, b);
	}

	// delete pen & brush
	SelectObject(hdc, oldpen);
	SelectObject(hdc, oldbrush);
	DeleteObject(pen);
	DeleteObject(brush);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		MyDraw(hdc);
		EndPaint(hwnd, &ps);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		hwnd = NULL;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE) {
			PostMessage(hwnd, WM_CLOSE, NULL, NULL);
		}
		break;
	case WM_TOUCH:
		//
		// point2 : タッチ操作を行った場合、ウインドウメッセージWM_TOUCHが送られてくる。
		//
		UpdateTouchStatus(hwnd, wParam, lParam);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = "dummyclassname";
	RegisterClassEx(&wc);

	hwnd = CreateWindow(wc.lpszClassName, "win32_multitouch_test", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, NULL, NULL);

	// 
	// point1 : マルチタッチ検出を行う場合は、RegisterTouchWindow()関数を使ってWM_TOUCHを受けるように設定
	//
	if (GetSystemMetrics(SM_DIGITIZER) & NID_MULTI_INPUT) {
		RegisterTouchWindow(hwnd, 0);
		enableMultiTouch = TRUE;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}