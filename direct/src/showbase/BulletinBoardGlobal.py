"""Instantiates the global :class:`~.BulletinBoard.BulletinBoard` object."""

__all__ = ['bulletinBoard']

from . import BulletinBoard

#: The global :class:`~.BulletinBoard.BulletinBoard` object.
bulletinBoard = BulletinBoard.BulletinBoard()
