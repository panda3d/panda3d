"DIRECT Animation Control Panel"

# Import Tkinter, Pmw, and the floater code from this directory tree.
from AppShell import *
from Tkinter import *
from tkSimpleDialog import askfloat
import Pmw
import string
import math
import types

FRAMES = 0
SECONDS = 1

class AnimPanel(AppShell):
    # Override class variables
    appname = 'Anim Panel'
    frameWidth  = 675
    frameHeight = 250
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, aList =  [], parent = None, **kw):
        INITOPT = Pmw.INITOPT
        if ((type(aList) == types.ListType) or
            (type(aList) == types.TupleType)):
            kw['actorList'] = aList
        else:
            kw['actorList'] = [aList]
        optiondefs = (
            ('title',               self.appname,       None),
            ('actorList',           [],                 None),
            ('Actor_label_width',   12,                 None),
            )
        self.defineoptions(kw, optiondefs)

        self.frameHeight = 60 + (50 * len(self['actorList']))
        # Initialize the superclass
        AppShell.__init__(self)

        # Execute option callbacks
        self.initialiseoptions(AnimPanel)

    def createInterface(self):
        # Handle to the toplevels interior
        interior = self.interior()
        menuBar = self.menuBar

        menuBar.addmenu('AnimPanel', 'Anim Panel Operations')
        # Actor control status
        menuBar.addcascademenu('AnimPanel', 'Control Status',
                               'Enable/disable actor control panels')
        menuBar.addmenuitem('Control Status', 'command',
                            'Enable all actor controls',
                            label = 'Enable all',
                            command = self.enableActorControls)
        menuBar.addmenuitem('Control Status', 'command',
                            'Disable all actor controls',
                            label = 'Disable all',
                            command = self.disableActorControls)
        # Frame Slider units
        menuBar.addcascademenu('AnimPanel', 'Display Units',
                               'Select display units')
        menuBar.addmenuitem('Display Units', 'command',
                            'Display frame counts', label = 'Frame count',
                            command = self.displayFrameCounts)
        menuBar.addmenuitem('Display Units', 'command',
                            'Display seconds', label = 'Seconds',
                            command = self.displaySeconds)
        # Reset all actor controls
        menuBar.addmenuitem('AnimPanel', 'command',
                            'Reset Actor controls to zero',
                            label = 'Reset all to zero',
                            command = self.resetAllToZero)
        menuBar.addmenuitem('AnimPanel', 'command',
                            'Reset Actor controls to end time',
                            label = 'Reset all to end time',
                            command = self.resetAllToEnd)

        # Add some buttons to update all Actor Controls
        self.fToggleAll = 1
        b = self.createcomponent(
            'toggleEnableButton', (), None,
            Button, (self.menuFrame,),
            text = 'Toggle Enable',
            command = self.toggleAllControls)
        b.pack(side = RIGHT, expand = 0)
            
        b = self.createcomponent(
            'showSecondsButton', (), None,
            Button, (self.menuFrame,),
            text = 'Show Seconds',
            command = self.displaySeconds)
        b.pack(side = RIGHT, expand = 0)

        b = self.createcomponent(
            'showFramesButton', (), None,
            Button, (self.menuFrame,),
            text = 'Show Frames',
            command = self.displayFrameCounts)
        b.pack(side = RIGHT, expand = 0)

        # Create a frame to hold all the actor controls
        actorFrame = Frame(interior)

        # Create a control for each actor
        index = 0
        self.actorControlList = []
        for actor in self['actorList']:
            ac = self.createcomponent(
                'actorControl%d' % index, (), 'Actor',
                ActorControl, (actorFrame,),
                text = actor.getName(),
                animList = actor.getAnimNames(),
                actor = actor)
            ac.pack(expand = 1, fill = X)
            self.actorControlList.append(ac)
            index = index + 1

        # Now pack the actor frame
        actorFrame.pack(expand = 1, fill = BOTH)

        # Create a frame to hold the playback controls
        controlFrame = Frame(interior)
        self.toStartButton = self.createcomponent(
            'toStart', (), None,
            Button, (controlFrame,),
            text = '<<',
            width = 8,
            command = self.resetAllToZero)
        self.toStartButton.pack(side = LEFT, expand = 1, fill = X)
        
        self.playButton = self.createcomponent(
            'playPause', (), None,
            Button, (controlFrame,),
            text = 'Play', width = 8,
            command = self.playActorControls)
        self.playButton.pack(side = LEFT, expand = 1, fill = X)

        self.stopButton = self.createcomponent(
            'playPause', (), None,
            Button, (controlFrame,),
            text = 'Stop', width = 8,
            command = self.stopActorControls)
        self.stopButton.pack(side = LEFT, expand = 1, fill = X)

        self.toEndButton = self.createcomponent(
            'toEnd', (), None,
            Button, (controlFrame,),
            text = '>>',
            width = 8,
            command = self.resetAllToEnd)
        self.toEndButton.pack(side = LEFT, expand = 1, fill = X)
        
        self.loopVar = IntVar()
        self.loopVar.set(0)
        self.loopButton = self.createcomponent(
            'loopButton', (), None,
            Checkbutton, (controlFrame,),
            text = 'Loop', width = 8,
            variable = self.loopVar)
        self.loopButton.pack(side = LEFT, expand = 1, fill = X)

        controlFrame.pack(fill = X)

    def playActorControls(self):
        fLoop = self.loopVar.get()
        for actorControl in self.actorControlList:
            actorControl.play(fLoop)

    def stopActorControls(self):
        for actorControl in self.actorControlList:
            actorControl.stop()

    def getActorControlAt(self, index):
        return self.actorControlList[index]

    def enableActorControlAt(self,index):
        self.getActorControlAt(index).enableControl()

    def toggleAllControls(self):
        if self.fToggleAll:
            self.disableActorControls()
        else:
            self.enableActorControls()
        self.fToggleAll = 1 - self.fToggleAll
        
    def enableActorControls(self):
        for actorControl in self.actorControlList:
            actorControl.enableControl()

    def disableActorControls(self):
        for actorControl in self.actorControlList:
            actorControl.disableControl()

    def disableActorControlAt(self,index):
        self.getActorControlAt(index).disableControl()

    def displayFrameCounts(self):
        for actorControl in self.actorControlList:
            actorControl.displayFrameCounts()

    def displaySeconds(self):
        for actorControl in self.actorControlList:
            actorControl.displaySeconds()

    def resetAllToZero(self):
        for actorControl in self.actorControlList:
            actorControl.resetToZero()

    def resetAllToEnd(self):
        for actorControl in self.actorControlList:
            actorControl.resetToEnd()

class ActorControl(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        DEFAULT_FONT = (('MS', 'Sans', 'Serif'), 12, 'bold')
        DEFAULT_ANIMS = ('neutral', 'run', 'walk')
        animList = kw.get('animList', DEFAULT_ANIMS)
        if len(animList) > 0:
            initActive = animList[0]
        else:
            initActive = DEFAULT_ANIMS[0]
        optiondefs = (
            ('text',            'Actor',            self._updateLabelText),
            ('actor',           None,               None),
            ('animList',        DEFAULT_ANIMS,      None),
            ('active',          initActive,         None),
            ('sLabel_width',    5,                  None),
            ('sLabel_font',     DEFAULT_FONT,       None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Handle to the toplevels hull
        interior = self.interior()
        interior.configure(relief = RAISED, bd = 2)

        # Instance variables
        self.offset = 0.0
        self.maxFrame = 1.0

        # Create component widgets
        self._label = self.createcomponent(
            'label', (), None,
            Menubutton, (interior,),            
            font=('MSSansSerif', 14, 'bold'),
            relief = RAISED, bd = 1,
            activebackground = '#909090',
            text = self['text'])
        # Top level menu
        labelMenu = Menu(self._label, tearoff = 0 )
        
        # Menu to select display mode
        self.unitsVar = IntVar()
        self.unitsVar.set(FRAMES)
        displayMenu = Menu(labelMenu, tearoff = 0 )        
        displayMenu.add_radiobutton(label = 'Frame count',
                                    value = FRAMES,
                                    variable = self.unitsVar,
                                    command = self.updateDisplay)
        displayMenu.add_radiobutton(label = 'Seconds',
                                    value = SECONDS,
                                    variable = self.unitsVar,
                                    command = self.updateDisplay)
        # Items for top level menu
        labelMenu.add_cascade(label = 'Display Units', menu = displayMenu)
        labelMenu.add_command(label = 'Set Offset', command = self.setOffset)
        labelMenu.add_command(label = 'Reset', command = self.resetToZero)
        # Now associate menu with menubutton
        self._label['menu'] = labelMenu
        self._label.pack(side = LEFT, fill = X)

        # Combo box to select current animation
        animMenu = self.createcomponent(
            'animMenu', (), None,
            Pmw.ComboBox, (interior,),
            labelpos = W, label_text = 'Anim:',
            entry_width = 12, selectioncommand = self.selectAnimNamed,
            scrolledlist_items = self['animList'])
        animMenu.selectitem(self['active'])
        animMenu.pack(side = 'left', padx = 5, expand = 0)

        # Combo box to select frame rate
        playRateList = [1/24.0, 0.1, 0.5, 1.0, 2.0,5.0,10.0]
        playRate = self['actor'].getPlayRate(self['active'])
        if playRate not in playRateList:
            playRateList.append(playRate)
            playRateList.sort
        playRateMenu = self.createcomponent(
            'playRateMenu', (), None,
            Pmw.ComboBox, (interior,),
            labelpos = W, label_text = 'X',
            entry_width = 4, selectioncommand = self.setPlayRate,
            scrolledlist_items = playRateList)
        playRateMenu.selectitem(`playRate`)
        playRateMenu.pack(side = LEFT, padx = 5, expand = 0)

        # A label
        playRateLabel = Label(interior, text = 'speed')
        playRateLabel.pack(side = LEFT)

        # Scale to control animation
        frameFrame = Frame(interior, relief = SUNKEN, bd = 1)
        self.minLabel = self.createcomponent(
            'minLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 0)
        self.minLabel.pack(side = LEFT)

        self.frameControl = self.createcomponent(
            'scale', (), None,
            Scale, (frameFrame,),
            from_ = 0.0, to = self.maxFrame, resolution = 1.0,
            command = self.goTo,
            orient = HORIZONTAL, showvalue = 1)
        self.frameControl.pack(side = LEFT, expand = 1)

        self.maxLabel = self.createcomponent(
            'maxLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = self.maxFrame)
        self.maxLabel.pack(side = LEFT)
        frameFrame.pack(side = LEFT, expand = 1, fill = X)

        # Checkbutton to enable/disable control
        self.frameActiveVar = IntVar()
        self.frameActiveVar.set(1)
        frameActive = self.createcomponent(
            'checkbutton', (), None,
            Checkbutton, (interior,),
            variable = self.frameActiveVar)
        frameActive.pack(side = LEFT, expand = 1)

        # Execute option callbacks
        self.initialiseoptions(ActorControl)
        self.updateDisplay()

    def _updateLabelText(self):
        self._label['text'] = self['text']

    def updateDisplay(self):
        actor = self['actor']
        active = self['active']
        self.fps = actor.getFrameRate(active)
        self.maxFrame = actor.getNumFrames(active) - 1
        self.maxSeconds = self.maxFrame / self.fps
        # Switch between showing frame counts and seconds
        if self.unitsVar.get() == FRAMES:
            newMin = int(math.floor(self.offset * self.fps))
            newMax = int(math.ceil(self.offset * self.fps)) + self.maxFrame
            self.minLabel['text'] = newMin
            self.maxLabel['text'] = newMax
            self.frameControl.configure(to = newMax, resolution = 1.0)
        else:
            newMin = self.offset
            newMax = self.offset + self.maxSeconds
            self.minLabel['text'] = "%.1f" % newMin
            self.maxLabel['text'] = "%.1f" % newMax
            self.frameControl.configure(to = newMax, resolution = 0.01)

    def selectAnimNamed(self, name):
        self['active'] = name
        self.updateDisplay()

    def setPlayRate(self, rate):
        self['actor'].setPlayRate(eval(rate), self['active'])
        self.updateDisplay()

    def setOffset(self):
        newOffset = askfloat(parent = self.interior(),
                             title = self['text'],
                             prompt = 'Start offset (seconds):')
        if newOffset != None:
            self.offset = newOffset
            self.updateDisplay()

    def enableControl(self):
        self.frameActiveVar.set(1)

    def disableControl(self):
        self.frameActiveVar.set(0)

    def displayFrameCounts(self):
        self.unitsVar.set(FRAMES)
        self.updateDisplay()

    def displaySeconds(self):
        self.unitsVar.set(SECONDS)
        self.updateDisplay()

    def play(self, fLoop):
        if self.frameActiveVar.get():
            if fLoop:
                self['actor'].loop(self['active'])
            else:
                self['actor'].play(self['active'])

    def goTo(self, t):
        if self.unitsVar.get() == FRAMES:
            self['actor'].pose(self['active'], string.atoi(t))
        else:
            self['actor'].pose(self['active'], int(string.atof(t) * self.fps))

    def stop(self):
        if self.frameActiveVar.get():
            self['actor'].stop()

    def resetToZero(self):
        self.frameControl.set(0.0)
        
    def resetToEnd(self):
        if self.unitsVar.get() == FRAMES:
            t = self.maxFrame
        else:
            t = self.maxFrame / self.fps
        self.frameControl.set(t)
        
        
