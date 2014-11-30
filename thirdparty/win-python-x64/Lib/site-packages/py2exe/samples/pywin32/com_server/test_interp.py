# A Python program that uses our binaries!
# (Note that this is *not* transformed into a .exe - its an example consumer 
# of our object, not part of our object)

from win32com.client import Dispatch
import pprint

interp = Dispatch("Python.Interpreter")
interp.Exec("import sys")

print "The COM object is being hosted in ", interp.Eval("sys.executable")
print "Path is:", interp.Eval("sys.path")
