""" This is a deprecated module that creates a global instance of ShowBase. """

__all__ = []

if __debug__:
    print('Using deprecated DirectStart interface.')

from direct.showbase import ShowBase
base = ShowBase.ShowBase()
