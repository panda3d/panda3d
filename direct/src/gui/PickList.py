"""PickList module: contains the PickList class"""

from ShowBaseGlobal import *
import PandaObject
import Frame
import Button

class PickList(PandaObject.PandaObject):
    """PickList class: display a menu of choices and report users
    choice (via mouse or keyboard) as an event with the choice as
    an extra argument
    """

    # special methods
    def __init__(self, name, choiceList):

        self.name = name
        self.frame = Frame.Frame(name)
        
        # listen for keyboard events
        self.accept("up-up", self.__decrementChoice)
        self.accept("down-up", self.__incrementChoice)
        self.accept("enter-up", self.__makeChoice, [1])

        # initialization
        self.choice = -1
        self.choiceList = []
        self.eventName = self.name
        self.frame.setOffset(0.015)
        
        # display the menu
        self.__displayChoices(choiceList)
        
    def cleanup(self):
        """cleanup(self)
        Remove events and cleanup the display
        """
        # remove keyboard events
        self.ignore("up-up")
        self.ignore("down-up")
        self.ignore("enter-up")

        # ignore all the buttons
        for item in self.frame.getItems():
            self.ignore(item.getGuiItem().getUpRolloverEvent())
            self.ignore(item.getGuiItem().getUpRolloverEvent())
            self.ignore(item.getGuiItem().getDownRolloverEvent())

        # reset the display
        self.frame.unmanage()
        del(self.frame)

    # accessing
    def getName(self):
        return self.name

    def getEventName(self):
        return self.eventName

    def setEventName(self, eventName):
        self.eventName = eventName

    # actions
    def __displayChoices(self, choiceList):
        """__displayChoices(self, string[])
        Display the list of choices
        """
        for choice in choiceList:
            # create a button for each choice
            button = Button.Button(choice)
            choiceIndex = choiceList.index(choice)
            # set the rollover-up event
            eventName = self.name + "-up-" + str(choiceIndex)
            button.button.setUpRolloverEvent(eventName)
            self.accept(eventName, self.__updateButtonChoice, [choiceIndex])
            # set the rollover-down event
            eventName = self.name + "-down-" + str(choiceIndex)
            button.button.setDownRolloverEvent(eventName)
            self.accept(eventName, self.__makeChoice)
            # set exit event
            eventName = self.name + "-exit-" + str(choiceIndex)
            button.button.setUpEvent(eventName)
            self.accept(eventName, self.__exitChoice)
            # keep a list of the choice buttons
            self.frame.addItem(button)
            self.choiceList.append(button)

        # set up the frame
        self.frame.makeWideAsWidest()
        self.frame.makeVertical()
        self.frame.recompute()
        
    def manage(self):
        self.frame.manage()

    def unmanage(self):
        self.frame.unmanage()
        
    # event handlers
    def __incrementChoice(self):
        # handle the up arrow
        if (self.choice >= 0):
            # exit lest choice, if it exists
            self.choiceList[self.choice].getGuiItem().exit()
        self.choice = self.choice + 1
        if (self.choice > len(self.choiceList) - 1):
            self.choice = 0
        # enter the new choice
        self.choiceList[self.choice].getGuiItem().enter()

    
    def __decrementChoice(self):
        # handle the down arrow
        if (self.choice >= 0):
            self.choiceList[self.choice].getGuiItem().exit()        
        self.choice = self.choice - 1
        if (self.choice < 0):
            self.choice = len(self.choiceList) - 1
        # enter this choice
        self.choiceList[self.choice].getGuiItem().enter()

    def __updateButtonChoice(self, newChoice):
        # handle the mouse rollovers
        self.choice = newChoice
        # make sure all the other buttons have exited
        for choice in self.choiceList:
            if not (self.choiceList.index(choice) == self.choice):
                choice.getGuiItem().exit()
	# throw a rollover event
	messenger.send(self.name + "-rollover")

    def __exitChoice(self):
        # reset choice when mouse exits a button
        self.choice = -1
        
    def __makeChoice(self, button=0):
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
                taskMgr.spawnTaskNamed(Task.doLater(.035, task,
                                                    "buttonUp-Later"),
                                   "doLater-buttonUp-later")
            # let everyone know a choice was made                
            messenger.send(self.eventName, [self.choice])



