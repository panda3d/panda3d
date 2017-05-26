"""TaskManagerGlobal module: contains the global task manager"""

__all__ = ['taskMgr']

from . import Task

#: The global task manager.
taskMgr = Task.TaskManager()
