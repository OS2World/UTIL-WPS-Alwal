/* wpsetup.cmd ... REXXマクロ機能サンプル

	機能：旧「Workplaceオブジェクト設定」機能の代替、Workplace
	　　　オブジェクトに対して、セットアップストリングを送付する。
	引数：オブジェクトID、セットアップストリング（複数指定可）
	　　　（引数の区切りは、空白で挟んだカンマ記号 ' , ' を用いる）
*/
CALL RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
CALL SysLoadFuncs

/* 引数を確認 → 不足していたらエラー扱い */
PARSE ARG ObjectID' , 'SetupStrings
IF SetupStrings='' THEN EXIT 1

ObjectID = STRIP(ObjectID)

/* 引数で指定された数だけ、オブジェクト設定を繰り返す */
IF LEFT(ObjectID,1)='<' THEN DO
	/* オブジェクトIDの１字目が'<' → SysSetObjectData を使用 */
	DO WHILE SetupStrings\=''
		PARSE VALUE SetupStrings WITH SetStr' , 'SetupStrings
		CALL SysSetObjectData ObjectID,STRIP(SetStr)
	END
END
ELSE DO
	/* オブジェクトIDの１字目が'<'以外 → パス名直接指定と判断、 */
	/* その場合、SysSetObjectData では上手く動作しない可能性が   */
	/* あるので、SysCreateObject を使用 */
	Class = 'WPObject'
	Title = FILESPEC('N',ObjectID)
	Location = FILESPEC('D',ObjectID) || FILESPEC('P',ObjectID)
	IF FILESPEC('P',Location)\='\' THEN
		Location = DELSTR(Location,LENGTH(Location))
	DO WHILE SetupStrings\=''
		PARSE VALUE SetupStrings WITH SetStr' , 'SetupStrings
		CALL SysCreateObject Class,Title,Location,STRIP(SetStr),'U'
	END
END

EXIT 0
