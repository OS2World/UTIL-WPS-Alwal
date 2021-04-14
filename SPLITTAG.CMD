/* splittag.cmd */
/*

　これは、HTMLファイル...例えば「REXXマクロリファレンス」（REXXMAC.HTM）を、
プレーンテキストファイルに変換するREXXスクリプトです。

　使い方は、コマンド・プロンプトにて、例えば以下のようにコマンド実行します。

[C:\ALWALTFX]SPLITTAG.CMD < REXXMAC.HTM > REXXMAC.DOC

*/
OPTIONS EXMODE
DO WHILE LINES()
	src = LINEIN()
	src = REPLACESTR(src,'<!--','')
	src = REPLACESTR(src,'-->','')
	src = REPLACESTR(src,'&quot','"')
	src = REPLACESTR(src,'&amp','&')
	src = REPLACESTR(src,'&copy','(C)')
	src = REPLACESTR(src,'&reg','(R)')
	DO FOREVER
		tp = POS('<',src)
		bp = POS('>',src)
		IF tp=0 | bp=0 THEN LEAVE
		src = DELSTR(src,tp,bp-tp+1)
	END
	src = REPLACESTR(src,'&lt','<')
	src = REPLACESTR(src,'&gt','>')
	CALL SPLITPUTLINE src
END
EXIT 0

REPLACESTR: PROCEDURE
	PARSE ARG src,tgt,rpl
	DO FOREVER
		cp = POS(tgt,src)
		IF cp=0 THEN cp = POS(TRANSLATE(tgt),src)
		IF cp=0 THEN LEAVE
		src = LEFT(src,cp-1) || rpl || SUBSTR(src,cp+LENGTH(tgt))
	END
RETURN src

SPLITPUTLINE: PROCEDURE
	distop = '!)?]}、。，．・？！゛゜ヽヾゝゞ々）］｝」』' /* 行頭禁則文字 */
	disbtm = '([{（［｛「『' /* 行末禁則文字 */

	PARSE ARG src

	DO FOREVER
		/* 74字前/後で分離 */
		fs = ''
		DO WHILE LENGTH(fs)<72
			IF src='' THEN LEAVE
			OPTIONS EXMODE
			fs = fs || LEFT(src,1)
			src = SUBSTR(src,2)
			OPTIONS NOEXMODE
		END
		OPTIONS EXMODE
		DO WHILE fs\=''
			/* 前部分に行末禁則文字 → 後部分に送る */
			c = RIGHT(fs,1)
			IF VERIFY(c,disbtm) THEN LEAVE
			fs = DELSTR(fs,LENGTH(fs))
			src = c || src
		END
		DO WHILE src\=''
			/* 後部分に行頭禁則文字 → 前部分に送る */
			c = LEFT(src,1)
			IF VERIFY(c,distop) THEN LEAVE
			fs = fs || c
			src = SUBSTR(src,2)
		END
		OPTIONS NOEXMODE
		/* 出力 */
		SAY fs
		/* 後部分の長さ０ → 終わり */
		IF src='' THEN LEAVE
	END

RETURN
