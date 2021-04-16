strOpenPath = WScript.Arguments.Item(0)
strSavePath = WScript.Arguments.Item(1)
Set InvApp = CreateObject("Inventor.Application")
InvApp.Visible = False
InvApp.SilentOperation = True
InvApp.Documents.Open strOpenPath, True
Set oDoc = InvApp.ActiveDocument
oDoc.SaveAs strSavePath, True
InvApp.Quit
