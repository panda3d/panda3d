"""
DirectNotify module: this module contains the DirectNotify class
"""

import Notifier
import Logger

class DirectNotify:
    """
    DirectNotify class: this class contains methods for creating
    mulitple notify categories via a dictionary of Notifiers.
    """

    def __init__(self):
        """
        DirectNotify class keeps a dictionary of Notfiers
        """
        self.__categories = { }
        # create a default log file
        self.logger = Logger.Logger()

        # This will get filled in later by ShowBase.py with a
        # C++-level StreamWriter object for writing to standard
        # output.
        self.streamWriter = None

    def __str__(self):
        """
        Print handling routine
        """
        return "DirectNotify categories: %s" % (self.__categories)

    #getters and setters
    def getCategories(self):
        """
        Return list of category dictionary keys
        """
        return (self.__categories.keys())

    def getCategory(self, categoryName):
        """getCategory(self, string)
        Return the category with given name if present, None otherwise
        """
        return (self.__categories.get(categoryName, None))

    def newCategory(self, categoryName, logger=None):
        """newCategory(self, string)
        Make a new notify category named categoryName. Return new category
        if no such category exists, else return existing category
        """
        if (not self.__categories.has_key(categoryName)):
            self.__categories[categoryName] = Notifier.Notifier(categoryName, logger)
            self.setDconfigLevel(categoryName)
        return (self.getCategory(categoryName))

    def setDconfigLevel(self, categoryName):
        """
        Check to see if this category has a dconfig variable
        to set the notify severity and then set that level. You cannot
        set these until config is set.
        """
        # We cannot check dconfig variables until config has been
        # set. Once config is set in ShowBase.py, it tries to set
        # all the levels again in case some were created before config
        # was created.
        try:
            config
        except:
            return 0

        dconfigParam = ("notify-level-" + categoryName)
        level = config.GetString(dconfigParam, "")

        if not level:
            # see if there's an override of the default config level
            level = config.GetString('default-directnotify-level', 'info')
        if not level:
            level = 'error'

        category = self.getCategory(categoryName)
        # Note - this print statement is making it difficult to
        # achieve "no output unless there's an error" operation - Josh
        # print ("Setting DirectNotify category: " + categoryName +
        #        " to severity: " + level)
        if level == "error":
            category.setWarning(0)
            category.setInfo(0)
            category.setDebug(0)
        elif level == "warning":
            category.setWarning(1)
            category.setInfo(0)
            category.setDebug(0)
        elif level == "info":
            category.setWarning(1)
            category.setInfo(1)
            category.setDebug(0)
        elif level == "debug":
            category.setWarning(1)
            category.setInfo(1)
            category.setDebug(1)
        else:
            print ("DirectNotify: unknown notify level: " + str(level)
                   + " for category: " + str(categoryName))
            

    def setDconfigLevels(self):
        for categoryName in self.getCategories():
            self.setDconfigLevel(categoryName)

    def setVerbose(self):
        for categoryName in self.getCategories():
            category = self.getCategory(categoryName)
            category.setWarning(1)
            category.setInfo(1)
            category.setDebug(1)
            
    def popupControls(self, tl = None):
        from direct.tkpanels import NotifyPanel
        NotifyPanel.NotifyPanel(self, tl)
        
    def giveNotify(self,cls):
        cls.notify = self.newCategory(cls.__name__)
