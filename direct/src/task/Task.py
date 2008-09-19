""" This module exists temporarily as a gatekeeper between
TaskOrig.py, the original Python implementation of the task system,
and TaskNew.py, the new C++ implementation. """

wantNewTasks = False
if __debug__:
    from pandac.PandaModules import ConfigVariableBool
    wantNewTasks = ConfigVariableBool('want-new-tasks', False).getValue()

if wantNewTasks:
    from TaskNew import *
else:
    from TaskOrig import *
