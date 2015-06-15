//
// win32_multitouch_test.cpp - Win32�Ń}���`�^�b�`�̌��o�e�X�g
//
// �Q�l :
//    Windows �^�b�` ���b�Z�[�W - �͂��߂�
//    https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd371581(v=vs.85).aspx
//
//    �����̐ڐG�_�̌��o�ƒǐ�
//    https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd744775(v=vs.85).aspx
//
//    TOUCHINPUT �\����
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
	// point3 : GetTouchInputInfo()���g���ă}���`�^�b�`���擾�B
	//          �I�������CloseTouchInputHandle()�Ńn���h������邱�ƁB
	//
	UINT touchNum = LOWORD(wParam);
	if (touchNum > 0) {
		PTOUCHINPUT pTouchInput = new TOUCHINPUT[touchNum];
		HTOUCHINPUT	hTouchInput = (HTOUCHINPUT)lParam;

		GetTouchInputInfo(hTouchInput, touchNum, pTouchInput, sizeof(TOUCHINPUT));

		for (UINT i = 0; i < touchNum; i++){
			TOUCHINPUT ti = pTouchInput[i];

			// 
			// point4: �f�W�^�C�U�̍��W����X�N���[�����W�ɕϊ����邱�ƁB
			//
			POINT pt;
			pt.x = TOUCH_COORD_TO_PIXEL(ti.x); // �P�ʂ̕ϊ��BTOUCHINPUT�ŗp�����Ă���P�ʂ̓s�N�Z�����W��1/100�̒l
			pt.y = TOUCH_COORD_TO_PIXEL(ti.y);
			ScreenToClient(hwnd, &pt);  // �f�X�N�g�b�v���W->�E�C���h�E���[�J�����W

			int	w = TOUCH_COORD_TO_PIXEL(ti.cxContact);
			int h = TOUCH_COORD_TO_PIXEL(ti.cyContact);

			//
			// point5 : �C�x���g�̎�ނ�dwFlags�Ɋi�[����Ă���
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
		// point2 : �^�b�`������s�����ꍇ�A�E�C���h�E���b�Z�[�WWM_TOUCH�������Ă���B
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
	// point1 : �}���`�^�b�`���o���s���ꍇ�́ARegisterTouchWindow()�֐����g����WM_TOUCH���󂯂�悤�ɐݒ�
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