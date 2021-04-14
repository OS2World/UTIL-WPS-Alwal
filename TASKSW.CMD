/* tasksw.cmd ... REXXマクロ機能サンプル

	機能：「タスクスイッチャ置換」機能のタスクスイッチャを表示する。
	引数：無し。
	備考：ALWALTFXのメイン・ウィンドウに直接メッセージを送る事で、
	　　　内部機能を直接制御しています。一種のウラ技ですね。(^^;)
	　　　もちろん、ALWALTFXが実行中であり、「タスクスイッチャ置換」
	　　　機能が有効になっていないと、正常に動作しません。

	　　　因みに、これで（無理矢理）表示させたタスクスイッチャは、
	　　　キーを離しても自動的に非表示になりません（Alt+SPACE 等の
	　　　取り消し操作を、明示的に行う必要がある）ので、注意。
*/
WM_ALWALTFX_TASKSW_REQUEST_SHOW = 'H1037' /* (WM_USER+55) */

/* REXX関数のロード */
CALL RxFuncAdd 'ArxLoadFuncs','ALWAREXX','ArxLoadFuncs'
CALL ArxLoadFuncs

/* ALWALTFXが実行中でない → 中断 */
IF \ArxQueryAlwaltfxStatus('EXEC?') THEN EXIT 1

/* 取り消し操作時にアクティブになるウィンドウ */
/* （つまり、現在アクティブなウィンドウ） */
hwcancel = ArxQueryWindowHandle('ACTIVE')

/* 最初にカーソルが設定されるウィンドウ */
hwnext   = ArxQueryWindowHandle(hwcancel,'NEXT')

/* ALWALTFX に、タスクスイッチャの表示要求メッセージを送る */
CALL ArxSendMessage ArxQueryAlwaltfxStatus('HWND') ,
 ,WM_ALWALTFX_TASKSW_REQUEST_SHOW ,hwnext ,hwcancel

EXIT 0
