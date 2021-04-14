/* ALWATSKS.C ... ALWALTFX「タスクスイッチャ置換」設定ダイアログ */
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include "alwatsks.h"
#include "alwatskd.h"


/*extern*/ IGNTASK_BAG IgnTask ={ 0 ,0 };

static void TASKSETTING_ReloadTask( HWND hdlg );
static BOOL IGNTASK_Add( HWND hdlg );
static BOOL IGNTASK_Remove( HWND hdlg );

/* INIファイル保存用データ定義 */
static const char INIFILE_NAME[] ={ "ALWATASK.INI" };
static const char INIFILE_IGNSTR_KEY[] ={ "IGNSTR" };
static const char INIFILE_IGNSTR_CNT[] ={ "CNT" };
static const char INIFILE_IGNSTR_BUF[] ={ "BUF" };


/* 関数名：TASKSETTING_Init */
/* 機能　：タスクスイッチャ設定の初期化 */
/* 引数　：無し */
/* 戻り値：無し */
/* 備考　：静的変数の初期化及びINIファイル読み込みを行っている */
void TASKSETTING_Init( HAB hab )
{
	HINI hini;

	IgnTask.cnt = 0;
	IgnTask.buf = 0;

	hini = PrfOpenProfile( hab ,(PSZ)INIFILE_NAME );
	if(hini){
		int ncnt = 0;
		{
			char cbuf[10] ={ '\0' };
			ULONG csiz = sizeof(cbuf);
			if( PrfQueryProfileData( hini ,(PSZ)INIFILE_IGNSTR_KEY ,(PSZ)INIFILE_IGNSTR_CNT ,cbuf ,&csiz ) ){
				/* ncnt = atoi( cbuf ) */
				char* lp;
				for( lp = cbuf ;*lp>='0' && *lp<='9' ;*lp++ ){
					ncnt = ncnt * 10 + *lp - '0';
				}
			}
		}
		if( ncnt ){
			ULONG nsiz = sizeof(IGNTASK_SINGLE) * ncnt;
			PVOID nbuf;
			if( !DosAllocMem( &nbuf ,nsiz ,PAG_COMMIT|PAG_READ|PAG_WRITE ) ){
				ULONG rsiz = nsiz;
				if( PrfQueryProfileData( hini ,(PSZ)INIFILE_IGNSTR_KEY ,(PSZ)INIFILE_IGNSTR_BUF ,nbuf ,&rsiz ) ){
					if( rsiz==nsiz ){
						IgnTask.cnt = ncnt;
						IgnTask.buf = nbuf;
					}
				}
				if( !IgnTask.buf ){ DosFreeMem( nbuf ); }
			}
		}
		PrfCloseProfile(hini);
	}
}


/* 関数名：TASKSETTING_Term */
/* 機能　：タスクスイッチャ設定の終了 */
/* 引数　：無し */
/* 戻り値：無し */
/* 備考　：INIファイル書き込み及び静的変数の解放を行っている */
void TASKSETTING_Term( HAB hab )
{
	HINI hini = PrfOpenProfile( hab ,(PSZ)INIFILE_NAME );
	if(hini){
		char buf[10];
		/* itoa( buf ,IgnTask.cnt ,10 ); */
		char* bp = &buf[sizeof(buf)-2];
		{ 
			int cnt = IgnTask.cnt;
			do{ *(bp--) = '0'+(cnt%10); cnt /= 10; } while(cnt);
			bp++;
		}
		buf[sizeof(buf)-1] = '\0';
		PrfWriteProfileString( hini ,(PSZ)INIFILE_IGNSTR_KEY ,(PSZ)INIFILE_IGNSTR_CNT ,bp );
		if( IgnTask.cnt ){
			ULONG wsiz = sizeof(IGNTASK_SINGLE) * IgnTask.cnt;
			PrfWriteProfileData( hini ,(PSZ)INIFILE_IGNSTR_KEY ,(PSZ)INIFILE_IGNSTR_BUF ,IgnTask.buf ,wsiz );
		}
		PrfCloseProfile(hini);
	}

	DosFreeMem( IgnTask.buf );
}


/* 関数名：TASKSW_SettingDlgProc */
/* 機能　：タスクスイッチャ設定用ダイアログ・プロシージャ */
/* 引数　： */
/* 戻り値： */
/* 備考　： */
MRESULT EXPENTRY TASKSETTING_DlgProc( HWND hdlg ,ULONG msg ,MPARAM mp1 ,MPARAM mp2 )
{
	switch( msg ){
	 case WM_INITDLG:
		{
			HWND hlhide = WinWindowFromID( hdlg ,ALWATASK_SETTING_LBOX_HIDE );
			int li;
			WinSendMsg( hlhide ,LM_DELETEALL ,0 ,0 );
			for( li = 0 ;li < IgnTask.cnt ;li++ ){
				WinSendMsg( hlhide ,LM_INSERTITEM ,MPFROMSHORT(li) ,MPFROMP(IgnTask.buf[li].str) );
			}
		}
		TASKSETTING_ReloadTask( hdlg );
		break;
	 case WM_CONTROL:
		switch( (ULONG)mp1 ){
		 case (ULONG)MPFROM2SHORT(ALWATASK_SETTING_LBOX_HIDE,LN_SELECT):
			WinEnableWindow( WinWindowFromID( hdlg ,ALWATASK_SETTING_PBTN_REMOVE )
			 ,((SHORT)WinSendMsg( HWNDFROMMP(mp2) ,LM_QUERYSELECTION ,MPFROMSHORT(0) ,0 )!=LIT_NONE) );
			break;
		 case (ULONG)MPFROM2SHORT(ALWATASK_SETTING_LBOX_SOURCE,LN_SELECT):
			WinEnableWindow( WinWindowFromID( hdlg ,ALWATASK_SETTING_PBTN_ADD )
			 ,((SHORT)WinSendMsg( HWNDFROMMP(mp2) ,LM_QUERYSELECTION ,MPFROMSHORT(0) ,0 )!=LIT_NONE) );
			break;
		}
		break;
	 case WM_COMMAND:
		switch( COMMANDMSG(&msg)->cmd ){
		 case ALWATASK_SETTING_PBTN_REMOVE:
			if( !IGNTASK_Remove( hdlg ) ){
				WinAlarm( HWND_DESKTOP ,WA_WARNING );
			}
			return 0;
		 case ALWATASK_SETTING_PBTN_ADD:
			if( !IGNTASK_Add( hdlg ) ){
				WinAlarm( HWND_DESKTOP ,WA_WARNING );
			}
			return 0;
		 case ALWATASK_SETTING_PBTN_RELOAD:
			TASKSETTING_ReloadTask( hdlg );
			return 0;
		}
		break;
	}
	return WinDefDlgProc( hdlg ,msg ,mp1 ,mp2 );
}


/* 関数名：IGNTASK_IsExist */
/* 機能　：非表示タスクタイトルバッファ内から、該当文字列を検索 */
/* 引数　：str = 検索対象文字列 */
/* 戻り値：有り=バッファ内の該当番号、無し=-1 */
/* 備考　： */
int IGNTASK_IsExist( const char* str )
{
	int rc = -1;
	int li;
	for( li = 0 ;li < IgnTask.cnt ;li++ ){
		if( !strcmp( IgnTask.buf[li].str ,str ) ){
			rc = li;
			break;
		}
	}
	return rc;
}


/* 関数名：TASKSETTING_ReloadTask */
/* 機能　：現在のタスク一覧を、「現行タスクタイトル」リストボックスに反映する */
/* 引数　：hdlg = ダイアログ・ウィンドウ・ハンドル */
/* 戻り値：無し */
/* 備考　：ダイアログ表示直後及び「再読込」ボタン押し下げ時に呼び出される */
static void TASKSETTING_ReloadTask( HWND hdlg )
{
	HAB hab = WinQueryAnchorBlock( hdlg );
	ULONG cnt = WinQuerySwitchList( hab ,0 ,0 );
	HWND hltask = WinWindowFromID( hdlg ,ALWATASK_SETTING_LBOX_SOURCE );
	WinSendMsg( hltask ,LM_DELETEALL ,0 ,0 );
	if(cnt){
		ULONG len = sizeof(SWBLOCK)+sizeof(SWENTRY)*(cnt-1);
		PSWBLOCK sw;
		if( !DosAllocMem( (PPVOID)&sw ,len ,PAG_COMMIT|PAG_READ|PAG_WRITE ) ){
			cnt = WinQuerySwitchList( hab ,sw ,len );
			if( cnt ){
				int li;
				for( li = 0 ;li < cnt ;li++ ){
					PSWCNTRL sc = &(sw->aswentry[li].swctl);
					if( sc->uchVisibility!=SWL_VISIBLE ) continue;
					if( sc->fbJump!=SWL_JUMPABLE ) continue;
					WinSendMsg( hltask ,LM_INSERTITEM ,MPFROMSHORT(LIT_END)
					 ,MPFROMP(sw->aswentry[li].swctl.szSwtitle) );
				}
			}
			DosFreeMem(sw);
		}
	}
}


/* 関数名：IGNTASK_Add */
/* 機能　： */
/* 引数　：hdlg = ダイアログ・ウィンドウ・ハンドル */
/* 戻り値：TRUE=追加成功、FALSE=追加失敗 */
/* 備考　：「追加」ボタン押し下げ時に呼び出される */
static BOOL IGNTASK_Add( HWND hdlg )
{
	BOOL rc = FALSE;
	HWND hlhide = WinWindowFromID( hdlg ,ALWATASK_SETTING_LBOX_HIDE );
	HWND hltask = WinWindowFromID( hdlg ,ALWATASK_SETTING_LBOX_SOURCE );
	SHORT sel = (SHORT)WinSendMsg( hltask ,LM_QUERYSELECTION ,MPFROMSHORT(0) ,0 );
	if( sel!=LIT_NONE ){
		char buf[MAXNAMEL+4];
		if( WinSendMsg( hltask ,LM_QUERYITEMTEXT ,MPFROM2SHORT(sel,sizeof(buf)) ,MPFROMP(buf) ) ){
			if( IGNTASK_IsExist(buf)<0 ){
				int ncnt = IgnTask.cnt + 1;
				ULONG nsiz = sizeof(IGNTASK_SINGLE) * ncnt;
				PVOID nbuf;
				if( !DosAllocMem( &nbuf ,nsiz ,PAG_COMMIT|PAG_READ|PAG_WRITE ) ){
					memcpy( nbuf ,IgnTask.buf ,nsiz - sizeof(IGNTASK_SINGLE) );
					DosFreeMem( IgnTask.buf );
					IgnTask.cnt = ncnt;
					IgnTask.buf = (IGNTASK_SINGLE*)nbuf;
					strcpy( IgnTask.buf[ncnt-1].str ,buf );
					switch( (SHORT)WinSendMsg( hlhide ,LM_INSERTITEM ,MPFROMSHORT(LIT_END) ,MPFROMP(buf) ) ){
					 case LIT_ERROR: case LIT_MEMERROR:
						IgnTask.cnt--;
						break;
					 default:
						rc = TRUE;
						WinSendMsg( hlhide ,LM_SELECTITEM ,MPFROMSHORT(IgnTask.cnt-1) ,MPFROMSHORT(TRUE) );
					}
				}
			}
		}
	}
	return rc;
}


/* 関数名：IGNTASK_Remove */
/* 機能　： */
/* 引数　：hdlg = ダイアログ・ウィンドウ・ハンドル */
/* 戻り値：TRUE=削除成功、FALSE=削除失敗 */
/* 備考　：「削除」ボタン押し下げ時に呼び出される */
static BOOL IGNTASK_Remove( HWND hdlg )
{
	BOOL rc = FALSE;
	HWND hlhide = WinWindowFromID( hdlg ,ALWATASK_SETTING_LBOX_HIDE );
	SHORT sel = (SHORT)WinSendMsg( hlhide ,LM_QUERYSELECTION ,MPFROMSHORT(0) ,0 );
	if( sel!=LIT_NONE ){
		if( sel<IgnTask.cnt ){
			int ncnt = IgnTask.cnt - 1;
			if( sel<IgnTask.cnt-1 ){
				memmove( &IgnTask.buf[sel] ,&IgnTask.buf[sel+1]
				 ,sizeof(IGNTASK_SINGLE) * (IgnTask.cnt - 1 - sel) );
			}
			IgnTask.cnt = ncnt;
			WinSendMsg( hlhide ,LM_DELETEITEM ,MPFROMSHORT(sel) ,0 );
			if(!IgnTask.cnt){
				WinEnableWindow( WinWindowFromID( hdlg ,ALWATASK_SETTING_PBTN_REMOVE ) ,FALSE );
			}
			rc = TRUE;
		}
	}
	return rc;
}

