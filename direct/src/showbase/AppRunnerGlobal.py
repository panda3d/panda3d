""" This module serves as a placeholder for the global AppRunner
object, which only exists when we are running a .p3d file via
runp3d.py or via the Panda3D plugin or standalone executable.

This is needed for apps that start themselves by importing
DirectStart; it provides a place for these apps to look for
the AppRunner at startup. """

#: Contains the global AppRunner instance, or None if this application
#: was not run from the runtime environment.
appRunner = None
