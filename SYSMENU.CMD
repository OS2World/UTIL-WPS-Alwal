/* sysmenu.cmd ... REXXマクロ機能サンプル

	機能：システムメニューアイコンを模擬的にクリックし、システム
	　　　メニューをポップアップさせる。
	引数：無し。
*/

/* ウィンドウの情報を取得する */
IF \ArxQueryWindowInfo('MAIN','main') THEN EXIT 0
IF \ArxQueryWindowInfo('SYSMENU','menu') THEN EXIT 0

/* 現在のマウスカーソル位置を取得 */
PARSE VALUE ArxQueryPointerPos() WITH ox oy

/* クリックする位置を計算する...画面外の場合、異常終了 */
px = main.X+menu.X
py = main.Y+main.CY-menu.CY
PARSE VALUE ArxQueryScreenSize() WITH sx sy
IF px<0 | px>=sx | py<0 | py>=sy THEN EXIT 1

/* クリック操作前に、メインウィンドウを最前列に表示する */
CALL ArxSetWindowPos 'MAIN',,,,,'TOP'

/* マウスカーソルをシステムメニューの上に移動し、クリック動作 */
CALL ArxSetPointerPos px,py
CALL ArxPutMouseButton 1

/* マウスカーソル位置を復元 */
CALL ArxSetPointerPos ox,oy

EXIT 0
