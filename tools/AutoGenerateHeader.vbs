wscript.echo "Generating Headers..."

Set WshShell = CreateObject("Wscript.Shell")
Set fso = CreateObject("Scripting.FileSystemObject")

Dim outdir
Set outdir = Nothing
Dim folder
Set folder = Nothing

For i = 0 To WScript.Arguments.Count - 1
	If WScript.Arguments(i) = "-o" Then
		i = i + 1
		If Not fso.FolderExists(WScript.Arguments(i)) Then
			fso.CreateFolder(WScript.Arguments(i))
		End If
		Set outdir = fso.GetFolder(WScript.Arguments(i))
	ElseIf WScript.Arguments(i) = "-f" Then
		i = i + 1
		If Not fso.FolderExists(WScript.Arguments(i)) Then
			WScript.Echo "folder " & WScript.Arguments(i) & " does not exist. Exiting..."
			WScript.Quit 1
		End If
		Set folder = fso.GetFolder(WScript.Arguments(i))
	ElseIf WScript.Arguments(i) = "-h" Then
		' Todo: help
	Else
		WScript.Echo "invalid argument " & WScript.Arguments(i)
	End If
Next

Set scriptFile = fso.GetFile(WScript.ScriptFullName)

If outdir Is Nothing Then
	Set outdir = scriptfile.ParentFolder
End If

If folder Is Nothing Then
	Set folder = scriptFile.ParentFolder
End If

Dim generateonly
generateonly = False

If outdir.Path = folder.Path Then
	generateonly = True
End If

For Each file In folder.Files
	If fso.GetExtensionName(file.Path) = "h" Then
		procedureSingleFile(file.Path)
	End If
Next

wscript.echo "Header Generation Complete"

WScript.Quit 0

Sub procedureSingleFile(fileName)
	WScript.Echo "Copying file " & fileName
	Dim outPath
	outPath = outdir.Path
	If Right(path, 1) <> "\" Then outPath = outPath & "\"
	If Not generateonly then
		fso.CopyFile fileName, outPath & fso.GetFileName(fileName)
	End if
	Set fs = fso.OpenTextFile(fileName)
	Do While Not fs.AtEndOfStream
		line = fs.ReadLine
		strings = Split(line, " ")
		If UBound(strings) >= 2 then
			isClass = (strings(0) = "class") Or (strings(0) = "struct")
			isExport = (Right(strings(1), 7) = "_EXPORT")
			If isClass And isExport Then
				truncatedLine = strings(2)
				colonPos = InStr(truncatedLine, ":")
				Dim generatedFileName
				If colonPos = 0 Then generatedFileName = Trim(truncatedline) Else generatedFileName = Trim(Left(truncatedLine, colonPos - 1))
				Dim path
				path = outPath & generatedFileName
				wscript.echo "Generating Header for " & generatedFileName
				Set generatedFile = fso.CreateTextFile(path, True, False)
				generatedLine = "#include """ & fso.GetFileName(fileName) & """"
				generatedFile.WriteLine generatedLine
				generatedFile.Close
			End If
		End If
	Loop
	fs.close
End Sub
