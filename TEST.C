/* TEST.C ... ALWALTFX「タスクスイッチャ置換」デバッグ用 */
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include "alwatask.h"


BOOL SWBLOCK_Alloc( HAB hab ,PSWBLOCK* sw );
VOID SWBLOCK_Free( PSWBLOCK* sw );


/* 関数名：main */
/* 機能　：メイン関数、設定ダイアログ → タスクスイッチャの順で表示する */
/* 引数　：無し */
/* 戻り値：無し */
/* 備考　：ALWALTFXが内部的に行っている処理を、模擬的に実現しています */
void main()
{
	HAB hab = WinInitialize(0);
	HMQ hmq = WinCreateMsgQueue(hab,0);
	if(hmq){
		/* タスクスイッチャモジュールの初期化 */
		TASKSW_PARAM prm;
		prm.cbSize = sizeof(TASKSW_PARAM);
		prm.hab = hab;
		prm.hwalwaltfx = HWND_DESKTOP;
		prm.hmodule = 0;
		if( TASKSW_CREATE(&prm) ){
			PSWBLOCK sw;
			/* 設定ダイアログを表示 */
			TASKSW_SETTING(&prm);
			/* アクティブ・ウィンドウが確定するまで待機 */
			while( !WinQueryActiveWindow(HWND_DESKTOP) ){
				QMSG qmsg;
				WinPeekMsg( hab ,&qmsg ,0 ,0 ,0 ,PM_NOREMOVE );
			}
			/* 現在のタスク一覧データを取得する */
			if( SWBLOCK_Alloc(hab,&sw) ){
				HWND hwactive = WinQueryActiveWindow(HWND_DESKTOP);
				HWND hwnext = WinQueryWindow( hwactive ,QW_NEXT );
				LONG cursor = -1 ,cancel = -1;
				int li;
				/* 初期カーソル位置及びキャンセル時に選択するタスクの */
				/* オフセット番号を検索する（見つからない → -1） */
				for( li = 0 ;li<sw->cswentry ;li++ ){
					if( sw->aswentry[li].swctl.hwnd==hwnext ) cursor = li;
					else
					if( sw->aswentry[li].swctl.hwnd==hwactive ) cancel = li;
				}
				/* タスクスイッチャ・ウィンドウを表示する */
				if( TASKSW_SHOW(&prm,sw,cursor,cancel) ){
					QMSG qmsg;
					/* タスクスイッチャ・ウィンドウが閉じるのを待つ */
					while( WinGetMsg(hab,&qmsg,0,0,0) ){
						WinDispatchMsg(hab,&qmsg);
					}
					/* タスクスイッチャ・ウィンドウを非表示にする */
					TASKSW_HIDE(&prm);
				}
				/* タスク一覧データを解放する */
				SWBLOCK_Free(&sw);
			}
			/* タスクスイッチャモジュールの終了処理 */
			TASKSW_DESTROY(&prm);
		}
		WinDestroyMsgQueue(hmq);
	}
	WinTerminate(hab);
}


/* 関数名：SWBLOCK_Alloc */
/* 機能　：現在のタスク一覧データを取得する */
/* 引数　：hab = アンカー・ブロック・ハンドル */
/* 　　　：sw = タスク一覧データ取得バッファポインタのポインタ */
/* 戻り値：TRUE=取得成功、FALSE=取得失敗 */
/* 備考　：ALWALTFXが内部的に行っている処理を、模擬的に実現しています */
BOOL SWBLOCK_Alloc( HAB hab ,PSWBLOCK* sw )
{
	BOOL rc = FALSE;
	ULONG cnt = WinQuerySwitchList( hab ,0 ,0 );
	if(cnt){
		ULONG len = sizeof(SWBLOCK)+sizeof(SWENTRY)*(cnt-1);
		if( !DosAllocMem( (PPVOID)sw ,len ,PAG_COMMIT|PAG_READ|PAG_WRITE ) ){
			cnt = WinQuerySwitchList( hab ,*sw ,len );
			if( cnt ){
				rc = TRUE;
			}
		}
	}
	if(!rc) SWBLOCK_Free(sw);
	return rc;
}


/* 関数名：SWBLOCK_Free */
/* 機能　：タスク一覧データを解放する */
/* 引数　：sw = タスク一覧データ取得バッファポインタのポインタ */
/* 戻り値：無し */
/* 備考　：ALWALTFXが内部的に行っている処理を、模擬的に実現しています */
VOID SWBLOCK_Free( PSWBLOCK* sw )
{
	DosFreeMem(*sw); *sw = 0;
}

