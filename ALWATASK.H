#ifndef __alwatask_h
#define __alwatask_h

/* ALWALTFXから呼び出される関数には、全て第一引数に下記構造体のポインタが入る */
typedef struct{
	ULONG cbSize; /* この構造体のサイズ */
	HAB hab; /* アンカー・ブロック・ハンドル */
	HWND hwalwaltfx; /* ALWALTFX のメイン・ウィンドウ・ハンドル */
	HMODULE hmodule; /* DLLのモジュール・ハンドル */
} TASKSW_PARAM;

/* ALWALTFXが要求する、DLL側で用意すべき関数の定義 */
typedef ULONG (EXPENTRY FN_TASKSW_CREATE)(const TASKSW_PARAM*);
typedef BOOL (EXPENTRY FN_TASKSW_SHOW)(const TASKSW_PARAM*,PSWBLOCK,LONG,LONG);
typedef VOID (EXPENTRY FN_TASKSW_HIDE)(const TASKSW_PARAM*);
typedef BOOL (EXPENTRY FN_TASKSW_ISSHOWING)(const TASKSW_PARAM*);
typedef VOID (EXPENTRY FN_TASKSW_DESTROY)(const TASKSW_PARAM*);
typedef BOOL (EXPENTRY FN_TASKSW_SETTING)(const TASKSW_PARAM*);

FN_TASKSW_CREATE TASKSW_CREATE;
FN_TASKSW_SHOW TASKSW_SHOW;
FN_TASKSW_HIDE TASKSW_HIDE;
FN_TASKSW_ISSHOWING TASKSW_ISSHOWING;
FN_TASKSW_DESTROY TASKSW_DESTROY;
FN_TASKSW_SETTING TASKSW_SETTING;

#define STR_TASKSW_CREATE "TASKSW_CREATE"
#define STR_TASKSW_SHOW "TASKSW_SHOW"
#define STR_TASKSW_HIDE "TASKSW_HIDE"
#define STR_TASKSW_ISSHOWING "TASKSW_ISSHOWING"
#define STR_TASKSW_DESTROY "TASKSW_DESTROY"
#define STR_TASKSW_SETTING "TASKSW_SETTING"

#endif /*__alwatask_h*/
