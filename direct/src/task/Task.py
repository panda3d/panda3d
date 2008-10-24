""" This module exists temporarily as a gatekeeper between
TaskOrig.py, the original Python implementation of the task system,
and TaskNew.py, the new C++ implementation. """

from pandac.libpandaexpressModules import ConfigVariableBool
wantNewTasks = ConfigVariableBool('want-new-tasks', True).getValue()

if wantNewTasks:
    from TaskNew import *
else:
    from TaskOrig import *
