"""Undocumented Module"""

__all__ = ['NotifyPanel']


class NotifyPanel:
    """NotifyPanel class: this class contains methods for creating
    a panel to control direct/panda notify categories."""

    def __init__(self, directNotify, tl = None):
        """
        NotifyPanel class pops up a control panel to view/set
        notify levels for all available DIRECT and PANDA notify categories
        """
        # Make sure TK mainloop is running
        from direct.showbase.TkGlobal import Pmw, Toplevel, Frame, Label, Radiobutton
        from direct.showbase.TkGlobal import HORIZONTAL, X, W, NW, BOTH, LEFT, RIGHT, IntVar
        # To get severity levels
        from pandac.PandaModules import NSFatal, NSError, NSWarning, NSInfo
        from pandac.PandaModules import NSDebug, NSSpam

        if tl == None:
            tl = Toplevel()
            tl.title('Notify Controls')
            tl.geometry('300x400')
        # Init active category
        self.activeCategory = None
        # Create widgets
        mainFrame = Frame(tl)
        # Paned widget for dividing two halves
        framePane = Pmw.PanedWidget(mainFrame,
                                    orient = HORIZONTAL)
        categoryFrame = framePane.add('categories', size = 200)
        severityFrame = framePane.add('severities', size = 50)
        # Category frame
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
            labelpos = 'nw', label_text = 'Categories:',
            label_font=('MSSansSerif', 10, 'bold'),
            listbox_takefocus = 1,
            items = categoryNames,
            selectioncommand = self.setActivePandaCategory)
        self.categoryList.pack(expand = 1, fill = 'both')
                                           
        # Severity frame
        Label(severityFrame, text = 'Severity:',
              font=('MSSansSerif', 10, 'bold'),
              justify = RIGHT, anchor = W).pack(fill = X, padx = 5)
        self.severity = IntVar()
        self.severity.set(0)
        self.fatalSeverity = Radiobutton(severityFrame, text = 'Fatal',
                                         justify = 'left', anchor = 'w',
                                         value = NSFatal,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.fatalSeverity.pack(fill = X)
        self.errorSeverity = Radiobutton(severityFrame, text = 'Error',
                                         justify = 'left', anchor = 'w',
                                         value = NSError,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.errorSeverity.pack(fill = X)
        self.warningSeverity = Radiobutton(severityFrame, text = 'Warning',
                                           justify = 'left', anchor = 'w',
                                           value = NSWarning,
                                           variable = self.severity,
                                           command = self.setActiveSeverity)
        self.warningSeverity.pack(fill = X)
        self.infoSeverity = Radiobutton(severityFrame, text = 'Info',
                                        justify = 'left', anchor = 'w',
                                        value = NSInfo,
                                        variable = self.severity,
                                        command = self.setActiveSeverity)
        self.infoSeverity.pack(fill = X)
        self.debugSeverity = Radiobutton(severityFrame, text = 'Debug',
                                         justify = 'left', anchor = 'w',
                                         value = NSDebug,
                                         variable = self.severity,
                                         command = self.setActiveSeverity)
        self.debugSeverity.pack(fill = X)
        self.spamSeverity = Radiobutton(severityFrame, text = 'Spam',
                                        justify = 'left', anchor = 'w',
                                        value = NSSpam,
                                        variable = self.severity,
                                        command = self.setActiveSeverity)
        self.spamSeverity.pack(fill = X)
        # Pack frames
        framePane.pack(expand = 1, fill = 'both')
        mainFrame.pack(expand = 1, fill = 'both')
        # Get listbox
        listbox = self.categoryList.component('listbox')
        # Bind updates to arrow buttons
        listbox.bind('<KeyRelease-Up>', self.setActivePandaCategory)
        listbox.bind('<KeyRelease-Down>', self.setActivePandaCategory)
        # And grab focus (to allow keyboard navigation)
        listbox.focus_set()
        # And set active index (so keypresses will start with index 0)
        listbox.activate(0)
        # Select first item
        self.categoryList.select_set(0)
        self.setActivePandaCategory()


    def _getPandaCategories(self, category):
        categories = [category]
        for i in range(category.getNumChildren()):
            child = category.getChild(i)
            categories.append(self._getPandaCategories(child))
        return categories

    def getPandaCategories(self):
        from pandac.PandaModules import Notify
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

    def setActivePandaCategory(self, event = None):
        categoryName = self.categoryList.getcurselection()[0]
        self.activeCategory = self.__categories.get(categoryName, None)
        if self.activeCategory:
            self.severity.set(self.activeCategory.getSeverity())

    def setActiveSeverity(self):
        if self.activeCategory:
            self.activeCategory.setSeverity(self.severity.get())

