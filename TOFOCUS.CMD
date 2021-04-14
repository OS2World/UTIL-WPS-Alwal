/* tofocus.cmd ... REXXマクロ機能サンプル

	機能：フォーカスウィンドウの真上に、マウスカーソルを移動させる。
	引数：無し。
*/
IF \ArxQueryWindowInfo('FOCUS','focus') THEN EXIT 1
CALL ArxSetPointerPos focus.GX+focus.CX/2,focus.GY+focus.CY/2
EXIT 0
