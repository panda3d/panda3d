"""
DirectNotify module: this module contains the DirectNotify class
"""

from __future__ import annotations

from panda3d.core import StreamWriter

from . import Notifier
from . import Logger


class DirectNotify:
    """
    DirectNotify class: this class contains methods for creating
    mulitple notify categories via a dictionary of Notifiers.
    """

    def __init__(self) -> None:
        """
        DirectNotify class keeps a dictionary of Notfiers
        """
        self.__categories: dict[str, Notifier.Notifier] = {}
        # create a default log file
        self.logger = Logger.Logger()

        # This will get filled in later by ShowBase.py with a
        # C++-level StreamWriter object for writing to standard
        # output.
        self.streamWriter: StreamWriter | None = None

    def __str__(self) -> str:
        """
        Print handling routine
        """
        return "DirectNotify categories: %s" % (self.__categories)

    #getters and setters
    def getCategories(self) -> list[str]:
        """
        Return list of category dictionary keys
        """
        return list(self.__categories.keys())

    def getCategory(self, categoryName: str) -> Notifier.Notifier | None:
        """getCategory(self, string)
        Return the category with given name if present, None otherwise
        """
        return self.__categories.get(categoryName, None)

    def newCategory(self, categoryName: str, logger: Logger.Logger | None = None) -> Notifier.Notifier:
        """newCategory(self, string)
        Make a new notify category named categoryName. Return new category
        if no such category exists, else return existing category
        """
        if categoryName not in self.__categories:
            self.__categories[categoryName] = Notifier.Notifier(categoryName, logger)
            self.setDconfigLevel(categoryName)
        notifier = self.getCategory(categoryName)
        assert notifier is not None
        return notifier

    def setDconfigLevel(self, categoryName: str) -> None:
        """
        Check to see if this category has a dconfig variable
        to set the notify severity and then set that level. You cannot
        set these until config is set.
        """

        # We use ConfigVariableString instead of base.config, in case
        # we're running before ShowBase has finished initializing
        from panda3d.core import ConfigVariableString

        dconfigParam = ("notify-level-" + categoryName)
        cvar = ConfigVariableString(dconfigParam, "")
        level = cvar.getValue()

        if not level:
            # see if there's an override of the default config level
            cvar2 = ConfigVariableString('default-directnotify-level', 'info')
            level = cvar2.getValue()
        if not level:
            level = 'error'

        category = self.getCategory(categoryName)
        assert category is not None, f'failed to find category: {categoryName!r}'
        # Note - this print statement is making it difficult to
        # achieve "no output unless there's an error" operation - Josh
        # print ("Setting DirectNotify category: " + categoryName +
        #        " to severity: " + level)
        if level == "error":
            category.setWarning(False)
            category.setInfo(False)
            category.setDebug(False)
        elif level == "warning":
            category.setWarning(True)
            category.setInfo(False)
            category.setDebug(False)
        elif level == "info":
            category.setWarning(True)
            category.setInfo(True)
            category.setDebug(False)
        elif level == "debug":
            category.setWarning(True)
            category.setInfo(True)
            category.setDebug(True)
        else:
            print("DirectNotify: unknown notify level: " + str(level)
                   + " for category: " + str(categoryName))

    def setDconfigLevels(self) -> None:
        for categoryName in self.getCategories():
            self.setDconfigLevel(categoryName)

    def setVerbose(self) -> None:
        for categoryName in self.getCategories():
            category = self.getCategory(categoryName)
            assert category is not None
            category.setWarning(True)
            category.setInfo(True)
            category.setDebug(True)

    def popupControls(self, tl = None):
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        NotifyPanel = importlib.import_module('direct.tkpanels.NotifyPanel')
        NotifyPanel.NotifyPanel(self, tl)

    def giveNotify(self, cls) -> None:
        cls.notify = self.newCategory(cls.__name__)
