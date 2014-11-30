# setup_interp.py
# A distutils setup script for the "interp" sample.

from distutils.core import setup
import py2exe

# Don't pull in all this MFC stuff used by the makepy UI.
py2exe_options = dict(excludes="pywin,pywin.dialogs,pywin.dialogs.list,win32ui")

setup(name="win32com 'interp' sample",
      com_server=["win32com.servers.interp"],
      options = dict(py2exe=py2exe_options)
)
