/* menuspy.cmd ... REXXマクロ機能サンプル

	機能：作業用メモリ関数を用いたサンプル。
	　　　メニュー・ウィンドウにフォーカスが当たっている時に、その
	　　　項目情報を列挙する。
	引数：無し。
	備考：例えば「キー操作拡張：REXXマクロ」機能に割り当てて使用します。
	　　　（結果は「出力モニタ」ウィンドウにて表示される）
	　　　メニュー項目のIDが解れば、ArxSysCommand 関数を用いて、そのメ
	　　　ニュー項目を選択したのと同等の効果を及ぼすREXXマクロを作る際
	　　　のヒントになるでしょう。
*/

/* メニュー・ウィンドウに関する各種シンボルの定義 */
MM_QUERYITEM          = 'H0182'
MM_QUERYITEMCOUNT     = 'H0184'
MM_QUERYITEMTEXT      = 'H018B'
MM_ITEMIDFROMPOSITION = 'H0190'

MIS_TEXT              = 'H0001'
MIS_BITMAP            = 'H0002'
MIS_SEPARATOR         = 'H0004'
MIS_OWNERDRAW         = 'H0008'
MIS_SUBMENU           = 'H0010'
MIS_MULTMENU          = 'H0020'
MIS_SYSCOMMAND        = 'H0040'
MIS_HELP              = 'H0080'
MIS_STATIC            = 'H0100'
MIS_BUTTONSEPARATOR   = 'H0200'
MIS_BREAK             = 'H0400'
MIS_BREAKSEPARATOR    = 'H0800'
MIS_GROUP             = 'H1000'
MIS_SINGLE            = 'H2000'

MIA_NODISMISS         = 'H0020'
MIA_FRAMED            = 'H1000'
MIA_CHECKED           = 'H2000'
MIA_DISABLED          = 'H4000'
MIA_HILITED           = 'H8000'

/* 現在のフォーカス・ウィンドウの種類をチェック */
CALL ArxQueryWindowInfo 'FOCUS' ,'info'
IF info.CLASS\='#4' THEN EXIT 0

/* メニューである → 項目数を参照する */
cnt = X2D(SUBSTR(ArxSendMessage('FOCUS',MM_QUERYITEMCOUNT,0,0),2))

IF cnt = 0 THEN EXIT 0

/* 項目が１つ以上ある → 十分な作業用メモリを確保して、参照準備 */
memsize = 256
IF \ArxAllocTempMem(memsize) THEN EXIT 1

DO li = 0 TO cnt-1
	SAY 'No.' li '________'

	/* li番目のメニュー項目のIDを取得 */
	CALL CHAROUT 'STDOUT:','ID = '
	rc = ArxSendMessage('FOCUS',MM_ITEMIDFROMPOSITION,li,0)
	id = ArxD2X(ArxSHORT1FROMMP(rc),4)
	IF id='HFFFF' THEN EXIT 1 /* MIT_ERROR */
	SAY id

	/* メニュー項目テキスト取得 */
	CALL CHAROUT 'STDOUT:','TEXT = '
	mp1 = ArxMPFROM2SHORT(id,memsize)
	rc = ArxSendMessage('FOCUS',MM_QUERYITEMTEXT,mp1,'TEMPMEM')
	len = ArxX2D(rc)
	if len=0 THEN DO
		SAY '(nothing)'
	END
	ELSE DO
		/* 長さ 0 以上 → 作業用メモリから取得 */
		IF \ArxQueryTempMemImage('image') THEN EXIT 1
		SAY '"'LEFT(image,len)'"'
	END

	/* メニュー項目情報を取得 */
	mp1 = id
	rc = ArxSendMessage('FOCUS',MM_QUERYITEM,mp1,'TEMPMEM')
	IF ArxX2D(rc)=0 THEN EXIT 1
	IF \ArxQueryTempMemImage('image') THEN EXIT 1
	/* メニュー項目スタイル */
	afStyle = ArxC2X(SUBSTR(image,4,1)||SUBSTR(image,3,1))
	SAY 'MENUITEM.afStyle = 'afStyle
	sc = '09'x
	IF ArxAND(afStyle,MIS_TEXT)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_TEXT'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_BITMAP)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_BITMAP'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_SEPARATOR)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_SEPARATOR'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_OWNERDRAW)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_OWNERDRAW'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_SUBMENU)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_SUBMENU'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_MULTMENU)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_MULTMENU'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_SYSCOMMAND)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_SYSCOMMAND'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_HELP)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_HELP'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_STATIC)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_STATIC'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_BUTTONSEPARATOR)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_BUTTONSEPARATOR'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_BREAK)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_BREAK'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_BREAKSEPARATOR)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_BREAKSEPARATOR'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_GROUP)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_GROUP'; sc = ' | '
	END
	IF ArxAND(afStyle,MIS_SINGLE)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIS_SINGLE'; sc = ' | '
	END
	SAY
	/* メニュー項目属性 */
	afAttribute = ArxC2X(SUBSTR(image,6,1)||SUBSTR(image,5,1))
	SAY 'MENUITEM.afAttribute = 'afAttribute
	sc = '09'x
	IF ArxAND(afAttribute,MIA_NODISMISS)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIA_NODISMISS'; sc = ' | '
	END
	IF ArxAND(afAttribute,MIA_FRAMED)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIA_FRAMED'; sc = ' | '
	END
	IF ArxAND(afAttribute,MIA_CHECKED)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIA_CHECKED'; sc = ' | '
	END
	IF ArxAND(afAttribute,MIA_DISABLED)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIA_DISABLED'; sc = ' | '
	END
	IF ArxAND(afAttribute,MIA_HILITED)\=0 THEN DO
		CALL CHAROUT 'STDOUT:',sc'MIA_HILITED'; sc = ' | '
	END
	SAY
	/* メニュー項目サブメニューウィンドウ・ハンドル */
	hwndSubMenu = ArxC2X( SUBSTR(image,12,1) || SUBSTR(image,11,1) ,
	 || SUBSTR(image,10,1) || SUBSTR(image,9,1) )
	SAY 'MENUITEM.hwndSubMenu = 'hwndSubMenu
	/* メニュー項目アイテム・ハンドル */
	hItem = ArxC2X( SUBSTR(image,16,1) || SUBSTR(image,15,1) ,
	 || SUBSTR(image,14,1) || SUBSTR(image,13,1) )
	SAY 'MENUITEM.hItem = 'hItem
	SAY '~~~~~~~~'
END

EXIT 0
