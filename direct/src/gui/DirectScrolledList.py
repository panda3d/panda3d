from DirectFrame import *
from DirectButton import *
import Task

class DirectScrolledList(DirectFrame):
    def __init__(self, parent = aspect2d, **kw):

        self.index = 0

        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('items',           [],        None),
            ('command',         None,      None),
            ('extraArgs',       [],        None),
            ('numItemsVisible', 1,         self.setNumItemsVisible),
            ('scrollSpeed',     8,         self.setScrollSpeed),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        self.incButton = self.createcomponent("incButton", (), None,
                                              DirectButton, (self,),
                                              )
        self.incButton.bind(B1PRESS, self.__incButtonDown)
        self.incButton.bind(B1RELEASE, self.__buttonUp)
        self.decButton = self.createcomponent("decButton", (), None,
                                              DirectButton, (self,),
                                              )
        self.decButton.bind(B1PRESS, self.__decButtonDown)
        self.decButton.bind(B1RELEASE, self.__buttonUp)
        self.itemFrame = self.createcomponent("itemFrame", (), None,
                                              DirectFrame, (self,),
                                              )
        for item in self["items"]:
            item.reparentTo(self.itemFrame)
            
        self.initialiseoptions(DirectScrolledList)
        self.recordMaxHeight()
        #if len(self["items"]) > 0:
        #    self.scrollTo(0)
        self.scrollTo(0)
        

    def recordMaxHeight(self):
        self.maxHeight = 0.0
        for item in self["items"]:
            self.maxHeight = max(self.maxHeight, item.getHeight())
        
    def setScrollSpeed(self):
        # Items per second to move
        self.scrollSpeed = self["scrollSpeed"]
        if self.scrollSpeed <= 0:
            self.scrollSpeed = 1

    def setNumItemsVisible(self):
        # Items per second to move
        self.numItemsVisible = self["numItemsVisible"]

    def destroy(self):
        taskMgr.remove(self.taskName("scroll"))
        DirectFrame.destroy(self)

    def scrollBy(self, delta):
        return self.scrollTo(self.index + delta)

    def scrollTo(self, index):
        self.index = index

        # Not enough items to even worry about scrolling,
        # just disable the buttons and do nothing
        if (len(self["items"]) <= self["numItemsVisible"]):
            self.incButton['state'] = DISABLED
            self.decButton['state'] = DISABLED
            # Hmm.. just reset self.index to 0 and bail out
            self.index = 0
            ret = 0
        else:
            if (self.index <= 0):
                self.index = 0
                self.decButton['state'] = DISABLED
                self.incButton['state'] = NORMAL
                ret = 0
            elif (self.index >= ( len(self["items"]) - self["numItemsVisible"])):
                self.index = len(self["items"]) - self["numItemsVisible"]
                self.incButton['state'] = DISABLED
                self.decButton['state'] = NORMAL
                ret = 0
            else:
                self.incButton['state'] = NORMAL
                self.decButton['state'] = NORMAL
                ret = 1

        # Hide them all
        for item in self["items"]:
            item.hide()
        # Then show the ones in range, and stack their positions 
        upperRange = min(len(self["items"]), self["numItemsVisible"])
        for i in range(self.index, self.index + upperRange):
            item = self["items"][i]
            item.show()
            item.setPos(0,0, - (i - self.index) * self.maxHeight)
        return ret

    def __scrollByTask(self, task):
        if ((task.time - task.prevTime) < task.delayTime):
            return Task.cont
        else:
            ret = self.scrollBy(task.delta)
            task.prevTime = task.time
            if ret:
                return Task.cont
            else:
                return Task.done
            
    def __incButtonDown(self, event):
        task = Task.Task(self.__scrollByTask)
        task.delayTime = (1.0 / self.scrollSpeed)
        task.prevTime = 0.0
        task.delta = 1
        self.scrollBy(task.delta)
        taskMgr.add(task, self.taskName("scroll"))

    def __decButtonDown(self, event):
        task = Task.Task(self.__scrollByTask)
        task.delayTime = (1.0 / self.scrollSpeed)
        task.prevTime = 0.0
        task.delta = -1
        self.scrollBy(task.delta)
        taskMgr.add(task, self.taskName("scroll"))

    def __buttonUp(self, event):
        taskMgr.remove(self.taskName("scroll"))

    def addItem(self, item):
        """
        Add this string and extraArg to the list
        """
        self['items'].append(item)
        item.reparentTo(self.itemFrame)
        self.refresh()
        

    def removeItem(self, item):
        """
        Remove this item from the panel
        """
        if item in self["items"]:
            self["items"].remove(item)
            item.reparentTo(hidden)
            self.refresh()
            return 1
        else:
            return 0
        
    def refresh(self):
        """
        Update the list - useful when adding or deleting items
        or changing properties that would effect the scrolling
        """
        self.recordMaxHeight()
        self.scrollTo(self.index)
        
