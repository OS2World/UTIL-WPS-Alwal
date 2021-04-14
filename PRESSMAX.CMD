/* pressmax.cmd ... REXXマクロ機能サンプル

	機能：ウィンドウの横幅を左右から32ピクセルずつ縮小する。
	引数：無し。
	備考：「ウィンドウ制御：REXXマクロ」機能の「WM_MINMAXFRAME(MAX)」に割り当
	　　　てて使用すると、ウィンドウ最大化位置/サイズを制限する事ができます。
*/
CALL ArxQueryWindowInfo 'main','info'
IF info.cx>64 THEN DO
	CALL ArxSetWindowPos 'main',info.x+32,,info.cx-64
END
EXIT 0
