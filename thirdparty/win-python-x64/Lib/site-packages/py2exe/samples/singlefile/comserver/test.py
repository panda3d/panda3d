import sys
from win32com.client import Dispatch

d = Dispatch("Python.Interpreter")
print "2 + 5 =", d.Eval("2 + 5")
d.Exec("print 'hi via COM'")

d.Exec("import sys")

print "COM server sys.version", d.Eval("sys.version")
print "Client sys.version", sys.version

print "COM server sys.path", d.Eval("sys.path")
print "Client sys.path", sys.path
