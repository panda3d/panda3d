# This setup script builds a single-file Python inprocess COM server.
#
from distutils.core import setup
import py2exe
import sys

# If run without args, build executables, in quiet mode.
if len(sys.argv) == 1:
    sys.argv.append("py2exe")
    sys.argv.append("-q")

class Target:
    def __init__(self, **kw):
        self.__dict__.update(kw)
        # for the versioninfo resources
        self.version = "0.6.0"
        self.company_name = "No Company"
        self.copyright = "no copyright"
        self.name = "py2exe sample files"

################################################################
# a COM server dll, modules is required
#

interp = Target(
    description = "Python Interpreter as COM server module",
    # what to build.  For COM servers, the module name (not the
    # filename) must be specified!
    modules = ["win32com.servers.interp"],
    # we only want the inproc server.
    create_exe = False,
    )

################################################################
# pywin32 COM pulls in a lot of stuff which we don't want or need.

excludes = ["pywin", "pywin.debugger", "pywin.debugger.dbgcon",
            "pywin.dialogs", "pywin.dialogs.list", "win32com.client"]

options = {
    "bundle_files": 1,
    "ascii": 1, # to make a smaller executable, don't include the encodings
    "compressed": 1, # compress the library archive
    "excludes": excludes, # COM stuff we don't want
    }

setup(
    options = {"py2exe": options},
    zipfile = None, # append zip-archive to the executable.
    com_server = [interp],
    )
