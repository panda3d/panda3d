# Execute this as 'setup.py py2exe' to create a .exe from docmaker.py
from distutils.core import setup
import py2exe

py2exe_options = dict(
    typelibs = [
        # typelib for 'Word.Application.8' - execute 
        # 'win32com/client/makepy.py -i' to find a typelib.
        ('{00020905-0000-0000-C000-000000000046}', 0, 8, 1),
    ]
)

setup(name="SpamBayes",
      console=["docmaker.py"],
      options = {"py2exe" : py2exe_options},
)
