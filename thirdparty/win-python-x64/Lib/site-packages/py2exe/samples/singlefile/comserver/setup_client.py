# This setup script builds a single-file Python inprocess COM server.
#
from distutils.core import setup
import py2exe
import sys

# If run without args, build executables, in quiet mode.
if len(sys.argv) == 1:
    sys.argv.append("py2exe")
    sys.argv.append("-q")

################################################################
# pywin32 COM pulls in a lot of stuff which we don't want or need.

excludes = ["pywin", "pywin.debugger", "pywin.debugger.dbgcon",
            "pywin.dialogs", "pywin.dialogs.list", "win32com.server"]

options = {
    "bundle_files": 1, # create singlefile exe
    "compressed": 1, # compress the library archive
    "excludes": excludes,
    "dll_excludes": ["w9xpopen.exe"] # we don't need this
    }

setup(
    options = {"py2exe": options},
    zipfile = None, # append zip-archive to the executable.
    console = ["test.py"]
    )
