# makeexe.mak ... デバッグ用EXEファイル作成用メイクファイル
DEBUG	=	0 # ← 1 にすると、デバッグ情報が組み込まれます

# Ｃコンパイラ、リンカ、リソースコンパイラ実行ファイル名の指定
CC		=	icc.exe
LINK	=	link386.exe
RC		=	rc.exe

# Ｃコンパイルオプション
# 順に、ロゴ表示無し、DBCS使用、//コメント使用、マルチスレッド、EXE作成、
# 各種ワーニングメッセージ出力指定
CFLAGS	=	/Q /Sn /Ss /Gm /Ge+ \
			/Wall /Wppt- /Wgnr- /Wcpy- /Wcls- /Wtrd- /Wvft- /Wcmp- /Wuni- /Wobs-
# リンクオプション
# 順に、ロゴ無し、バッチ処理、大小文字区別有り、PMアプリ、
# 各種最適化オプション
LFLAGS  =   /NOLOGO /BATCH /NOI /PMTYPE:PM \
			/A:2 /F /E /PACKC /PACKD


!if $(DEBUG)==0
# デバッグ指定無し時の追加オプション
# Ｃコンパイルオプション：最適化有り（VAC++の場合、指定しない方が良いかも）
CFLAGS  =   /O $(CFLAGS)
!else
# デバッグ指定有り時の追加オプション
# Ｃコンパイルオプション：デバッグ情報挿入、デバッグ用メモリ管理関数の使用
# リンカオプション：デバッグ情報挿入
CFLAGS  =   /Ti /Tm $(CFLAGS)
LFLAGS  =   /DEBUG $(LFLAGS)
!endif

# ターゲット及び依存ファイルの定義
TARGET	=	test.exe
OBJS	=	test.obj alwatask.obj alwatsks.obj
RES		=	alwatskd.res
MAP		=	nul
LIBS	=	
DEF		=	nul

CFLAGS	=	$(CFLAGS) /DMAKE_DLL=0

.SUFFIXES:
.SUFFIXES:	.c

$(TARGET):	$(OBJS) $(DEF) $(RES)
    $(LINK) $(LFLAGS) @<<
$(OBJS)
$@
$(MAP)
$(LIBS)
$(DEF)
<<
	$(RC) $(RES) $@

.c.obj:
	$(CC) /C $(CFLAGS) $?

test.c:	alwatask.h
alwatask.c:	alwatask.h alwatsks.h alwatskd.h
alwatsks.c:	alwatsks.h alwatskd.h

#[EOF]
