/* alextsrt.cmd ... REXXマクロ機能サンプル

	機能：フォルダ上での「分類.名前」操作を、「分類.拡張子」に置換する。
	引数：無し。
	備考：「ウィンドウ制御：REXXマクロ」機能の「WM_COMMAND」に割り当てて使
	　　　用します。
	　　　割り当てると、副作用として、普通の「分類.名前」が行えなくなって
	　　　しまうので注意。(^^;)
*/
SORTOBJNAME = 'H00001770' /* 「分類.名前」のコマンドＩＤ */

IF ArxSysCommand('FOLDER?') THEN DO
	CALL ArxQueryMsgParam 'msg'
	IF msg.MP1=SORTOBJNAME THEN DO
		CALL ArxReplaceCommand 'SORTEXT'
	END
END

EXIT 0
