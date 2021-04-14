/* close.cmd ... REXXマクロ機能サンプル

	機能：他のセッションから ALWALTFX のREXX関数を呼び出すサンプル。
	　　　指定された名前を持つフォルダが開いていれば、閉じる。
	引数：フォルダ名（大小文字区別無し...末尾の「- アイコン表示」等は含めない）
	備考：先んじて ALWALTFX を実行させておく必要があります。
*/

/* REXX関数のロード */
CALL RxFuncAdd 'ArxLoadFuncs','ALWAREXX','ArxLoadFuncs'
CALL ArxLoadFuncs

/* ALWALTFX が実行中か否か確認 */
IF \ArxQueryAlwaltfxStatus('EXEC') THEN EXIT 1

OPTIONS EXMODE

PARSE UPPER ARG target
IF target='' THEN EXIT 1
IF LEFT(target,1)='"' & RIGHT(target,1)='"' THEN DO
	/* ダブルクォーテーションで囲われている引数対策 */
	target = SUBSTR(target,2,LENGTH(target)-2)
END

/* デスクトップ直下のウィンドウ・ハンドルを取得 */
IF \ArxEnumWindowHandle(ArxQueryWindowHandle('DESKTOP'),'enum') THEN EXIT 1

DO li = 1 TO enum.0
	CALL ArxQueryWindowInfo enum.li,'info'
	IF info.class='wpFolder window' THEN DO
		valid = 0
		IF TRANSLATE(info.title)=target THEN DO
			/* タイトル文字列と完全に一致 → 通常のフォルダ用の判定 */
			valid = 1
		END
		ELSE DO
			/* XFolder 対応 */
			len = LENGTH(target)
			IF TRANSLATE(LEFT(info.title,len))=target THEN DO
				IF SUBSTR(info.title,len+1,2)=' (' & RIGHT(info.title,1)=')' THEN DO
					valid = 1
				END
			END
		END

		IF valid THEN DO
			/* 「クローズ」メッセージを通知 */
			CALL ArxPostMessage enum.li,'H0021'/*WM_SYSCOMMAND*/ ,
			 ,'H8004'/*SC_CLOSE*/ ,
			 ,ArxMPFROM2SHORT(2/*CMDSRC_MENU*/,0/*FALSE*/)
		END
	END
END

EXIT 0
