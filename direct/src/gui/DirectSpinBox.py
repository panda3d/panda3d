'''This module contains the DirectSpinBox class.

A DirectSpinBox is a type of Entry specialized for number selection.
It contains two buttons to its right for increasing and decreasing the
numeric value given in the entry field.  Values can also be directly
typed in the entry and will be checked against the type given to the
spiner box which is *int* by default.

Options available for this Widget are:

*   value
    The value selected by the spiner. This will always return the number
    in the format given in the valueType option.  You can also pass a
    string or other type which the valueType function is possible to
    convert from.

*   textFormat
    This is a format string which will be used to write the value into
    the entry field

*   stepSize
    When the up and down arrows are pressed, stepSize will be used to
    determine how much the value should be in-/decreased

*   minValue
    The minimum value that can be entered in the spiner value. If values
    given to the spiner are lower than the min value, the spiner will
    set the minValue as new value.

*   maxValue
    The maximum value that can be entered in the spiner value. If values
    given to the spiner are larger than the max value, the spiner will
    set the maxValue as new value.

*   valueType
    A function like float, int or similar which will convert the value
    given to the spiner.  It's return value if it was able to convert
    the value correctly will then be converted using the textFormat to
    a string which is entered into the textfield.

*   repeatdelay
    Delay in seconds at which the value should be in-/decreased when the
    up or down arrow button is held down

*   repeatStartdelay
    This delay will be used to determine when the repeat functionality
    is actually started after the user presses down any of the buttons

*   command
    This command will be passed to the entry field and hence will be
    called, whenever the user presses enter in it.

*   extraArgs
    Extra arguments passed to the entry field for when the command is
    used

*   incButtonCallback
    A callback function which will be called when the increase button is
    first pressed.  It won't be called when the button is held down and
    the value is changed repeatedly.

*   decButtonCallback
    A callback function which will be called when the decrease button is
    first pressed.  It won't be called when the button is held down and
    the value is changed repeatedly.

'''

__all__ = ['DirectSpinBox']

from panda3d.core import *
from . import DirectGuiGlobals as DGG
from direct.directnotify import DirectNotifyGlobal
from direct.task.Task import Task
from .DirectFrame import *
from .DirectEntry import *
from .DirectButton import *

from direct.showbase.MessengerGlobal import messenger
from direct.task.TaskManagerGlobal import taskMgr

class DirectSpinBox(DirectFrame):
    notify = DirectNotifyGlobal.directNotify.newCategory('DirectSpinBox')

    def __init__(self, parent = None, **kw):
        assert self.notify.debugStateCall(self)
        self.__valueChangeCallback = None
        # Inherits from DirectFrame
        optiondefs = (
            # Define type of DirectGuiWidget
            ('value',              0,         None),
            ('textFormat',         '{:0d}',   None),
            ('stepSize',           1,         None),
            ('minValue',           0,         None),
            ('maxValue',           100,       None),
            ('valueType',          int,       None),
            ('repeatdelay',        0.125,     None),
            ('repeatStartdelay',   0.25,      None),
            ('command',            None,      None),
            ('extraArgs',          [],        None),
            ('incButtonCallback',  None,      self.setIncButtonCallback),
            ('decButtonCallback',  None,      self.setDecButtonCallback),
            ('valueChangeCallback',None,      self.setValueChangeCallback),
            ('buttonOrientation',  DGG.VERTICAL, DGG.INITOPT),
            # set default border width to 0
            ('borderWidth',        (0,0),     None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize superclasses
        DirectFrame.__init__(self, parent)

        # create the textfield that will hold the text value
        self.valueEntry = self.createcomponent('valueEntry', (), None,
                                               DirectEntry, (self,),
                                               #overflow = 1, #DOESN'T WORK WITH RIGHT TEXT ALIGN!!!
                                               width = 2,
                                               text_align = TextNode.ARight,
                                               command = self['command'],
                                               extraArgs = self['extraArgs'],
                                               focusOutCommand = self.focusOutCommand,
                                               )
        self.valueEntry.bind(DGG.WHEELUP, self.__mousewheelUp)
        self.valueEntry.bind(DGG.WHEELDOWN, self.__mousewheelDown)

        # try set the initial value
        try:
            self.setValue(self['value'])
        except:
            # Make sure the initial value is good
            self.setValue(0)

        # This font contains the up and down arrow
        shuttle_controls_font = loader.loadFont('shuttle_controls')

        # create the up arrow button
        self.incButton = self.createcomponent('incButton', (), None,
                                              DirectButton, (self,),
                                              text = '5' if self['buttonOrientation'] == DGG.VERTICAL else '4',
                                              text_font = shuttle_controls_font,
                                              )
        # Set commands for the Inc Button
        self.incButton.bind(DGG.B1PRESS, self.__incButtonDown)
        self.incButton.bind(DGG.B1RELEASE, self.__buttonUp)

        # create the down arrow button
        self.decButton = self.createcomponent('decButton', (), None,
                                              DirectButton, (self,),
                                              text = '6' if self['buttonOrientation'] == DGG.VERTICAL else '3',
                                              text_font = shuttle_controls_font
                                              )
        # Set commands for the Dec Button
        self.decButton.bind(DGG.B1PRESS, self.__decButtonDown)
        self.decButton.bind(DGG.B1RELEASE, self.__buttonUp)

        # Set the spiners elements position
        self.resetPosition()
        if self['frameSize'] is None:
            # Calculate the spiners frame size only if we don't have a
            # custom frameSize
            self.recalcFrameSize()

        # Here we check for custom values of properties.
        # We need to do this for all components which have been edited
        # by code after they have been set up by createcomponent.
        if self['incButton_pos'] is not None:
            self.incButton.setPos(self['incButton_pos'])
        if self['decButton_pos'] is not None:
            self.decButton.setPos(self['decButton_pos'])
        if self['valueEntry_pos'] is not None:
            self.valueEntry.setPos(self['valueEntry_pos'])

        self.initialiseoptions(DirectSpinBox)

    def resetPosition(self):
        '''
        Positions the two buttons to the right of the text entry
        '''
        assert self.notify.debugStateCall(self)
        # Position the text Entry field centered vertically
        valCenter = self.valueEntry.getCenter()
        self.valueEntry.setPos(0, 0, -valCenter[1])

        # Position the Inc Button
        incCenter = self.incButton.getCenter()
        incWidth = self.incButton.getWidth()
        incHeight = self.incButton.getHeight()
        incBorderW = self.incButton['borderWidth'][0]
        incBorderH = self.incButton['borderWidth'][1]
        if self['buttonOrientation'] == DGG.VERTICAL:
            self.incButton.setPos(-incCenter[0]+incWidth/2+incBorderW, 0, -incCenter[1]+incHeight/2+incBorderH)
            self.incButton.setX(self.incButton, self.valueEntry.bounds[1])
        else:
            self.incButton.setPos(-incCenter[0]+incWidth/2+incBorderW, 0, -incCenter[1])
            self.incButton.setX(self.incButton, self.valueEntry.bounds[1])

        # Position the Dec Button
        decCenter = self.decButton.getCenter()
        decWidth = self.decButton.getWidth()
        decHeight = self.decButton.getHeight()
        decBorderW = self.decButton['borderWidth'][0]
        decBorderH = self.decButton['borderWidth'][1]
        if self['buttonOrientation'] == DGG.VERTICAL:
            self.decButton.setPos(-decCenter[0]+decWidth/2+decBorderW, 0, -decCenter[1]-decHeight/2-decBorderH)
            self.decButton.setX(self.decButton, self.valueEntry.bounds[1])
        else:
            valWidth = self.valueEntry.getWidth()
            self.decButton.setPos(-valWidth-decCenter[0]-decWidth/2-decBorderW, 0, -decCenter[1])
            self.decButton.setX(self.decButton, self.valueEntry.bounds[1])


    def recalcFrameSize(self):
        '''
        Set the surrounding frame so the spinner will actually look
        like a box and will be able to give correct values for
        functions and properties like getWidth, getCenter and bounds
        '''
        assert self.notify.debugStateCall(self)
        if self['buttonOrientation'] == DGG.VERTICAL:
            l = self.valueEntry.bounds[0] - self['borderWidth'][0]
            r = self.decButton.getX() + self.decButton.bounds[1] + self.decButton['borderWidth'][0] + self['borderWidth'][0]
            b = self.valueEntry.getZ() + self.valueEntry.bounds[2] - self['borderWidth'][1]
            t = self.valueEntry.getZ() + self.valueEntry.bounds[3] + self['borderWidth'][1]
        else:
            l = self.decButton.getX() + self.decButton.bounds[0] - self.incButton['borderWidth'][0] - self['borderWidth'][0]
            r = self.incButton.getX() + self.incButton.bounds[1] + self.incButton['borderWidth'][0] + self['borderWidth'][0]
            b = self.valueEntry.getZ() + self.valueEntry.bounds[2] - self['borderWidth'][1]
            t = self.valueEntry.getZ() + self.valueEntry.bounds[3] + self['borderWidth'][1]
        self['frameSize'] = (l, r, b, t)

    def __repeatStepTask(self, task):
        assert self.notify.debugStateCall(self)
        ret = self.doStep(task.stepSize)
        task.setDelay(self['repeatdelay'])
        if ret:
            return Task.again
        else:
            return Task.done

    def __incButtonDown(self, event):
        assert self.notify.debugStateCall(self)
        task = Task(self.__repeatStepTask)
        task.stepSize = self['stepSize']
        taskName = self.taskName('repeatStep')
        #print 'incButtonDown: adding ', taskName
        taskMgr.doMethodLater(self['repeatStartdelay'], task, taskName)
        self.doStep(task.stepSize)
        messenger.send('wakeup')
        if self.__incButtonCallback:
            self.__incButtonCallback()

    def __decButtonDown(self, event):
        assert self.notify.debugStateCall(self)
        task = Task(self.__repeatStepTask)
        task.stepSize = -self['stepSize']
        taskName = self.taskName('repeatStep')
        #print 'decButtonDown: adding ', taskName
        taskMgr.doMethodLater(self['repeatStartdelay'], task, taskName)
        self.doStep(task.stepSize)
        messenger.send('wakeup')
        if self.__decButtonCallback:
            self.__decButtonCallback()

    def __buttonUp(self, event):
        assert self.notify.debugStateCall(self)
        taskName = self.taskName('repeatStep')
        #print 'buttonUp: removing ', taskName
        taskMgr.remove(taskName)

    def __mousewheelUp(self, event):
        assert self.notify.debugStateCall(self)
        self.doStep(self['stepSize'])

    def __mousewheelDown(self, event):
        assert self.notify.debugStateCall(self)
        self.doStep(-self['stepSize'])

    def doStep(self, stepSize):
        """Adds the value given in stepSize to the current value stored
        in the spinner.  Pass a negative value to subtract from the
        current value.
        """
        assert self.notify.debugStateCall(self)
        #print 'doStep[', stepSize,']'

        return self.setValue(self['value'] + stepSize)

    def __checkValue(self, newValue):
        assert self.notify.debugStateCall(self)
        try:
            value = self['valueType'](newValue)
            if value < self['minValue']:
                self.notify.info('Value out of range value: {} min allowed value: {}'.format(value, self['minValue']))
                return self['minValue']
            if value > self['maxValue']:
                self.notify.info('Value out of range value: {} max allowed value: {}'.format(value, self['maxValue']))
                return self['maxValue']
            return value
        except:
            self.notify.info('ERROR: NAN {}'.format(newValue))
            return None

    def get(self):
        '''
        Returns the value in string format (see getValue to get the value in it's specific type)
        '''
        assert self.notify.debugStateCall(self)
        return self.valueEntry.get()

    def getValue(self):
        '''
        Returns the value in it's actual type (see get to get the value as a string)
        '''
        assert self.notify.debugStateCall(self)
        return self['value']

    def setValue(self, newValue):
        '''
        Set a new value for the spinbox to display. newValue can be any type which
        can be converted by the function set in valueType
        '''
        assert self.notify.debugStateCall(self)
        value = self.__checkValue(newValue)
        if value is None:
            self.valueEntry.enterText(self['textFormat'].format(self['value']))
            return False

        self.valueEntry.enterText(self['textFormat'].format(value))
        self['value'] = value

        if self.__valueChangeCallback:
            self.__valueChangeCallback()

        return True

    def focusOutCommand(self):
        assert self.notify.debugStateCall(self)
        self.setValue(self.get())

    def setIncButtonCallback(self):
        assert self.notify.debugStateCall(self)
        self.__incButtonCallback = self['incButtonCallback']

    def setDecButtonCallback(self):
        assert self.notify.debugStateCall(self)
        self.__decButtonCallback = self['decButtonCallback']

    def setValueChangeCallback(self):
        assert self.notify.debugStateCall(self)
        self.__valueChangeCallback = self['valueChangeCallback']

'''
from direct.showbase.ShowBase import ShowBase
base = ShowBase()

spinBox = DirectSpinBox(pos=(0,0,-0.25), value=5, minValue=-100, maxValue=100, repeatdelay=0.125, buttonOrientation=DGG.HORIZONTAL, valueEntry_text_align=TextNode.ACenter, borderWidth=(1,1))
spinBox.setScale(0.1)
spinBox["relief"] = 2
spinBox = DirectSpinBox(pos=(0,0,0.25), valueEntry_width=10, borderWidth=(2,2), frameColor=(1,0,0,1))
spinBox.setScale(0.1)
base.run()
'''
