"""PickList module: contains the PickList class"""
from ShowBaseGlobal import *
from GuiGlobals import *
import PandaObject
import Frame
import Button

class PickList(PandaObject.PandaObject):
    """PickList class: display a menu of choices and report users
    choice (via mouse or keyboard) as an event with the choice as
    an extra argument
    """

    # special methods
    def __init__(self, name, choiceList,
                 scale = 0.1,
                 width = None,
                 drawOrder = getDefaultDrawOrder(),
                 font = getDefaultFont()):

        self.name = name
        self.frame = Frame.Frame(name)
        
        # initialization
        self.choice = -1
        self.choiceList = []
        self.eventName = self.name
        self.frame.setOffset(0.015)
        
        # display the menu
        self.__displayChoices(choiceList, scale, width, drawOrder, font)
        self.isClean = 0
        return None

    def owns(self, item):
        for x in self.choiceList:
            if (x.button == item):
                return 1
        return None

    def cleanup(self):
        """cleanup(self)
        """
        if self.isClean == 0:
            self.isClean = 1
            self.disable()
            self.frame.unmanage()
            self.frame = None
            self.choiceList = []
        return None
        
    # accessing
    def getName(self):
        return self.name

    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

    # actions
    def __displayChoices(self, choiceList,
                         scale, width, drawOrder, font):
        """__displayChoices(self, string[])
        Display the list of choices
        """

        if width == None:
            # First, compute the maximum width of the buttons.  We do this
            # ahead of time so the Gui code doesn't have to do it and take
            # forever about it.
            width = 0
            text = TextNode()
            text.setFont(font)
            for choice in choiceList:
                w = text.calcWidth(choice) + 0.2
                width = max(width, w)

        # Now create all the buttons.
        for choice in choiceList:
            # create a button for each choice
            button = Button.Button(choice, scale = scale, width = width,
                                   drawOrder = drawOrder, font = font,
                                   event = "choose")
            choiceIndex = choiceList.index(choice)
            button.setBehaviorEventParameter(choiceIndex)
            # set the rollover-up event
            eventName = self.name + "-up"
            button.button.setUpRolloverEvent(eventName)
            # set exit event
            eventName = self.name + "-exit"
            button.button.setUpEvent(eventName)
            # keep a list of the choice buttons
            self.frame.addItem(button)
            self.choiceList.append(button)
        
        # set up the frame
        self.frame.makeVertical()

        return None

    def manage(self):
        self.enable()
        self.frame.manage()
        for x in self.choiceList:
            x.startBehavior()
        return None

    def unmanage(self):
        self.disable()
        self.frame.unmanage()
        return None

    def enable(self):
        # turn the buttons on
        self.activate()
        
        # listen for keyboard events
        self.accept("up-up", self.__decrementChoice)
        self.accept("down-up", self.__incrementChoice)
        self.accept("enter-up", self.__makeChoice, [1, None, None])
        self.accept(self.name + "-up", self.__updateButtonChoice)
        self.accept(self.name + "-exit", self.__exitChoice)
        self.accept("choose", self.__makeChoice, [0])

    def disable(self, button=None):
        # turn the buttons off
        self.deactivate(button)

        # ignore all hooks
        self.ignore("up-up")
        self.ignore("down-up")
        self.ignore("enter-up")
        self.ignore(self.name + "-up")
        self.ignore(self.name + "-exit")
        self.ignore("choose")

    def activate(self):
        # make sure items are active
        for choice in self.choiceList:
            choice.getGuiItem().up()

    def deactivate(self, button=None):
        # make sure all items are inactive, if a button
        # is passed in do NOT deactivate it.
        for choice in self.choiceList:
            if (choice.getGuiItem().isActive()):
                if not (button == None):
                    if not (self.choiceList.index(choice) == button):
                        choice.getGuiItem().inactive()
                else:
                    choice.getGuiItem().inactive()
                    
    def recompute(self):
        self.frame.recompute()

    def setPos(self, x, y):
        self.frame.setPos(x, y)

    # event handlers
    def __incrementChoice(self):
        # handle the up arrow
        choice = self.choice + 1
        if (choice > len(self.choiceList) - 1):
            choice = 0
        # enter the new choice
        self.choiceList[choice].getGuiItem().enter()
    
    def __decrementChoice(self):
        # handle the down arrow
        choice = self.choice - 1
        if (choice < 0):
            choice = len(self.choiceList) - 1
        # enter this choice
        self.choiceList[choice].getGuiItem().enter()

    def __updateButtonChoice(self, item):
        # handle the mouse rollovers
        if self.owns(item):
            if (self.choice == -1):
                self.choice = item.getBehaviorEventParameter()
            if (self.choice != item.getBehaviorEventParameter()):
                self.choice = item.getBehaviorEventParameter()
                # make sure all the other buttons have exited
                for choice in self.choiceList:
                    if (choice.button != item):
                        choice.getGuiItem().exit()
                # throw event
                messenger.send(self.name + "-rollover")
            
    def __exitChoice(self, item):
        # reset choice when mouse exits a button
        if self.owns(item):
            if (self.choice == item.getBehaviorEventParameter()):
                self.choice = -1
        
    def __makeChoice(self, button, item, whichOne):
        # handle the enter key
        if (self.choice == -1):
            # nothing selected yet
            return None
        else:
            # if the keyboard was used, update the button manually
            if (button):
                self.choiceList[self.choice].getGuiItem().down()
                def buttonUp(state):
                    state.choice.getGuiItem().up()
                    return Task.done
                task = Task.Task(buttonUp)
                task.choice = self.choiceList[self.choice]
                taskMgr.spawnTaskNamed(Task.doLater(0.035, task,
                                                    "buttonUp-later"),
                                       "doLater-buttonUp-later")
            else:
                # let everyone know a choice was made                
                if self.owns(item):
                    messenger.send(self.eventName, [self.choice])
                    self.disable(self.choice)
                
                



