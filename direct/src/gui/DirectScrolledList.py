"""Contains the DirectScrolledList class.

See the :ref:`directscrolledlist` page in the programming manual for a more
in-depth explanation and an example of how to use this class.
"""

__all__ = ['DirectScrolledListItem', 'DirectScrolledList']

from panda3d.core import *
from direct.showbase import ShowBaseGlobal
from . import DirectGuiGlobals as DGG
from direct.directnotify import DirectNotifyGlobal
from direct.task.Task import Task
from .DirectFrame import *
from .DirectButton import *
import sys

if sys.version_info >= (3,0):
    stringType = str
else:
    stringType = basestring


class DirectScrolledListItem(DirectButton):
    """
    While you are not required to use a DirectScrolledListItem for a
    DirectScrolledList, doing so takes care of the highlighting and
    unhighlighting of the list items.
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("DirectScrolledListItem")

    def __init__(self, parent=None, **kw):
        assert self.notify.debugStateCall(self)
        self._parent = parent
        if "command" in kw:
            self.nextCommand = kw.get("command")
            del kw["command"]
        if "extraArgs" in kw:
            self.nextCommandExtraArgs = kw.get("extraArgs")
            del kw["extraArgs"]
        optiondefs = (
            ('parent', self._parent,    None),
            ('command', self.select, None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        DirectButton.__init__(self)
        self.initialiseoptions(DirectScrolledListItem)

    def select(self):
        assert self.notify.debugStateCall(self)
        self.nextCommand(*self.nextCommandExtraArgs)
        self._parent.selectListItem(self)


class DirectScrolledList(DirectFrame):
    notify = DirectNotifyGlobal.directNotify.newCategory("DirectScrolledList")

    def __init__(self, parent = None, **kw):
        assert self.notify.debugStateCall(self)
        self.index = 0
        self.__forceHeight = None

        """ If one were to want a scrolledList that makes and adds its items
           as needed, simply pass in an items list of strings (type 'str')
           and when that item is needed, itemMakeFunction will be called
           with the text, the index, and itemMakeExtraArgs.  If itemMakeFunction
           is not specified, it will create a DirectFrame with the text."""

        # if 'items' is a list of strings, make a copy for our use
        # so we can modify it without mangling the user's list
        if 'items' in kw:
            for item in kw['items']:
                if not isinstance(item, stringType):
                    break
            else:
                # we get here if every item in 'items' is a string
                # make a copy
                kw['items'] = kw['items'][:]

        self.nextItemID = 10

        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('items',              [],        None),
            ('itemsAlign',  TextNode.ACenter, DGG.INITOPT),
            ('itemsWordwrap',      None,      DGG.INITOPT),
            ('command',            None,      None),
            ('extraArgs',          [],        None),
            ('itemMakeFunction',   None,      None),
            ('itemMakeExtraArgs',  [],        None),
            ('numItemsVisible',    1,         self.setNumItemsVisible),
            ('scrollSpeed',        8,         self.setScrollSpeed),
            ('forceHeight',        None,      self.setForceHeight),
            ('incButtonCallback',  None,      self.setIncButtonCallback),
            ('decButtonCallback',  None,      self.setDecButtonCallback),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        self.incButton = self.createcomponent("incButton", (), None,
                                              DirectButton, (self,),
                                              )
        self.incButton.bind(DGG.B1PRESS, self.__incButtonDown)
        self.incButton.bind(DGG.B1RELEASE, self.__buttonUp)
        self.decButton = self.createcomponent("decButton", (), None,
                                              DirectButton, (self,),
                                              )
        self.decButton.bind(DGG.B1PRESS, self.__decButtonDown)
        self.decButton.bind(DGG.B1RELEASE, self.__buttonUp)
        self.itemFrame = self.createcomponent("itemFrame", (), None,
                                              DirectFrame, (self,),
                                              )
        for item in self["items"]:
            if not isinstance(item, stringType):
                item.reparentTo(self.itemFrame)

        self.initialiseoptions(DirectScrolledList)
        self.recordMaxHeight()
        self.scrollTo(0)

    def setForceHeight(self):
        assert self.notify.debugStateCall(self)
        self.__forceHeight = self["forceHeight"]

    def recordMaxHeight(self):
        assert self.notify.debugStateCall(self)
        if self.__forceHeight is not None:
            self.maxHeight = self.__forceHeight
        else:
            self.maxHeight = 0.0
            for item in self["items"]:
                if not isinstance(item, stringType):
                    self.maxHeight = max(self.maxHeight, item.getHeight())

    def setScrollSpeed(self):
        assert self.notify.debugStateCall(self)
        # Items per second to move
        self.__scrollSpeed = self["scrollSpeed"]
        if self.__scrollSpeed <= 0:
            self.__scrollSpeed = 1

    def setNumItemsVisible(self):
        assert self.notify.debugStateCall(self)
        # Items per second to move
        self.__numItemsVisible = self["numItemsVisible"]

    def destroy(self):
        assert self.notify.debugStateCall(self)
        taskMgr.remove(self.taskName("scroll"))
        if hasattr(self, "currentSelected"):
            del self.currentSelected
        if self.__incButtonCallback:
            self.__incButtonCallback = None
        if self.__decButtonCallback:
            self.__decButtonCallback = None
        self.incButton.destroy()
        self.decButton.destroy()
        DirectFrame.destroy(self)

    def selectListItem(self, item):
        assert self.notify.debugStateCall(self)
        if hasattr(self, "currentSelected"):
            self.currentSelected['state']=DGG.NORMAL
        item['state']=DGG.DISABLED
        self.currentSelected=item

    def scrollBy(self, delta):
        assert self.notify.debugStateCall(self)
        #print "scrollBy[", delta,"]"
        return self.scrollTo(self.index + delta)

    def getItemIndexForItemID(self, itemID):
        assert self.notify.debugStateCall(self)
        #for i in range(len(self["items"])):
        #    print "buttontext[", i,"]", self["items"][i]["text"]

        if len(self["items"]) == 0:
            return 0

        if isinstance(self["items"][0], stringType):
            self.notify.warning("getItemIndexForItemID: cant find itemID for non-class list items!")
            return 0

        for i in range(len(self["items"])):
            if(self["items"][i].itemID == itemID):
                return i
        self.notify.warning("getItemIndexForItemID: item not found!")
        return 0

    def scrollToItemID(self, itemID, centered=0):
        assert self.notify.debugStateCall(self)
        self.scrollTo(self.getItemIndexForItemID(itemID), centered)

    def scrollTo(self, index, centered=0):
        """ scrolls list so selected index is at top, or centered in box"""
        assert self.notify.debugStateCall(self)
        #print "scrollTo[", index,"] called, len(self[items])=", len(self["items"])," self[numItemsVisible]=", self["numItemsVisible"]
        try:
            self["numItemsVisible"]
        except:
            # RAU hack to kill 27633
            self.notify.info('crash 27633 fixed!')
            return

        numItemsVisible = self["numItemsVisible"]
        numItemsTotal = len(self["items"])
        if(centered):
            self.index = index - (numItemsVisible // 2)
        else:
            self.index = index

        # Not enough items to even worry about scrolling,
        # just disable the buttons and do nothing
        if (len(self["items"]) <= numItemsVisible):
            self.incButton['state'] = DGG.DISABLED
            self.decButton['state'] = DGG.DISABLED
            # Hmm.. just reset self.index to 0 and bail out
            self.index = 0
            ret = 0
        else:
            if (self.index <= 0):
                self.index = 0
                #print "at list start, ", len(self["items"]),"  ", self["numItemsVisible"]
                self.decButton['state'] = DGG.DISABLED
                self.incButton['state'] = DGG.NORMAL
                ret = 0
            elif (self.index >= (numItemsTotal - numItemsVisible)):
                self.index = numItemsTotal - numItemsVisible
                #print "at list end, ", len(self["items"]),"  ", self["numItemsVisible"]
                self.incButton['state'] = DGG.DISABLED
                self.decButton['state'] = DGG.NORMAL
                ret = 0
            else:
                # deal with an edge condition - make sure any tasks are removed from the disabled arrows.
                if (self.incButton['state'] == DGG.DISABLED) or (self.decButton['state'] == DGG.DISABLED):
                    #print "leaving list start/end, ", len(self["items"]),"  ", self["numItemsVisible"]
                    self.__buttonUp(0)
                self.incButton['state'] = DGG.NORMAL
                self.decButton['state'] = DGG.NORMAL
                ret = 1

        #print "self.index set to ", self.index

        # Hide them all
        for item in self["items"]:
            if not isinstance(item, stringType):
                item.hide()

        # Then show the ones in range, and stack their positions
        upperRange = min(numItemsTotal, numItemsVisible)
        for i in range(self.index, self.index + upperRange):
            item = self["items"][i]
            #print "stacking buttontext[", i,"]", self["items"][i]["text"]
            # If the item is a 'str', then it has not been created (scrolled list is 'as needed')
            #  Therefore, use the the function given to make it or just make it a frame
            if isinstance(item, stringType):
                if self['itemMakeFunction']:
                    # If there is a function to create the item
                    item = self['itemMakeFunction'](item, i, self['itemMakeExtraArgs'])
                else:
                    item = DirectFrame(text = item,
                                       text_align = self['itemsAlign'],
                                       text_wordwrap = self['itemsWordwrap'],
                                       relief = None)
                #print "str stacking buttontext[", i,"]", self["items"][i]["text"]
                # Then add the newly formed item back into the normal item list
                self["items"][i] = item
                item.reparentTo(self.itemFrame)
                self.recordMaxHeight()

            item.show()
            item.setPos(0, 0,  -(i-self.index) * self.maxHeight)
            #print 'height bug tracker: i-%s idx-%s h-%s' % (i, self.index, self.maxHeight)

        if self['command']:
            # Pass any extra args to command
            self['command'](*self['extraArgs'])
        return ret

    def makeAllItems(self):
        assert self.notify.debugStateCall(self)
        for i in range(len(self['items'])):
            item = self["items"][i]
            # If the item is a 'str', then it has not been created
            # Therefore, use the the function given to make it or
            # just make it a frame
            #print "Making " + str(item)
            if isinstance(item, stringType):
                if self['itemMakeFunction']:
                    # If there is a function to create the item
                    item = self['itemMakeFunction'](item, i, self['itemMakeExtraArgs'])
                else:
                    item = DirectFrame(text = item,
                                       text_align = self['itemsAlign'],
                                       text_wordwrap = self['itemsWordwrap'],
                                       relief = None)
                # Then add the newly formed item back into the normal item list
                self["items"][i] = item
                item.reparentTo(self.itemFrame)
        self.recordMaxHeight()

    def __scrollByTask(self, task):
        assert self.notify.debugStateCall(self)
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
        assert self.notify.debugStateCall(self)
        task = Task(self.__scrollByTask)
        task.setDelay(1.0 / self.__scrollSpeed)
        task.prevTime = 0.0
        task.delta = 1
        taskName = self.taskName("scroll")
        #print "incButtonDown: adding ", taskName
        taskMgr.add(task, taskName)
        self.scrollBy(task.delta)
        messenger.send('wakeup')
        if self.__incButtonCallback:
            self.__incButtonCallback()

    def __decButtonDown(self, event):
        assert self.notify.debugStateCall(self)
        task = Task(self.__scrollByTask)
        task.setDelay(1.0 / self.__scrollSpeed)
        task.prevTime = 0.0
        task.delta = -1
        taskName = self.taskName("scroll")
        #print "decButtonDown: adding ", taskName
        taskMgr.add(task, taskName)
        self.scrollBy(task.delta)
        messenger.send('wakeup')
        if self.__decButtonCallback:
            self.__decButtonCallback()

    def __buttonUp(self, event):
        assert self.notify.debugStateCall(self)
        taskName = self.taskName("scroll")
        #print "buttonUp: removing ", taskName
        taskMgr.remove(taskName)

    def addItem(self, item, refresh=1):
        """
        Add this string and extraArg to the list
        """
        assert self.notify.debugStateCall(self)
        if not isinstance(item, stringType):
            # cant add attribs to non-classes (like strings & ints)
            item.itemID = self.nextItemID
            self.nextItemID += 1
        self['items'].append(item)
        if not isinstance(item, stringType):
            item.reparentTo(self.itemFrame)
        if refresh:
            self.refresh()
        if not isinstance(item, stringType):
            return item.itemID  # to pass to scrollToItemID

    def removeItem(self, item, refresh=1):
        """
        Remove this item from the panel
        """
        assert self.notify.debugStateCall(self)
        #print "remove item called", item
        #print "items list", self['items']
        if item in self["items"]:
            #print "removing item", item
            if hasattr(self, "currentSelected") and self.currentSelected is item:
                del self.currentSelected
            self["items"].remove(item)
            if not isinstance(item, stringType):
                item.reparentTo(ShowBaseGlobal.hidden)
            self.refresh()
            return 1
        else:
            return 0

    def removeAndDestroyItem(self, item, refresh = 1):
        """
        Remove and destroy this item from the panel.
        """
        assert self.notify.debugStateCall(self)
        if item in self["items"]:
            if hasattr(self, "currentSelected") and self.currentSelected is item:
                del self.currentSelected
            if (hasattr(item, 'destroy') and hasattr(item.destroy, '__call__')):
                item.destroy()
            self["items"].remove(item)
            if not isinstance(item, stringType):
                item.reparentTo(ShowBaseGlobal.hidden)
            self.refresh()
            return 1
        else:
            return 0

    def removeAllItems(self, refresh=1):
        """
        Remove this item from the panel
        Warning 2006_10_19 tested only in the trolley metagame
        """
        assert self.notify.debugStateCall(self)
        retval = 0
        #print "remove item called", item
        #print "items list", self['items']
        while len (self["items"]):
            item = self['items'][0]
            #print "removing item", item
            if hasattr(self, "currentSelected") and self.currentSelected is item:
                del self.currentSelected
            self["items"].remove(item)
            if not isinstance(item, stringType):
                #RAU possible leak here, let's try to do the right thing
                #item.reparentTo(ShowBaseGlobal.hidden)
                item.removeNode()
            retval = 1

        if (refresh):
            self.refresh()

        return retval

    def removeAndDestroyAllItems(self, refresh = 1):
        """
        Remove and destroy all items from the panel.
        Warning 2006_10_19 tested only in the trolley metagame
        """
        assert self.notify.debugStateCall(self)
        retval = 0
        while len (self["items"]):
            item = self['items'][0]
            if hasattr(self, "currentSelected") and self.currentSelected is item:
                del self.currentSelected
            if (hasattr(item, 'destroy') and hasattr(item.destroy, '__call__')):
                item.destroy()
            self["items"].remove(item)
            if not isinstance(item, stringType):
                #RAU possible leak here, let's try to do the right thing
                #item.reparentTo(ShowBaseGlobal.hidden)
                item.removeNode()
            retval = 1
        if (refresh):
            self.refresh()
        return retval

    def refresh(self):
        """
        Update the list - useful when adding or deleting items
        or changing properties that would affect the scrolling
        """
        assert self.notify.debugStateCall(self)
        self.recordMaxHeight()
        #print "refresh called"
        self.scrollTo(self.index)

    def getSelectedIndex(self):
        assert self.notify.debugStateCall(self)
        return self.index

    def getSelectedText(self):
        assert self.notify.debugStateCall(self)
        if isinstance(self['items'][self.index], stringType):
          return self['items'][self.index]
        else:
          return self['items'][self.index]['text']

    def setIncButtonCallback(self):
        assert self.notify.debugStateCall(self)
        self.__incButtonCallback = self["incButtonCallback"]

    def setDecButtonCallback(self):
        assert self.notify.debugStateCall(self)
        self.__decButtonCallback = self["decButtonCallback"]


"""
from DirectGui import *

def makeButton(itemName, itemNum, *extraArgs):
    def buttonCommand():
        print itemName, itemNum
    return DirectButton(text = itemName,
                        relief = DGG.RAISED,
                        frameSize = (-3.5, 3.5, -0.2, 0.8),
                        scale = 0.85,
                        command = buttonCommand)

s = scrollList = DirectScrolledList(
    parent = aspect2d,
    relief = None,
    # Use the default dialog box image as the background
    image = DGG.getDefaultDialogGeom(),
    # Scale it to fit around everyting
    image_scale = (0.7, 1, .8),
    # Give it a label
    text = "Scrolled List Example",
    text_scale = 0.06,
    text_align = TextNode.ACenter,
    text_pos = (0, 0.3),
    text_fg = (0, 0, 0, 1),
    # inc and dec are DirectButtons
    # They can contain a combination of text, geometry and images
    # Just a simple text one for now
    incButton_text = 'Increment',
    incButton_relief = DGG.RAISED,
    incButton_pos = (0.0, 0.0, -0.36),
    incButton_scale = 0.1,
    # Same for the decrement button
    decButton_text = 'Decrement',
    decButton_relief = DGG.RAISED,
    decButton_pos = (0.0, 0.0, 0.175),
    decButton_scale = 0.1,
    # each item is a button with text on it
    numItemsVisible = 4,
    itemMakeFunction = makeButton,
    items = ['Able', 'Baker', 'Charlie', 'Delta', 'Echo', 'Foxtrot',
             'Golf', 'Hotel', 'India', 'Juliet', 'Kilo', 'Lima'],
    # itemFrame is a DirectFrame
    # Use it to scale up or down the items and to place it relative
    # to eveything else
    itemFrame_pos = (0, 0, 0.06),
    itemFrame_scale = 0.1,
    itemFrame_frameSize = (-3.1, 3.1, -3.3, 0.8),
    itemFrame_relief = DGG.GROOVE,
    )
"""
