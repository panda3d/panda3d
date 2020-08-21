"""Contains the global :class:`~.Task.TaskManager` object."""

__all__ = ['taskMgr']

from . import Task

#: The global task manager.
taskMgr = Task.TaskManager()
