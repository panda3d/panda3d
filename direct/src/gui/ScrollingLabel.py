from ShowBaseGlobal import *
import PandaObject
import Frame
import GuiFrame
import Button
import GuiLabel
import Sign

font = (loader.loadModelOnce("phase_3/models/fonts/ttf-comic")).node()

class ScrollingLabel(PandaObject.PandaObject):

    # special methods
    def __init__(self, name, itemList):

        self.name = name
        self.eventName = self.name
        self.frame = Frame.Frame(name)
        self.frame.setOffset(0.015)
        self.item = 0
        self.items = []
        self.keyFocus = 1

        # create the new title
        label = GuiLabel.GuiLabel.makeSimpleTextLabel(self.name, font)
        label.setForegroundColor(1., 0., 0., 1.)
        label.setBackgroundColor(1., 1., 1., 0.)
        self.title = Sign.Sign(self.name, label)
        self.frame.addItem(self.title)
        
        # create a sign for each item in itemList
        for item in itemList:
            thisLabel = GuiLabel.GuiLabel.makeSimpleTextLabel(item, font)
            thisLabel.setForegroundColor(0., 0., 0., 1.)
            thisLabel.setBackgroundColor(1., 1., 1., 1.)
            thisSign = Sign.Sign(item, thisLabel)
            self.items.append(thisSign)
            # add each item temporarily
            self.frame.addItem(thisSign)

        # make all labels the same width
        self.frame.makeWideAsWidest()

        # remove all labels except the first
        for itemNum in range(1, len(self.items)):
            self.frame.removeItem(self.items[itemNum])

        # pack the first label under the name 
        self.frame.packItem(self.items[self.item], GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.items[self.item], GuiFrame.GuiFrame.ALIGNLEFT,
                            self.title)

        # create the scroll buttons
        self.leftButton = Button.Button(self.name + "-left", " < ")
        self.leftButton.getGuiItem().setDownRolloverEvent(self.name + "-left")
        self.leftButton.getGuiItem().setUpRolloverEvent(self.name + "-rollover")
        self.frame.addItem(self.leftButton)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.leftButton, GuiFrame.GuiFrame.LEFT,
                            self.title)        
        self.rightButton = Button.Button(self.name + "-right", " > ")
        self.rightButton.getGuiItem().setDownRolloverEvent(self.name +
                                                           "-right")    
        self.rightButton.getGuiItem().setUpRolloverEvent(self.name + "-rollover")
        self.frame.addItem(self.rightButton)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.rightButton, GuiFrame.GuiFrame.RIGHT,
                            self.title)        

        # listen for the scroll buttons
        self.accept(self.name + "-left", self.handleLeftButton)
        self.accept(self.name + "-right", self.handleRightButton)
        # listen for keyboard hits
        self.setKeyFocus(1)
        
        # refresh the frame
        self.frame.recompute()


    def cleanup(self):
        # remove gui items
        del(self.frame)
        for item in self.items:
            del(item)

        # ignore events
        self.ignore(self.name + "-left")
        self.ignore(self.name + "-right")
        self.setKeyFocus(0)
        
    # accessing
    def getTitle(self):
        return self.name

    def setTitle(self, name):
        self.name = name
        self.title.setText(name)
        self.frame.recompute()
        
    def getItem(self):
        return self.items[self.item]

    def setItem(self, item):
        self.frame.removeItem(self.items[self.item])
        self.item = item
        self.addItem()
        
    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

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
        self.frame.manage()

    def unmanage(self):
        self.frame.unmanage()
        
    def handleLeftButton(self):
        # update the current item and the scroll label
        self.frame.removeItem(self.items[self.item])
        self.item = self.item - 1
        if (self.item < 0):
            self.item = len(self.items) - 1
        self.addItem()

    def handleRightButton(self):
        # update the current item and the scroll label
        self.frame.removeItem(self.items[self.item])
        self.item = self.item + 1
        if (self.item >= len(self.items)):
            self.item = 0
        self.addItem()

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
    
    def addItem(self):
        self.frame.addItem(self.items[self.item])
        self.frame.packItem(self.items[self.item], GuiFrame.GuiFrame.UNDER,
                            self.title)
        self.frame.packItem(self.items[self.item], GuiFrame.GuiFrame.ALIGNLEFT,
                            self.title)
        self.items[self.item].manage()
        self.frame.recompute()
        messenger.send(self.eventName, [self.item])

    





