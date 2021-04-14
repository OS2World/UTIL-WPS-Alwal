/* ptrmove.cmd ... REXXマクロ機能サンプル

	機能：引数で指定された方向及び幅へ、マウスカーソルを移動させる。
	引数：方向（上下左右）に対応したキャラクタ（U,D,L,R）、移動幅
	　　　（引数の区切りには空白文字を用いる）
	　　　移動幅の省略時値は 1 。また、移動幅に 0 未満の値を指定す
	　　　ると、画面端まで移動する。
*/

/* 現在のマウスカーソル位置を取得 */
PARSE VALUE ArxQueryPointerPos() WITH ptr.x ptr.y

/* パラメータにて指定された方向及び移動幅の参照 */
PARSE UPPER ARG direct pase

/* 移動幅指定無し → 1pixel */
IF pase='' THEN pase = 1

/* 新しいマウスカーソル位置を算出 */
IF pase<0 THEN DO
	/* 移動幅指定が負の値 → 画面端まで移動 */
	PARSE VALUE ArxQueryScreenSize() WITH scr.cx scr.cy
	SELECT
		WHEN direct='U' THEN ptr.y = scr.cy - 1 /*↑*/
		WHEN direct='D' THEN ptr.y = 0 /*↓*/
		WHEN direct='L' THEN ptr.x = 0 /*←*/
		WHEN direct='R' THEN ptr.x = scr.cx - 1 /*→*/
		OTHERWISE NOP
	END
END
ELSE DO
	/* 移動幅指定が正の値 → 指定幅だけ移動 */
	SELECT
		WHEN direct='U' THEN ptr.y = ptr.y + pase /*↑*/
		WHEN direct='D' THEN ptr.y = ptr.y - pase /*↓*/
		WHEN direct='L' THEN ptr.x = ptr.x - pase /*←*/
		WHEN direct='R' THEN ptr.x = ptr.x + pase /*→*/
		OTHERWISE NOP
	END
END

/* マウスカーソル位置を更新 */
CALL ArxSetPointerPos ptr.x,ptr.y

EXIT 0
