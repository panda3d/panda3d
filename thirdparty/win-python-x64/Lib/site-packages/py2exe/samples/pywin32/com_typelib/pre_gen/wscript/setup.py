# Execute this as 'setup.py py2exe' to create a .exe from docmaker.py
from distutils.core import setup
import py2exe

py2exe_options = dict(
    typelibs = [
        # typelib for 'Windows Script Host Object Model', which we
        # have pre-generated into wsh-typelib-stubs.py
        ('{F935DC20-1CF0-11D0-ADB9-00C04FD58A0B}', 0, 1, 0, 'wsh-typelib-stubs.py'),
    ]
)

setup(name="SpamBayes",
      console=["show_info.py"],
      options = {"py2exe" : py2exe_options},
)
