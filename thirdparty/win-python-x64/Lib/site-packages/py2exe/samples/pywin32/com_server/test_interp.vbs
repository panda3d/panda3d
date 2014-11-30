rem A VBScript program that uses the py2exe created COM object.
rem Register the object (see README.txt), then execute:
rem cscript.exe test_interp.vbs
set interp = CreateObject("Python.Interpreter")
interp.Exec "import sys"

WScript.Echo("The COM object is being hosted in " & interp.Eval("sys.executable"))
WScript.Echo("Path is:" & interp.Eval("str(sys.path)"))
