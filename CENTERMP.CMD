/* centermp.cmd ... REXXマクロ機能サンプル

	機能：他のセッションから ALWALTFX のREXX関数を呼び出すサンプル。
	　　　画面の解像度及びマウスボタン状態を取得し、マウスボタンが
	　　　押し下げられていない場合のみ、画面の中央へマウスポインタ
	　　　を移動する。
	引数：無し。
*/

/* REXX関数のロード */
CALL RxFuncAdd 'ArxQueryScreenSize','ALWAREXX','ArxQueryScreenSize'
CALL RxFuncAdd 'ArxQueryMouseButton','ALWAREXX','ArxQueryMouseButton'
CALL RxFuncAdd 'ArxSetPointerPos','ALWAREXX','ArxSetPointerPos'

/* 画面の解像度の取得 */
PARSE VALUE ArxQueryScreenSize() WITH cx cy

/* マウスボタンの状態を取得 */
PARSE VALUE ArxQueryMouseButton() WITH buttons btnsta

IF buttons > 0 THEN DO
	/* 押し下げられたマウスボタンの有無を確認 */
	DO li = 1 TO buttons
		IF WORD(btnsta,li)='1' THEN LEAVE
	END
	/* 押し下げが無ければ、画面の中央へマウスポインタを移動 */
	IF li>buttons THEN DO
		CALL ArxSetPointerPos cx/2,cy/2
	END
END

/* REXX関数のアンロード */
CALL RxFuncDrop 'ArxQueryScreenSize'
CALL RxFuncDrop 'ArxQueryMouseButton'
CALL RxFuncDrop 'ArxSetPointerPos'

EXIT 0
