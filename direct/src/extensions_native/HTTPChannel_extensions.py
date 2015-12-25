####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

from panda3d import core
from .extension_native_helpers import Dtool_funcToMethod

"""
HTTPChannel-extensions module: contains methods to extend functionality
of the HTTPChannel class
"""

def spawnTask(self, name = None, callback = None, extraArgs = []):
        """Spawns a task to service the download recently requested
        via beginGetDocument(), etc., and/or downloadToFile() or
        downloadToRam().  If a callback is specified, that function is
        called when the download is complete, passing in the extraArgs
        given.

        Returns the newly-spawned task.
        """
        if not name:
            name = str(self.getUrl())
        from direct.task import Task
        task = Task.Task(self.doTask)
        task.callback = callback
        task.callbackArgs = extraArgs
        return taskMgr.add(task, name)

if hasattr(core, 'HTTPChannel'):
    Dtool_funcToMethod(spawnTask, core.HTTPChannel)
del spawnTask
#####################################################################

def doTask(self, task):
        from direct.task import Task
        if self.run():
            return Task.cont
        if task.callback:
            task.callback(*task.callbackArgs)
        return Task.done

if hasattr(core, 'HTTPChannel'):
    Dtool_funcToMethod(doTask, core.HTTPChannel)
del doTask
#####################################################################
