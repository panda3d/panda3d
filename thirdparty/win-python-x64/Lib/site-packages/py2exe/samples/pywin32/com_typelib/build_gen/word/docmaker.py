# A test program that generates a word document.
import sys
import os
from win32com.client import gencache

# When built with py2exe, it assumes 'Word.Application.9' is installed
# on the machine performing the build.  The typelibs from the local machine
# will be used to generate makepy files, and those generated files will
# be included in the py2exe library (generally in a .zip file)

# The resulting application should run without referencing any typelibs on
# the target system.

# It will create a file:
filename = os.path.abspath(
              os.path.join(os.path.dirname(sys.argv[0]), "output.doc"))

word = gencache.EnsureDispatch("Word.Application.9")

# For the sake of ensuring the correct module is used...
mod = sys.modules[word.__module__]
print "The module hosting the object is", mod


word.Visible = 1
doc = word.Documents.Add()
wrange = doc.Range()
for i in range(10):
    wrange.InsertAfter("Hello from py2exe %d\n" % i)
doc.SaveAs(filename)
word.Quit()
print "Done - saved to", os.path.abspath(filename)
