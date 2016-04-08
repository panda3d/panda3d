"""TaskManagerGlobal module: contains the global task manager"""

__all__ = ['taskMgr']

from . import Task

taskMgr = Task.TaskManager()
