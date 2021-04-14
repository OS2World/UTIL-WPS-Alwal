#ifndef __alwatsks_h
#define __alwatsks_h

void TASKSETTING_Init( HAB hab );
void TASKSETTING_Term( HAB hab );

FNWP TASKSETTING_DlgProc;

/* タスクスイッチ・ウィンドウ設定：非表示タスクタイトル */
typedef struct{
	char str[MAXNAMEL+4];
} IGNTASK_SINGLE;

typedef struct{
	int cnt;
	IGNTASK_SINGLE* buf;
} IGNTASK_BAG;

int IGNTASK_IsExist( const char* str );

extern IGNTASK_BAG IgnTask;

#endif /*__alwatsks_h*/
