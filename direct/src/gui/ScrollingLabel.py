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
        
        # display the name of the scrolling label
        label = GuiLabel.GuiLabel.makeSimpleTextLabel(self.name, font)
        label.setForegroundColor(1., 0., 0., 1.)
        label.setBackgroundColor(1., 1., 1., 0.)
        name = Sign.Sign(self.name, label)
        self.frame.addItem(name)
        
        # create a sign for each item in itemList
        for item in itemList:
            #thisLabel = GuiLabel.GuiLabel.makeSimpleTextLabel(item, font)
            #thisLabel.setForegroundColor(0., 0., 0., 1.)
            #thisLabel.setBackgroundColor(1., 1., 1., 1.)
            #thisSign = Sign.Sign(item, thisLabel)
            thisSign = Button.Button(item)
            self.items.append(thisSign)
            # add each item temporarily
            self.frame.addItem(thisSign)

        # make all labels the same width
        self.frame.makeWideAsWidest()

        # remove all labels except the first
        for itemNum in range(1, len(self.items)):
            self.frame.removeItem(self.items[itemNum])

        # pack the first label under the name 
        self.frame.packItem(1, GuiFrame.GuiFrame.UNDER, 0)

        # create the scroll buttons
        leftButton = Button.Button(self.name + "-left", " < ")
        leftButton.getGuiItem().setDownRolloverEvent(self.name + "-left")
        self.frame.addItem(leftButton)
        self.frame.packItem(2, GuiFrame.GuiFrame.UNDER ,0)
        self.frame.packItem(2, GuiFrame.GuiFrame.LEFT ,0)        
        rightButton = Button.Button(self.name + "-right", " > ")
        rightButton.getGuiItem().setDownRolloverEvent(self.name + "-right")    
        self.frame.addItem(rightButton)
        self.frame.packItem(3, GuiFrame.GuiFrame.UNDER ,0)
        self.frame.packItem(3, GuiFrame.GuiFrame.RIGHT ,0)        

        # listen for the scroll buttons
        self.accept(self.name + "-left", self.handleLeftButton)
        self.accept(self.name + "-right", self.handleRightButton)

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

    # accessing
    def getName(self):
        return self.name

    def getItem(self):
        return self.item

    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

    def setPos(self, x, y):
        self.frame.setPos(x, y)

    def setScale(self, scale):
        self.frame.setScale(scale)


    # actions
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

    def addItem(self):
        self.frame.addItem(self.items[self.item])
        self.frame.packItem(3, GuiFrame.GuiFrame.UNDER, 0)
        self.items[self.item].manage()
        self.frame.recompute()
        messenger.send(self.eventName + str(self.item))

    





