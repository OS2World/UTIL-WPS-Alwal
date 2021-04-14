/* aafxsusp.cmd ... REXXマクロ機能サンプル

	機能：他のセッションから ALWALTFX のREXX関数を呼び出すサンプル。
	　　　ALWALTFXの「全機能休止」チェックのON/OFFを切り替えます。
	引数：無し。
	備考：ALWALTFXの「全機能休止」チェックボックス・ウィンドウを、
	　　　直接制御しています。
*/
ASD_SUSPEND = 102 /* 「全機能休止」チェックボックス・ウィンドウのＩＤ値 */

BM_QUERYCHECK = 'H0124' /* ツールキット定義の各種シンボル */
BM_SETCHECK   = 'H0125'
WM_CONTROL    = 'H0030'
BN_CLICKED    = 1

/* REXX関数のロード */
CALL RxFuncAdd 'ArxLoadFuncs','ALWAREXX','ArxLoadFuncs'
CALL ArxLoadFuncs

/* ALWALTFXが実行中でない → 中断 */
IF \ArxQueryAlwaltfxStatus('EXEC?') THEN EXIT 1

/* ALWALTFXメイン・ウィンドウ・ハンドルの取得*/
hwalwaltfx = ArxQueryAlwaltfxStatus('HWND')

/* 「全機能休止」チェックボックス・ウィンドウ・ハンドルの取得 */
hwcheck = ArxQueryWindowHandle( hwalwaltfx ,ASD_SUSPEND )

/* 「全機能休止」チェックボックスの状態をトグルする */
CALL ArxSendMessage hwcheck ,BM_SETCHECK ,
 ,\ArxX2D( ArxSendMessage( hwcheck ,BM_QUERYCHECK ,0 ,0 ) ) ,0

/* 「全機能休止」チェックボックスの状態変化をメイン・ウィンドウに知らせる */
CALL ArxSendMessage hwalwaltfx ,WM_CONTROL ,
 ,ArxMPFROM2SHORT(ASD_SUSPEND,BN_CLICKED) ,hwcheck

EXIT 0
