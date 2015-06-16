win32_multitouch_test
====
Win32でWM_TOUCHを取り扱うメモ。

参考
----
  * Windows タッチ メッセージ - はじめに
    * https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd371581(v=vs.85).aspx

  * 複数の接触点の検出と追跡
    * https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd744775(v=vs.85).aspx

  * TOUCHINPUT 構造体
    * https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd317334(v=vs.85).aspx

ポイント
----
(1) マルチタッチ検出を行う場合は、RegisterTouchWindow()関数を使ってWM_TOUCHを受けるように設定


    if (GetSystemMetrics(SM_DIGITIZER) & NID_MULTI_INPUT) {
        RegisterTouchWindow(hwnd, 0);
        enableMultiTouch = TRUE;
    }


(2) タッチ操作を行った場合、ウインドウメッセージWM_TOUCHが送られてくる。


(3) GetTouchInputInfo()を使ってマルチタッチ情報取得。終わったらCloseTouchInputHandle()でハンドルを閉じること。

    UINT touchNum = LOWORD(wParam);
	if (touchNum > 0) {
        PTOUCHINPUT pTouchInput = new TOUCHINPUT[touchNum];
        HTOUCHINPUT	hTouchInput = (HTOUCHINPUT)lParam;

        GetTouchInputInfo(hTouchInput, touchNum, pTouchInput, sizeof(TOUCHINPUT));
            .
            .
            .
        delete[] pTouchInput;
        pTouchInput = NULL;
        CloseTouchInputHandle(hTouchInput);


(4) TOUCHINPUT構造体に格納されているデジタイザの座標は、スクリーン座標に変換しておくこと。

    TOUCHINPUT ti = pTouchInput[i];

    POINT pt;
    pt.x = TOUCH_COORD_TO_PIXEL(ti.x); // 単位の変換。TOUCHINPUTで用いられている単位はピクセル座標の1/100の値
    pt.y = TOUCH_COORD_TO_PIXEL(ti.y);
    ScreenToClient(hwnd, &pt);         // デスクトップ座標->ウインドウローカル座標

    int	w = TOUCH_COORD_TO_PIXEL(ti.cxContact); //cxContact, cyContactは指が触れている部分の大きさを表している
    int h = TOUCH_COORD_TO_PIXEL(ti.cyContact);

(5) DOWN, MOVE, TOUCHなど、イベントの種類はTOUCHINPUT構造体のdwFlagsに格納されている

    TOUCHINPUT ti = pTouchInput[i];

    if (ti.dwFlags & TOUCHEVENTF_DOWN) {
    	// タッチ開始。
    }
    else if (ti.dwFlags & TOUCHEVENTF_MOVE) {
    	// タッチ部分の移動
    }
    else if (ti.dwFlags & TOUCHEVENTF_UP) {
    	// タッチ終了。指を離した場合
    }
    else {
    }

(6) TOUCHINPUT構造体のdwIDに追跡を行うために必要なIDが割り振られる

    * dwIDに割り振られる数値はOS全体でインクリメントされている様子。
