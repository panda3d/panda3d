
"""DirectNotify module: this module contains the DirectNotify class"""

import Notifier
import Logger

class DirectNotify:
    """DirectNotify class: this class contains methods for creating
    mulitple notify categories via a dictionary of Notifiers."""
    
    def __init__(self):
        """__init__(self)
        DirectNotify class keeps a dictionary of Notfiers"""
        self.__categories = { }
        # create a default log file
        self.logger = Logger.Logger()

    def __str__(self):
        """__str__(self)
        Print handling routine"""
        return "DirectNotify categories: %s" % (self.__categories)

    #getters and setters
    def getCategories(self):
        """getCategories(self)
        Return list of category dictionary keys"""
        return (self.__categories.keys())

    def getCategory(self, categoryName):
        """getCategory(self, string)
        Return the category with given name if present, None otherwise"""
        return (self.__categories.get(categoryName, None))

    def newCategory(self, categoryName, logger=None):
        """newCategory(self, string)
        Make a new notify category named categoryName. Return new category
        if no such category exists, else return existing category"""
        if (not self.__categories.has_key(categoryName)):
            self.__categories[categoryName] = Notifier.Notifier(categoryName, logger)
            self.setDconfigLevel(categoryName)
        else:
            print "Warning: DirectNotify: category '%s' already exists" % \
                  (categoryName)
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
        if level:
            print ("Setting DirectNotify category: " + dconfigParam +
                   " to severity: " + level)
            category = self.getCategory(categoryName)
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

    def popupControls(self, tl = None):
        NotifyControlPanel(self, tl)

class NotifyControlPanel:
    """NotifyControlPanel class: this class contains methods for creating
    a panel to control direct/panda notify categories."""

    def __init__(self, directNotify, tl = None):
        """__init__(self)
        NotifyControlPanel class keeps a dictionary of Panda Notfiers"""
        self.__categories = {}
        self.activeCategory = None
        self.popupControls(directNotify, tl)

    def _getPandaCategories(self, category):
        categories = [category]
        for i in range(category.getNumChildren()):
            child = category.getChild(i)
            categories.append(self._getPandaCategories(child))
        return categories

    def getPandaCategories(self):
        from PandaModules import Notify
        topCategory = Notify.ptr().getTopCategory()
        return self._getPandaCategories(topCategory)

    def _getPandaCategoriesAsList(self, pc, list):
        import types
        for item in pc:
            if type(item) == types.ListType:
                self._getPandaCategoriesAsList(item, list)
            else:
                list.append(item)

    def getPandaCategoriesAsList(self):
        pc = self.getPandaCategories()
        pcList = []
        self._getPandaCategoriesAsList(pc, pcList)
        return pcList[1:]

    def setActivePandaCategory(self):
        categoryName = self.categoryList.getcurselection()[0]
        self.activeCategory = self.__categories.get(categoryName, None)
        if self.activeCategory:
            self.severity.set(self.activeCategory.getSeverity())

    def setActiveSeverity(self):
        if self.activeCategory:
            self.activeCategory.setSeverity(self.severity.get())

    def popupControls(self, directNotify, tl = None):
        """ popupControls()
            Popup control panel for interval.
        """
        base.wantTk = 1
        import TkGlobal
        from Tkinter import * 
        import Pmw
        from NotifySeverity import *
        if tl == None:
            tl = Toplevel()
            tl.title('Notify Controls')
            tl.geometry('300x400')
        mainFrame = Frame(tl)
        # Paned widget for dividing two halves
        framePane = Pmw.PanedWidget(mainFrame, orient = HORIZONTAL)
        categoryFrame = framePane.add('categories', size = 200)
        severityFrame = framePane.add('severities', size = 50)
        # Category frame
        from NotifyCategory import *
        # Assemble PANDA categories
        categories = self.getPandaCategoriesAsList()
        self.__categories = {}
        categoryNames = []
        for category in categories:
            name = category.getBasename()
            self.__categories[name] = category
            categoryNames.append(name)
        # Assemble DIRECT categories
        for name in directNotify.getCategories():
            category = directNotify.getCategory(name)
            self.__categories[name] = category
            categoryNames.append(name)
        # Sort resulting list of names
        categoryNames.sort()
        # Create a listbox
        self.categoryList = Pmw.ScrolledListBox(
            categoryFrame,
            labelpos = NW, label_text = 'Categories:',
            listbox_takefocus = 1,
            items = categoryNames,
            selectioncommand = self.setActivePandaCategory)
        self.categoryList.pack(expand = 1, fill = BOTH)
                                           
        # Severity frame
        Label(severityFrame, text = 'Severity:',
              justify = LEFT, anchor = W).pack()
        self.severity = IntVar()
        self.severity.set(0)
        self.fatalSeverity = Radiobutton(severityFrame, text = 'Fatal',
                                         justify = LEFT, anchor = W,
                                         value = NSFatal,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.fatalSeverity.pack(fill = X)
        self.errorSeverity = Radiobutton(severityFrame, text = 'Error',
                                         justify = LEFT, anchor = W,
                                         value = NSError,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.errorSeverity.pack(fill = X)
        self.warningSeverity = Radiobutton(severityFrame, text = 'Warning',
                                           justify = LEFT, anchor = W,
                                           value = NSWarning,
                                           variable = self.severity,
                                           command = self.setActiveSeverity)
        self.warningSeverity.pack(fill = X)
        self.infoSeverity = Radiobutton(severityFrame, text = 'Info',
                                        justify = LEFT, anchor = W,
                                        value = NSInfo,
                                        variable = self.severity,
                                        command = self.setActiveSeverity)
        self.infoSeverity.pack(fill = X)
        self.debugSeverity = Radiobutton(severityFrame, text = 'Debug',
                                         justify = LEFT, anchor = W,
                                         value = NSDebug,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.debugSeverity.pack(fill = X)
        self.spamSeverity = Radiobutton(severityFrame, text = 'Spam',
                                        justify = LEFT, anchor = W,
                                        value = NSSpam,
                                        variable = self.severity,
                                        command = self.setActiveSeverity)
        self.spamSeverity.pack(fill = X)
        # Pack frames
        framePane.pack(expand = 1, fill = BOTH)
        mainFrame.pack(expand = 1, fill = BOTH)
        # Select first item
        self.categoryList.select_set(0)
        self.setActivePandaCategory()
        # And grab focus (to allow keyboard navigation)
        self.categoryList.component('listbox').focus_set()
        # And set active index (so keypresses will start with index 0)
        self.categoryList.component('listbox').activate(0)





