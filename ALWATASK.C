/* ALWATASK.C ... ALWALTFX「タスクスイッチャ置換」サンプル */
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include "alwatask.h"
#include "alwatsks.h"
#include "alwatskd.h"

#if MAKE_DLL
/* DLL初期化ルーチン */
/* 注：Ｃライブラリをリンクさせる場合は、削除もしくはコメントアウトする必要があります */
ULONG EXPENTRY _DLL_InitTerm( HMODULE hmodule ,ULONG flg ){ return TRUE; }
#endif

static const char TASKSW_CLASSNAME[] ={ "ALWALTFX_TASKSW" };
FNWP TASKSW_WndProc;

static BOOL TASKSW_AllocMem( ULONG ssiz ,ULONG vsiz );
static void TASKSW_FreeMem(void);
static int  TASKSW_CursorFromChar( UCHAR chr );
static int  TASKSW_CursorFromPoint( PPOINTS ptr );
static void TASKSW_SetNewCursor( HWND hwnd ,int ncsr );
static void TASKSW_DrawCursor( HPS hps ,BOOL flg );
static void TASKSW_Draw3DFrame( HPS hps ,PRECTL rect ,int width ,LONG col1 ,LONG col2 );

static HWND HwTaskFrame;  /* タスクスイッチャのフレーム・ウィンドウ・ハンドル */
static HWND HwTaskClient; /* タスクスイッチャのクライアント・ウィンドウ・ハンドル */
static HWND HwTaskText;   /* 選択されているタスク名表示用静的制御ウィンドウ・ハンドル */

/* 各種コマンドＩＤ */
#define TASKCMD_SELECT 1 /* 選択 */
#define TASKCMD_CANCEL 2 /* 取り消し */
#define TASKCMD_UP     3 /* ↑へカーソル移動 */
#define TASKCMD_DOWN   4 /* ↓     〃        */
#define TASKCMD_LEFT   5 /* ←     〃        */
#define TASKCMD_RIGHT  6 /* →     〃        */

/* アクセラレータ・テーブル */
#define ACCEL_CNT 9
static struct{
	ACCELTABLE table;
	ACCEL acc[ACCEL_CNT-1];
} AccelTable ={
  { ACCEL_CNT ,0
 ,{  { AF_ALT|AF_VIRTUALKEY          ,VK_ENTER   ,TASKCMD_SELECT } } }
 ,{  { AF_ALT|AF_VIRTUALKEY          ,VK_NEWLINE ,TASKCMD_SELECT }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_SPACE   ,TASKCMD_CANCEL }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_UP      ,TASKCMD_UP     }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_DOWN    ,TASKCMD_DOWN   }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_LEFT    ,TASKCMD_LEFT   }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_RIGHT   ,TASKCMD_RIGHT  }
    ,{ AF_ALT|AF_VIRTUALKEY          ,VK_TAB     ,TASKCMD_RIGHT  }
    ,{ AF_ALT|AF_SHIFT|AF_VIRTUALKEY ,VK_BACKTAB ,TASKCMD_LEFT   } }
};

/* 数字・アルファベットキー検出テーブル */
static const UCHAR ScanCode_Numeric[10][2] ={
  { 0x0B ,0x52 } /* '0' ... １つめがフルキー側、２つめがテンキー側 */
 ,{ 0x02 ,0x4F } /* '1' */
 ,{ 0x03 ,0x50 } /* '2' */
 ,{ 0x04 ,0x51 } /* '3' */
 ,{ 0x05 ,0x4B } /* '4' */
 ,{ 0x06 ,0x4C } /* '5' */
 ,{ 0x07 ,0x4D } /* '6' */
 ,{ 0x08 ,0x47 } /* '7' */
 ,{ 0x09 ,0x48 } /* '8' */
 ,{ 0x0A ,0x49 } /* '9' */
};

static const UCHAR ScanCode_Alphabet[26] ={
  0x1E /* 'A' */
 ,0x30 /* 'B' */
 ,0x2E /* 'C' */
 ,0x20 /* 'D' */
 ,0x12 /* 'E' */
 ,0x21 /* 'F' */
 ,0x22 /* 'G' */
 ,0x23 /* 'H' */
 ,0x17 /* 'I' */
 ,0x24 /* 'J' */
 ,0x25 /* 'K' */
 ,0x26 /* 'L' */
 ,0x32 /* 'M' */
 ,0x31 /* 'N' */
 ,0x18 /* 'O' */
 ,0x19 /* 'P' */
 ,0x10 /* 'Q' */
 ,0x13 /* 'R' */
 ,0x1F /* 'S' */
 ,0x14 /* 'T' */
 ,0x16 /* 'U' */
 ,0x2F /* 'V' */
 ,0x11 /* 'W' */
 ,0x2D /* 'X' */
 ,0x15 /* 'Y' */
 ,0x2C /* 'Z' */
};

static HACCEL Haccel;     /* タスクスイッチャ用アクセラレータ・テーブル・ハンドル */

static SWBLOCK* SwBlock = 0; /* タスク一覧構造体のバッファ */
static int* BufValid = 0;    /* ↑の表示するオフセット番号の列挙バッファ */
static int CntValid;         /* ↑の合計数 */
static int Cursor;           /* タスク選択カーソル位置 */
static int NumCancel;        /* タスクスイッチ取り消し時の戻り先番号 */

static LONG SvScrX,SvScrY; /* スクリーンの縦横幅システム値 */
static LONG SvPtrX,SvPtrY; /* アイコンの縦横幅システム値 */
static LONG TitleY;        /* タスク名表示静的制御ウィンドウの縦幅 */
static LONG DlgBkCol;      /* タスクウィンドウの背景色 */
static HPOINTER HptrPgm;   /* アイコンを持たないタスク用の仮アイコン */

#define	TASKW_ENUM_X 10 /* タスクウィンドウ上に置ける横並び数 */

#define FRAME_X 4 /* （仮想）フレームの横太さ */
#define FRAME_Y 4 /* （仮想）フレームの縦太さ */

/* ↓タスクアイコン表示先頭Ｘ座標(左端) */
#define ICO_TOP_X (SvPtrX/2)
/* ↓タスクアイコン表示先頭Ｙ座標(上端)（c=総タスク数） */
#define ICO_TOP_Y(c) ((SvPtrY/2)+(SvPtrY*3/2)*((c)/TASKW_ENUM_X)+TitleY)
/* ↓タスクアイコン表示Ｘ座標算出（n=タスク番号） */
#define ICO_LOC_X(n) (ICO_TOP_X+(SvPtrX*3/2)*((n)%TASKW_ENUM_X))
/* ↓タスクアイコン表示Ｙ座標算出（c=総タスク数、n=タスク番号） */
#define ICO_LOC_Y(c,n) (ICO_TOP_Y(c)-(SvPtrY*3/2)*((n)/TASKW_ENUM_X))

/* ↓タスクアイコン外周に描画されるカーソル枠の座標（c=総タスク数、n=タスク番号） */
#define CSR_LEFT(n) (ICO_LOC_X(n)-SvPtrX/4)
#define CSR_BOTTOM(c,n) (ICO_LOC_Y(c,n)-SvPtrY/4)
#define CSR_RIGHT(n) (ICO_LOC_X(n)+SvPtrX*5/4-1)
#define CSR_TOP(c,n) (ICO_LOC_Y(c,n)+SvPtrY*5/4-1)

/* ↓タスクウィンドウそのもののサイズ及び表示座標（c=総タスク数） */
#define TASKW_SIZ_X (FRAME_X*2+ICO_LOC_X(TASKW_ENUM_X-1)+ICO_LOC_X(1))
#define TASKW_SIZ_Y(c) (FRAME_Y*2+ICO_TOP_Y((c)+TASKW_ENUM_X))
#define TASKW_OFS_X ((SvScrX-TASKW_SIZ_X)/2)
#define TASKW_OFS_Y(c) ((SvScrY-TASKW_SIZ_Y(c))/2)


/* 関数名：TASKSW_CREATE */
/* 機能　：タスクスイッチャ・ウィンドウを作成する（表示は行わない） */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 戻り値：正常終了で呼び出し元から送られるパラメータの構造体 TASKSW_PARAM */
/* 　　　：のサイズを、 異常終了で 0 を返す */
/* 備考　：DLLロード直後に一度だけ呼び出される */
/* 　　　：この関数は、必ず定義されなければならない */
ULONG EXPENTRY TASKSW_CREATE( const TASKSW_PARAM* prm )
{
	ULONG rc = 0;

	/* 一番初めに、ALWALTFXから送られるパラメータのサイズが、こちらが希望 */
	/* するサイズを満たしているが、確認しなければならない */
	if( prm->cbSize>=sizeof(TASKSW_PARAM) ){
		TASKSETTING_Init( prm->hab );
		if( WinRegisterClass( prm->hab ,(PSZ)TASKSW_CLASSNAME ,TASKSW_WndProc ,0 ,0 ) ){
			ULONG fcf = 0;
			HwTaskFrame = WinCreateStdWindow( HWND_DESKTOP ,0 ,&fcf
			 ,(PSZ)TASKSW_CLASSNAME ,(PSZ)TASKSW_CLASSNAME ,WS_VISIBLE ,prm->hmodule ,0 ,&HwTaskClient );
			if( HwTaskFrame!=0 ){
				SvScrX = WinQuerySysValue( HWND_DESKTOP ,SV_CXSCREEN );
				SvScrY = WinQuerySysValue( HWND_DESKTOP ,SV_CYSCREEN );
				SvPtrX = WinQuerySysValue( HWND_DESKTOP ,SV_CXICON );
				SvPtrY = WinQuerySysValue( HWND_DESKTOP ,SV_CYICON );
				TitleY = WinQuerySysValue( HWND_DESKTOP ,SV_CYTITLEBAR );
				DlgBkCol = WinQuerySysColor( HWND_DESKTOP ,SYSCLR_DIALOGBACKGROUND ,0 );
				HptrPgm = WinQuerySysPointer( HWND_DESKTOP ,SPTR_PROGRAM ,FALSE );
				HwTaskText = WinCreateWindow( HwTaskFrame ,WC_STATIC ,""
				 ,WS_VISIBLE|SS_TEXT|DT_CENTER ,0 ,0 ,0 ,0 ,HwTaskFrame ,HWND_BOTTOM ,1 ,0 ,0 );
				WinSetPresParam( HwTaskText ,PP_BACKGROUNDCOLOR ,sizeof(LONG) ,&DlgBkCol );

				Haccel = WinCreateAccelTable( prm->hab ,&AccelTable.table );
				WinSetAccelTable( prm->hab ,Haccel ,HwTaskFrame );

				/* 初期化完了の印に、こちら(DLL側)が想定する TASKSW_PARAM */
				/* のサイズを返す。ALWALTFX は、その値からこのDLLが想定する */
				/* ALWALTFX のバージョンを割り出す（将来用の仕組み） */
				rc = sizeof(TASKSW_PARAM);
			}
		}
	}

	return rc;
}


/* 関数名：TASKSW_SHOW */
/* 機能　：タスクスイッチャ・ウィンドウを表示する */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 　　　：sw = タスク一覧構造体へのポインタ */
/* 　　　：cursor = 最初に選択されるタスクのオフセット番号 */
/* 　　　：cancel = 取り消し操作時にアクティブになるタスクのオフセット番号 */
/* 　　　：（cursor,cancelの値が-1の場合、該当するタスクが無い事を指す） */
/* 戻り値：正常終了で TRUE、異常終了で FALSE */
/* 備考　：Alt+TAB操作を検出した時などに、ALWALTFX から呼び出される */
/* 　　　：（表示中に呼び出される事は無い。後述の TASKSW_ISSHOWING を参照） */
/* 　　　：この関数は、必ず定義されなければならない */
BOOL EXPENTRY TASKSW_SHOW( const TASKSW_PARAM* prm ,PSWBLOCK sw ,LONG cursor ,LONG cancel )
{
	BOOL rc = FALSE;
	int li;

	ULONG ssiz = sizeof(SWBLOCK)+sizeof(SWENTRY)*(sw->cswentry-1);
	ULONG vsiz = sizeof(int)*sw->cswentry;

	if( TASKSW_AllocMem(ssiz,vsiz) ){
		/* タスク情報を保持するメモリバッファ割り当て成功 */
		memcpy( SwBlock ,sw ,ssiz );
		CntValid = 0;
		Cursor = 0;
		NumCancel = -1;
		/* タスク情報の内容を一通り確認、表示するタスクを選定する */
		for( li = 0 ;li < SwBlock->cswentry ;li++ ){
			int lj;
			PSWCNTRL sc = &(SwBlock->aswentry[li].swctl);
			/* 不可視もしくは選択不可タスク → スキップ */
			if( sc->uchVisibility!=SWL_VISIBLE ) continue;
			if( sc->fbJump!=SWL_JUMPABLE ) continue;
			/* 非表示タスクタイトル設定に合致していないか確認 */
			for( lj = 0 ;lj < IgnTask.cnt ;lj++ ){
				if( !strcmp( IgnTask.buf[lj].str ,sc->szSwtitle ) ) break;
			}
			if( lj < IgnTask.cnt ){
				/* 非表示タスクタイトル設定に合致 → スキップ */
				continue;
			}

			if( li==cursor ) Cursor = CntValid; /* カーソル初期位置 */
			else
			if( li==cancel ) NumCancel = li;  /* 取消操作時の戻り先 */

			BufValid[CntValid++] = li; /* 表示するタスク番号に追加 */
		}
		if( CntValid ){
			WinSetWindowPos( HwTaskFrame ,HWND_TOP
			 ,TASKW_OFS_X ,TASKW_OFS_Y(CntValid)
			 ,TASKW_SIZ_X ,TASKW_SIZ_Y(CntValid)
			 ,SWP_MOVE|SWP_SIZE|SWP_ZORDER|SWP_SHOW|SWP_ACTIVATE );
			WinSetFocus( HWND_DESKTOP ,HwTaskClient );
			rc = TRUE;
		}
	}

	if(!rc) TASKSW_FreeMem();

	return rc;
}


/* 関数名：TASKSW_HIDE */
/* 機能　：タスクスイッチャ・ウィンドウを非表示にする */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 戻り値：無し */
/* 備考　：タスクスイッチャを非表示にすべき時に、ALWALTFX から呼び出される */
/* 　　　：（表示中ではない場合でも、呼び出される可能性がある） */
/* 　　　：この関数は、必ず定義されなければならない */
VOID EXPENTRY TASKSW_HIDE( const TASKSW_PARAM* prm )
{
	WinShowWindow( HwTaskFrame ,FALSE );
	WinSetWindowPos( HwTaskFrame ,0 ,0,0,0,0 ,SWP_DEACTIVATE );
	TASKSW_FreeMem();
}


/* 関数名：TASKSW_ISSHOWING */
/* 機能　：タスクスイッチャ・ウィンドウを表示されているか否かを返す */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 戻り値：表示中で TRUE、非表示中で FALSE */
/* 備考　：タスクスイッチャ表示前などに、ALWALTFX から呼び出される */
/* 　　　：なお、タスクスイッチャ表示時は、タスクスイッチャ・ウィンドウ自身 */
/* 　　　：（もしくは子ウィンドウ）がフォーカスを持っていなければならない */
/* 　　　：つまり、タスクスイッチャ・ウィンドウの表示←→非表示のトグルと、 */
/* 　　　：アクティブ←→非アクティブのトグルとは、同期していなければならな */
/* 　　　：い、という事である（ALWALTFXの内部処理は、それを前提としている） */
/* 　　　：この関数は、必ず定義されなければならない */
BOOL EXPENTRY TASKSW_ISSHOWING( const TASKSW_PARAM* prm )
{
	return WinIsWindowVisible( HwTaskFrame );
}


/* 関数名：TASKSW_DESTROY */
/* 機能　：タスクスイッチャ・ウィンドウを破棄する */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 戻り値：無し */
/* 備考　：DLLアンロード直前に一度だけ呼び出される */
/* 　　　：（TASKSW_CREATEの成否に関わらず、必ず呼び出される） */
/* 　　　：この関数は、必ず定義されなければならない */
VOID EXPENTRY TASKSW_DESTROY( const TASKSW_PARAM* prm )
{
	TASKSETTING_Term( prm->hab );
	WinSetAccelTable( prm->hab ,0 ,HwTaskFrame );
	WinDestroyAccelTable( Haccel );
	WinDestroyWindow( HwTaskText );
	WinDestroyWindow( HwTaskFrame );
}


/* 関数名：TASKSW_SETTING */
/* 機能　：タスクスイッチャ・ウィンドウ設定用ダイアログ・ウィンドウを表示する */
/* 引数　：prm = 呼び出し元から送られるパラメータ */
/* 戻り値：正常終了で TRUE、異常終了で FALSE */
/* 備考　：この関数は、必ずしも定義する必要は無い */
BOOL EXPENTRY TASKSW_SETTING( const TASKSW_PARAM* prm )
{
	return WinDlgBox( HWND_DESKTOP ,prm->hwalwaltfx ,TASKSETTING_DlgProc
	 ,prm->hmodule ,ALWATASK_SETTING_MAIN ,0 )!=DID_ERROR;
}


/* 関数名：TASKSW_WndProc */
/* 機能　：タスクスイッチャ・クライアント・ウィンドウ・プロシージャ */
/* 引数　： */
/* 戻り値： */
/* 備考　：ウィンドウ・プロシージャで要求される処理は、以下の３点である */
/* 　　　：　・ウィンドウの描画 */
/* 　　　：　・操作者の操作に対する応答 */
/* 　　　：　・タスク切り替え時及び非アクティブ時にウィンドウを非表示に */
MRESULT EXPENTRY TASKSW_WndProc( HWND hwnd ,ULONG msg ,MPARAM mp1 ,MPARAM mp2 )
{
	static SIZEL size;

	switch(msg){
	 case WM_PAINT:
		if(!SwBlock||!BufValid) break; /* 初期化確認 */
		else{
			int li;
			RECTL rect;
			HPS hps = WinBeginPaint( hwnd ,0 ,0 );
			GpiCreateLogColorTable( hps ,0 ,LCOLF_RGB ,0 ,0 ,0 );
			WinQueryWindowRect( hwnd ,&rect );
			WinFillRect( hps ,&rect ,DlgBkCol );
			rect.xLeft = 0;
			rect.yBottom = 0;
			rect.xRight = size.cx - 1;
			rect.yTop = size.cy - 1;
			TASKSW_Draw3DFrame( hps ,&rect ,1 ,CLR_WHITE ,CLR_DARKGRAY );
			rect.xLeft = FRAME_X;
			rect.yBottom = FRAME_Y;
			rect.xRight = size.cx - 1 - FRAME_X;
			rect.yTop = size.cy - 1 - FRAME_Y;
			TASKSW_Draw3DFrame( hps ,&rect ,1 ,CLR_DARKGRAY ,CLR_WHITE );
			for( li = 0 ;li < CntValid ;li++ ){
				HPOINTER hptr = (HPOINTER)WinSendMsg( SwBlock->aswentry[BufValid[li]].swctl.hwnd ,WM_QUERYICON ,0 ,0 );
				WinDrawPointer( hps ,ICO_LOC_X(li) ,ICO_LOC_Y(CntValid,li) ,hptr?hptr:HptrPgm ,DP_NORMAL );
			}
			TASKSW_DrawCursor(hps,TRUE);
			WinEndPaint(hps);
		}
		return 0;
	 case WM_SIZE:
		size.cx = SHORT1FROMMP(mp2);
		size.cy = SHORT2FROMMP(mp2);
		WinSetWindowPos( HwTaskText ,0 ,FRAME_X+1 ,FRAME_Y+1 ,size.cx-(FRAME_X+1)*2 ,TitleY ,SWP_MOVE|SWP_SIZE );
		break;
	 case WM_CHAR:
		if( (CHARMSG(&msg)->fs&(KC_VIRTUALKEY|KC_KEYUP))==(KC_VIRTUALKEY|KC_KEYUP)
		 && CHARMSG(&msg)->vkey==VK_ALT ){
			/* Altキー押し下げ解除 → 「選択」と同じ操作 */
			WinSendMsg( hwnd ,WM_COMMAND
			 ,MPFROMSHORT(TASKCMD_SELECT) ,MPFROM2SHORT(CMDSRC_OTHER,FALSE) );
			return (MRESULT)TRUE;
		}
		if(!SwBlock||!BufValid) break; /* 初期化確認 */
		if( (CHARMSG(&msg)->fs&(KC_SCANCODE|KC_KEYUP))==(KC_SCANCODE) ){
			/* 先頭キャラクタが一致するタスクへカーソル移動 */
			int ncsr = -1;
			int li;
			/* '0'-'9'チェック */
			for( li = 0 ;li<10 ;li++ ){
				if( CHARMSG(&msg)->scancode==ScanCode_Numeric[li][0]
				 || CHARMSG(&msg)->scancode==ScanCode_Numeric[li][1] ){
					break;
				}
			}
			if(li<10){
				ncsr = TASKSW_CursorFromChar( '0'+li );
			}
			else{
				/* 'A'-'Z'チェック */
				for( li = 0 ;li<26 ;li++ ){
					if( CHARMSG(&msg)->scancode==ScanCode_Alphabet[li] ){
						break;
					}
				}
				if(li<26){
					ncsr = TASKSW_CursorFromChar( 'A'+li );
				}
			}
			if( ncsr>=0 ){
				TASKSW_SetNewCursor( hwnd ,ncsr );
				return (MRESULT)TRUE;
			}
		}
		break;
	 case WM_BUTTON1CLICK: case WM_BUTTON2CLICK: case WM_BUTTON3CLICK:
	 case WM_BUTTON1DBLCLK: case WM_BUTTON2DBLCLK: case WM_BUTTON3DBLCLK:
		if(SwBlock&&BufValid){ /* 初期化確認 */
			int ncsr = TASKSW_CursorFromPoint( (PPOINTS)(&mp1) );
			if( ncsr>=0 ){
				BOOL same = (ncsr==Cursor);
				/* マウスボタンが押し下げられた位置へカーソルを移動する */
				TASKSW_SetNewCursor( hwnd ,ncsr );
				/* ダブルクリック時は更に、「選択」を行う */
				if(same) switch(msg){
				 case WM_BUTTON1DBLCLK: case WM_BUTTON2DBLCLK: case WM_BUTTON3DBLCLK:
					WinSendMsg( hwnd ,WM_COMMAND
					 ,MPFROMSHORT(TASKCMD_SELECT) ,MPFROM2SHORT(CMDSRC_OTHER,FALSE) );
				}
				return (MRESULT)TRUE;
			}
		}
		break;
	 case WM_COMMAND:
		if(SwBlock&&BufValid){ /* 初期化確認 */
			int ncsr = Cursor;
			switch( COMMANDMSG(&msg)->cmd ){
			 case TASKCMD_SELECT:
				{
					const SWENTRY* swe = &SwBlock->aswentry[BufValid[Cursor]];
					HSWITCH hs = swe->hswitch;
					HWND hw = swe->swctl.hwnd;
					WinSwitchToProgram( hs );
					/* ↑だけでは、「隠す」されたウィンドウが表示されない事があるので、↓ */
					WinShowWindow( hw ,TRUE );
				}
				TASKSW_HIDE((TASKSW_PARAM*)NULL);
				break;
			 case TASKCMD_CANCEL:
				if( NumCancel>=0 ){
					ULONG style = WinQueryWindowULong( SwBlock->aswentry[NumCancel].swctl.hwnd ,QWL_STYLE );
					if( (style&WS_VISIBLE) && !(style&WS_MINIMIZED) ){
						/* 「取消」先が表示されている場合のみ、タスクを切り替える */
						WinSwitchToProgram( SwBlock->aswentry[NumCancel].hswitch );
					}
				}
				TASKSW_HIDE((TASKSW_PARAM*)NULL);
				break;
			 case TASKCMD_UP:
				if( (ncsr-=TASKW_ENUM_X)<0 ){
					if( (ncsr+=TASKW_ENUM_X*((CntValid-1)/TASKW_ENUM_X+1))>=CntValid ){
						ncsr = CntValid-1;
					}
				}
				break;
			 case TASKCMD_DOWN:
				if( (ncsr+=TASKW_ENUM_X)>=CntValid ){
					if( (ncsr-=TASKW_ENUM_X*((CntValid-1)/TASKW_ENUM_X+1))<0 ){
						ncsr = CntValid-1;
					}
				}
				break;
			 case TASKCMD_LEFT:
				if( (--ncsr)<0 ) ncsr = CntValid-1;
				break;
			 case TASKCMD_RIGHT:
				if( (++ncsr)>=CntValid ) ncsr = 0;
				break;
			}
			if( Cursor!=ncsr ){
				TASKSW_SetNewCursor( hwnd ,ncsr );
			}
		}
		break;
	 case WM_ACTIVATE:
		if( !SHORT1FROMMP(mp1) ){
			/* 非アクティブ化で、ウィンドウを非表示にしなければならない */
			/* （理由は前記関数 TASKSW_ISSHOWING のコメントを参照） */
			TASKSW_HIDE((TASKSW_PARAM*)NULL);
#if !MAKE_DLL
			/* デバッグ用コード。非アクティブ化したら自動終了 */
			WinPostMsg( hwnd ,WM_QUIT ,0 ,0 );
#endif
		}
		break;
	}
	return WinDefWindowProc( hwnd ,msg ,mp1 ,mp2 );
}


/* 関数名：TASKSW_AllocMem */
/* 機能　：タスクスイッチャ用作業用メモリの割り当て */
/* 引数　：２種類のバッファに割り振られるべきメモリサイズ */
/* 戻り値：正常終了で TRUE、異常終了で FALSE */
/* 備考　： */
static BOOL TASKSW_AllocMem( ULONG ssiz ,ULONG vsiz )
{
	BOOL rc;

	TASKSW_FreeMem(); /* 念のため、一旦メモリ解放 */
	rc = !DosAllocMem( (PPVOID)&SwBlock ,ssiz ,PAG_COMMIT|PAG_READ|PAG_WRITE )
	  && !DosAllocMem( (PPVOID)&BufValid ,vsiz ,PAG_COMMIT|PAG_READ|PAG_WRITE );
	if(!rc) TASKSW_FreeMem();

	return rc;
}


/* 関数名：TASKSW_FreeMem */
/* 機能　：タスクスイッチャ用作業用メモリの解放 */
/* 引数　：無し */
/* 戻り値：無し */
/* 備考　： */
static void TASKSW_FreeMem(void)
{
	DosFreeMem( SwBlock ); SwBlock = 0;
	DosFreeMem( BufValid ); BufValid = 0;
}


/* 関数名：TASKSW_CursorFromChar */
/* 機能　：タスク名の先頭キャラクタが一致するタスクのカーソル位置を返す */
/* 引数　：chr = 先頭キャラ(ASCII) */
/* 戻り値：カーソル位置もしくは -1（無効なキャラ） */
/* 備考　：現在のカーソル位置の次のタスク名から順に検索する */
static int TASKSW_CursorFromChar( UCHAR chr )
{
	int rc = -1;
	int ncsr = Cursor;
	int li;
	for( li = 0 ;li < CntValid ;li++ ){
		if(li!=0){
			UCHAR topc = SwBlock->aswentry[BufValid[ncsr]].swctl.szSwtitle[0];
			if( topc==chr || topc==(chr|0x20) ) break;
		}
		if( (++ncsr)>=CntValid ) ncsr = 0;
	}
	if( li < CntValid ){
		rc = ncsr;
	}
	return rc;
}


/* 関数名：TASKSW_CursorFromPoint */
/* 機能　：指定された座標におけるカーソル位置を返す */
/* 引数　：ptr = ポインター位置データのポインタ */
/* 戻り値：カーソル位置もしくは -1（無効な座標） */
/* 備考　： */
static int TASKSW_CursorFromPoint( PPOINTS ptr )
{
	int rc = -1;
	int lx;

	for( lx = 0 ;lx < TASKW_ENUM_X ;lx++ ){
		if( CSR_LEFT(lx) <= ptr->x && ptr->x <= CSR_RIGHT(lx) ){
			int ly;
			for( ly = 0 ;ly < CntValid ;ly+=TASKW_ENUM_X ){
				if( CSR_BOTTOM(CntValid,ly) <= ptr->y && ptr->y <= CSR_TOP(CntValid,ly) ){
					int nc = ly + lx;
					if( nc < CntValid ){ rc = nc; }
					break;
				}
			}
			break;
		}
	}

	return rc;
}


/* 関数名：TASKSW_SetNewCursor */
/* 機能　：カーソル位置の更新 */
/* 引数　：hwnd = ウィンドウ・ハンドル */
/* 戻り値：無し */
/* 備考　： */
static void TASKSW_SetNewCursor( HWND hwnd ,int ncsr )
{
	HPS hps = WinGetPS(hwnd);
	GpiCreateLogColorTable( hps ,0 ,LCOLF_RGB ,0 ,0 ,0 );
	if( Cursor != ncsr ){
		TASKSW_DrawCursor( hps ,FALSE );
		Cursor = ncsr;
	}
	TASKSW_DrawCursor( hps ,TRUE );
	WinReleasePS(hps);
}


/* 関数名：TASKSW_DrawCursor */
/* 機能　：タスクスイッチャ内のカーソル描画 */
/* 引数　：hps = 表示空間ハンドル */
/* 　　　：flg = 描画フラグ（TRUE=表示、FALSE=非表示） */
/* 戻り値：無し */
/* 備考　： */
static void TASKSW_DrawCursor( HPS hps ,BOOL flg )
{
	RECTL rect;


	rect.xLeft = CSR_LEFT(Cursor);
	rect.yBottom = CSR_BOTTOM(CntValid,Cursor);
	rect.xRight = CSR_RIGHT(Cursor);
	rect.yTop = CSR_TOP(CntValid,Cursor);

	if(!flg){
		TASKSW_Draw3DFrame( hps ,&rect ,2 ,DlgBkCol ,DlgBkCol );
	}
	else{
		TASKSW_Draw3DFrame( hps ,&rect ,2 ,CLR_DARKGRAY ,CLR_WHITE );
		WinSetWindowText( HwTaskText ,SwBlock->aswentry[BufValid[Cursor]].swctl.szSwtitle );
	}
}


/* 関数名：TASKSW_Draw3DFrame */
/* 機能　：３次元風矩形の描画 */
/* 引数　：hps = 表示空間ハンドル */
/* 　　　：rect = 描画座標 */
/* 　　　：width =  線幅 */
/* 　　　：col1 = 左および上の線の色 */
/* 　　　：col2 = 右および下の線の色 */
/* 戻り値：無し */
/* 備考　： */
static void TASKSW_Draw3DFrame( HPS hps ,PRECTL rect ,int width ,LONG col1 ,LONG col2 )
{
	int li;
	POINTL p[4];

	p[0].x = rect->xLeft;	p[0].y = rect->yTop;	/* 左上 */
	p[1].x = rect->xRight;	p[1].y = rect->yTop;	/* 右上 */
	p[2].x = rect->xRight;	p[2].y = rect->yBottom;	/* 右下 */
	p[3].x = rect->xLeft;	p[3].y = rect->yBottom;	/* 左下 */

	for( li = 0 ;li < width ;li++ ){
		GpiMove( hps ,&p[3] );
		GpiSetColor( hps ,col1 );
		GpiPolyLine( hps ,2 ,&p[0] );
		GpiSetColor( hps ,col2 );
		GpiPolyLine( hps ,2 ,&p[2] );

		p[0].x++; p[0].y--;	/* 左上 */
		p[1].x--; p[1].y--;	/* 右上 */
		p[2].x--; p[2].y++;	/* 右下 */
		p[3].x++; p[3].y++;	/* 左下 */
	}
}

