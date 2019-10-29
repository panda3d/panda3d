""" This module serves as a placeholder for the global AppRunner
object, which only exists when we are running a .p3d file via
runp3d.py or via the Panda3D plugin or standalone executable.

This is needed for apps that start themselves by importing
DirectStart; it provides a place for these apps to look for
the AppRunner at startup.

.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""

#: Contains the global :class:`~.AppRunner.AppRunner` instance, or None
#: if this application was not run from the runtime environment.
appRunner = None
