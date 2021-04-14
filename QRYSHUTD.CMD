/* qryshutd.cmd ... REXXマクロ機能サンプル

	機能：デスクトップまたはワープセンター及びそのプルダウンメニューにフォー
	　　　カスが当たっている時、「OS/2の終了」ダイアログを表示する。
	引数：無し。
	備考：「キー操作拡張：REXXマクロ」機能に割り当てて使用します。
	　　　「Alt+F4」キー操作にこのマクロを割り当てると、シャットダウン操作が
	　　　Windows互換っぽくなります？

	　　　　※「Alt+F4」操作は、本来PMで「クローズ」操作に予約？されている
	　　　　　ためか、割り当てても ALWALTFX が上手くキー入力を拾えない事が
	　　　　　あります...まぁご愛敬という事で...(^^;ゞ
*/

/* ウィンドウ情報取得 */
IF ArxQueryWindowInfo('MAIN','info') THEN DO
	/* フォーカス・ウィンドウはフォルダにかどうか確認 */
	IF ArxSysCommand('FOLDER?') THEN DO
		/* デスクトップ・フォルダかどうか確認する。
		   判別するための明示的な手段は無い（と思う）ため、ウィンドウ・ス
		   タイル値に含まれている、フレームウィンドウ・ボーダーの種類で判
		   別を行っている。
		   （細線ボーダー → デスクトップ・フォルダ、それ以外 → その他の
		   フォルダ） */
		IF ArxAND(info.STYLE,'H00000980','H')='H00000100' THEN DO
			/* デスクトップ・フォルダである */
			/*  → 「OS/2の終了」メッセージを通知する */
			WM_COMMAND = 'H0020'
			CMDSRC_MENU = 2
			CALL ArxPostMessage 'MAIN',WM_COMMAND,'H02C0',CMDSRC_MENU
			EXIT 0
		END
	END
	/* フォーカス・ウィンドウがフォルダ以外 → ワープセンターであるかどう
	   か確認 */
	ELSE DO
		hwnd = SearchWarpCenter(info.CLASS)
		IF hwnd\='' THEN DO
			/* ワープセンターである */
			/* → 「OS/2の終了」ボタン上にポインターを移動させ、クリック */
			CALL ArxQueryWindowInfo hwnd,'info'
			CALL ArxSetPointerPos info.CY*5.5,info.Y+8
			CALL ArxPutMouseButton 1
			CALL ArxAlarm 'WARNING'
			EXIT 0
		END
	END
END

CALL ArxReplaceKey 'DEFAULT'
EXIT 0


/* 現在のウィンドウがワープセンター（と推測できる）かどうか確認 */
SearchWarpCenter: PROCEDURE
	hwnd = ''
	IF ARG(1)='#3' THEN DO
		CALL ArxQueryWindowString 'MAIN','str'
		IF str='SmartCenter' THEN DO
			hwnd = 'MAIN'
		END
		ELSE
		IF ArxSysCommand('WPSWIN?') THEN DO
			hwparent  = ArxQueryWindowHandle('PARENT')
			hwdesktop = ArxQueryWindowHandle('DESKTOP')
			IF hwparent=hwdesktop THEN DO
				CALL ArxEnumWindowHandle 'DESKTOP','hwenum'
				DO li = 1 TO hwenum.0
					CALL ArxQueryWindowString hwenum.li,'str'
					IF str='SmartCenter' THEN DO
						hwnd = hwenum.li
						LEAVE
					END
				END
			END
		END
	END
RETURN hwnd
