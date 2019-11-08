"""Instantiates the global :class:`~.Messenger.Messenger` object."""

__all__ = ['messenger']

from . import Messenger

#: Contains the global :class:`~.Messenger.Messenger` instance.
messenger = Messenger.Messenger()
