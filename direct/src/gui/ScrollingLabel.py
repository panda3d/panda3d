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
                 font = getDefaultFont()):

        self.name = name
        self.eventName = self.name
        self.frame = Frame.Frame(name)
        self.frame.setOffset(0.015)
        self.item = 0
        self.items = itemList
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
        self.title = Sign.Sign(self.name, self.name, Label.ScrollTitle,
                               scale, width, drawOrder, font)
        self.frame.addItem(self.title)
                
        self.itemSign = Sign.Sign('item', '', Label.ScrollItem,
                                  scale, width, drawOrder, font)
        self.frame.addItem(self.itemSign)
            
        # pack the first label under the name 
        self.frame.packItem(self.itemSign, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.itemSign, GuiFrame.GuiFrame.ALIGNLEFT,
                            self.title)
        
        # create the scroll buttons
        self.leftButton = Button.Button(self.eventName + "-left", " < ",
                                        scale, None, drawOrder, font)
        self.leftButton.getGuiItem().setDownRolloverEvent(self.eventName + "-left")
        self.leftButton.getGuiItem().setUpRolloverEvent(self.eventName + "-rollover")
        self.frame.addItem(self.leftButton)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.LEFT,
                            self.title)        
        self.rightButton = Button.Button(self.eventName + "-right", " > ",
                                         scale, None, drawOrder, font)
        self.rightButton.getGuiItem().setDownRolloverEvent(self.eventName +
                                                           "-right")    
        self.rightButton.getGuiItem().setUpRolloverEvent(self.eventName + "-rollover")
        self.frame.addItem(self.rightButton)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.RIGHT,
                            self.title)        

        # listen for the scroll buttons
        #self.accept(self.eventName + "-left", self.handleLeftButton)
        #self.accept(self.eventName + "-right", self.handleRightButton)
        
        # listen for keyboard hits
        #self.setKeyFocus(0)

        # set list to first element
        self.setItem(self.item)
        
        # refresh the frame
        self.frame.recompute()
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
        # ignore events
        self.ignore(self.eventName + "-left")
        self.ignore(self.eventName + "-right")
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
        self.title.setText(name)
        self.frame.recompute()

    def getItemSign(self):
        return self.itemSign
    
    def getItem(self):
        return self.items[self.item]

    def setItem(self, item):
        self.item = item
        self.itemSign.setText(self.items[self.item])
        
    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

    def getPos(self):
        return self.frame.getPos()
    
    def setPos(self, x, y):
        self.frame.setPos(x, y)
        self.frame.recompute()

    def setScale(self, scale):
        self.frame.setScale(scale)
        self.frame.recompute()

    def getKeyFocus(self):
        return self.keyFocus

    def setKeyFocus(self, focus):
        self.keyFocus = focus
        if (focus == 1):
            # ignore keyboard hits
            self.ignore("left-up")
            self.ignore("right-up")            
            # listen for keyboard hits
            self.accept("left-up", self.handleLeftArrow)
            self.accept("right-up", self.handleRightArrow)
        else:
            # ignore keyboard hits
            self.ignore("left-up")
            self.ignore("right-up")
        
    # actions
    def recompute(self):
        self.frame.recompute()
        
    def manage(self):
        # listen for the scroll buttons
        self.accept(self.eventName + "-left", self.handleLeftButton)
        self.accept(self.eventName + "-right", self.handleRightButton)

        self.frame.manage()
        self.setKeyFocus(0)

	return None

    def unmanage(self):
        # ignore keyboard hits
        self.ignore("left-up")
        self.ignore("right-up")            

        # ignore events
        self.ignore(self.eventName + "-left")
        self.ignore(self.eventName + "-right")
	self.ignore(self.eventName + "-rollover")
        self.setKeyFocus(0)

        self.frame.unmanage()

	return None
        
    def handleLeftButton(self):
        # update the current item and the scroll label
        self.item = self.item - 1
        if (self.item < 0):
            self.item = len(self.items) - 1
        self.setItem(self.item)
        messenger.send(self.eventName, [self.item])

    def handleRightButton(self):
        # update the current item and the scroll label
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
        self.handleLeftButton()

    def handleRightArrow(self):
        # make the button toggle
        self.rightButton.getGuiItem().down()
        self.spawnButtonUpTask(self.rightButton)
        # then act like a mouse click
        self.handleRightButton()

    def spawnButtonUpTask(self, button):
        def buttonUp(state):
            state.button.getGuiItem().up()
            return Task.done
        task = Task.Task(buttonUp)
        task.button = button
        taskMgr.spawnTaskNamed(Task.doLater(.035, task, "buttonUp-later"),
                               "doLater-buttonUp-later")
    


    





