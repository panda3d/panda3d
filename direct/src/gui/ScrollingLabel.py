from ShowBaseGlobal import *
from GuiGlobals import *
import PandaObject
import Frame
import GuiFrame
import Button
import GuiLabel
import Sign
import Label



class ScrollingLabel(PandaObject.PandaObject):

    # special methods
    def __init__(self, name, itemList,
                 label = None,
                 scale = 0.1,
                 width = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont(),
                 showLabels = 1):

        self.name = name
        if (label == None):
            self.label = self.name
        else:
            self.label = label
        self.eventName = self.name
        self.frame = Frame.Frame(name)
        self.frame.setOffset(0.015)
        self.item = 0
        self.items = itemList
        self.showLabels = showLabels
        if (showLabels):
            # we'll need a card to add text to later
            itemString = " "
        else:
            # no card needed
            itemString = ""
        self.keyFocus = 1

        if width == None:
            # Compute the maximum width of the all the items.
            width = 0
            text = TextNode()
            text.setFont(font)
            for item in itemList:
                w = text.calcWidth(item) + 0.2
                width = max(width, w)

        # create the new title
        self.title = Sign.Sign(self.name, self.label, Label.ScrollTitle,
                               scale, width, drawOrder, font)
        self.frame.addItem(self.title)
                
        self.itemSign = Sign.Sign('item', itemString, Label.ScrollItem,
                                  scale, width, drawOrder, font)
        self.frame.addItem(self.itemSign)
            
        # pack the first label under the name 
        self.frame.packItem(self.itemSign, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.itemSign, GuiFrame.GuiFrame.ALIGNLEFT,
                            self.title)
        
        # create the scroll buttons
        self.leftButton = Button.Button(self.eventName + "-left",
                                        label = " < ",
                                        scale = scale,
                                        drawOrder = drawOrder,
                                        font = font, event = "left-button")
        self.leftButton.getGuiItem().setUpRolloverEvent(self.eventName + "-rollover")
        self.frame.addItem(self.leftButton)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.LEFT,
                            self.title)        
        self.rightButton = Button.Button(self.eventName + "-right",
                                         label = " > ",
                                         scale = scale,
                                         drawOrder = drawOrder,
                                         font = font, event = "right-button")
        self.rightButton.getGuiItem().setUpRolloverEvent(self.eventName + "-rollover")
        self.frame.addItem(self.rightButton)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.RIGHT,
                            self.title)        

        # set list to first element
        self.setItem(self.item)
        
        # refresh the frame
        self.frame.recompute()
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
        # ignore events
        self.ignore("left-button")
        self.ignore("right-button")
	self.ignore(self.eventName + "-rollover")
        self.setKeyFocus(0)

        # remove gui items
        self.frame = None
        self.items = None

	self.label = None
	self.title = None
	self.itemSign = None
	self.leftButton = None
	self.rightButton = None
	self.frame = None
	return None

    # accessing
    def getTitle(self):
        return self.name

    def setTitle(self, name):
        self.name = name
        if (self.showLabels):
            self.title.setText(name)
            self.frame.recompute()

    def getItemSign(self):
        return self.itemSign
    
    def getItem(self):
        return self.items[self.item]

    def setItem(self, item):
        self.item = item
        if (self.showLabels):
            self.itemSign.setText(self.items[self.item])
        
    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

    def getPos(self):
        return self.frame.getPos()
    
    def setPos(self, x, y):
        self.frame.freeze()
        self.frame.setPos(x, y)
        self.frame.recompute()
        self.frame.thaw()
        
    def setScale(self, scale):
        self.frame.freeze()
        self.frame.setScale(scale)
        self.frame.recompute()
        self.frame.thaw()

    def setWidth(self, width):
        self.frame.freeze()
        self.itemSign.setWidth(width)
        self.frame.recompute()
        self.frame.thaw()
        
    def getKeyFocus(self):
        return self.keyFocus

    def setKeyFocus(self, focus):
        self.keyFocus = focus
        if (focus == 1):
            # remove old keyboard hooks
            self.ignore("left-up")
            self.ignore("right-up")            
            # listen for new keyboard hits
            self.accept("left-up", self.handleLeftArrow)
            self.accept("right-up", self.handleRightArrow)
        else:
            # remove keyboard hooks
            self.ignore("left-up")
            self.ignore("right-up")
        
    # actions
    def recompute(self):
        self.frame.recompute()
        
    def manage(self):
        # listen for the scroll buttons
        self.accept("left-button", self.handleLeftButton)
        self.accept("right-button", self.handleRightButton)

        self.frame.manage()
        self.setKeyFocus(0)

        self.leftButton.startBehavior()
        self.rightButton.startBehavior()

	return None

    def unmanage(self):
        # ignore keyboard hits
        self.ignore("left-up")
        self.ignore("right-up")            

        # ignore events
        self.ignore("left-button")
        self.ignore("right-button")
	self.ignore(self.eventName + "-rollover")
        self.setKeyFocus(0)

        self.frame.unmanage()

	return None
        
    def handleLeftButton(self, item):
        # update the current item and the scroll label
        if (self.leftButton.button == item):
            self.item = self.item - 1
            if (self.item < 0):
                self.item = len(self.items) - 1
            self.setItem(self.item)
            messenger.send(self.eventName, [self.item])

    def handleRightButton(self, item):
        # update the current item and the scroll label
        if (self.rightButton.button == item):
            self.item = self.item + 1
            if (self.item >= len(self.items)):
                self.item = 0
            self.setItem(self.item)
            messenger.send(self.eventName, [self.item])

    def handleLeftArrow(self):
        # make the button toggle
        self.leftButton.getGuiItem().down()
        self.spawnButtonUpTask(self.leftButton)
        # then act like a mouse click
        self.handleLeftButton(self.leftButton.button)

    def handleRightArrow(self):
        # make the button toggle
        self.rightButton.getGuiItem().down()
        self.spawnButtonUpTask(self.rightButton)
        # then act like a mouse click
        self.handleRightButton(self.rightButton.button)

    def spawnButtonUpTask(self, button):
        def buttonUp(state):
            state.button.getGuiItem().up()
            return Task.done
        task = Task.Task(buttonUp)
        task.button = button
        taskMgr.spawnTaskNamed(Task.doLater(.035, task, "buttonUp-later"),
                               "doLater-buttonUp-later")
    


    





