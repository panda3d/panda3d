"""
This is a shortcut that instantiates ShowBase automatically on import,
opening a graphical window and setting up the scene graph.
This example demonstrates its use:

   import direct.directbase.DirectStart
   run()

While it may be considered useful for quick prototyping in the interactive
Python shell, using it in applications is not considered good style.
As such, it has been deprecated starting with Panda3D 1.9.  It is equivalent
to and may be replaced by the following code:

   from direct.showbase.ShowBase import ShowBase
   base = ShowBase()
"""

__all__ = []

if __debug__:
    print('Using deprecated DirectStart interface.')

from direct.showbase import ShowBase
base = ShowBase.ShowBase()
