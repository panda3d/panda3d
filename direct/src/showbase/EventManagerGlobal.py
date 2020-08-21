"""Contains the global :class:`.EventManager` instance."""

__all__ = ['eventMgr']

from . import EventManager

#: The global event manager.
eventMgr = EventManager.EventManager()
