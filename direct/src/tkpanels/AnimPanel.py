"DIRECT Animation Control Panel"

# Import Tkinter, Pmw, and the floater code from this directory tree.
from Tkinter import *
from tkSimpleDialog import askfloat
import Pmw
import string
import math

FRAMES = 0
SECONDS = 1

class AnimPanel(Pmw.MegaToplevel):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',               'Anim Panel',       None),
            ('actorList',           (),                 None),
            ('Actor_label_width',   12,                 None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        Pmw.MegaToplevel.__init__(self, parent)

        # Handle to the toplevels hull
        hull = self.component('hull')

        # A handy little help balloon
        balloon = self.balloon = Pmw.Balloon()
        # Start with balloon help disabled
        self.balloon.configure(state = 'none')

        menuFrame = Frame(hull, relief = GROOVE, bd = 2)
        
        menuBar = Pmw.MenuBar(menuFrame, hotkeys = 1, balloon = balloon)
        menuBar.pack(side = LEFT, expand = 1, fill = X)
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
                            'Reset Actor controls',
                            label = 'Reset all',
                            command = self.resetAll)
        # Exit panel
        menuBar.addmenuitem('AnimPanel', 'command',
                            'Exit Anim Panel',
                            label = 'Exit',
                            command = self.destroy)
        
        menuBar.addmenu('Help', 'Anim Panel Help Operations')
        self.toggleBalloonVar = IntVar()
        self.toggleBalloonVar.set(0)
        menuBar.addmenuitem('Help', 'checkbutton',
                            'Toggle balloon help',
                            label = 'Balloon Help',
                            variable = self.toggleBalloonVar,
                            command = self.toggleBalloon)
        menuFrame.pack(fill = X)

        # Create a frame to hold all the actor controls
        actorFrame = Frame(hull)

        # Create a control for each actor
        index = 0
        self.actorControlList = []
        for actor in self['actorList']:
            ac = self.createcomponent(
                'actorControl%d' % index, (), 'Actor',
                ActorControl, (actorFrame,))
            ac.pack(expand = 1, fill = X)
            self.actorControlList.append(ac)
            index = index + 1

        # Now pack the actor frame
        actorFrame.pack(expand = 1, fill = BOTH)

        # Create a frame to hold the playback controls
        controlFrame = Frame(hull)
        self.playPauseVar = IntVar()
        self.playPauseVar.set(0)
        self.playPauseButton = self.createcomponent(
            'playPause', (), None,
            Checkbutton, (controlFrame,),
            text = 'Play', width = 8,
            variable = self.playPauseVar,
            indicatoron = FALSE)
        self.playPauseButton.pack(side = LEFT, expand = 1, fill = X)

        self.resetButton = self.createcomponent(
            'reset', (), None,
            Button, (controlFrame,),
            text = 'Reset All',
            width = 8,
            command = self.resetAll)
        self.resetButton.pack(side = LEFT, expand = 1, fill = X)
        
        self.loopVar = IntVar()
        self.loopVar.set(0)
        self.loopButton = self.createcomponent(
            'loopButton', (), None,
            Checkbutton, (controlFrame,),
            text = 'Loop', width = 8,
            variable = self.loopVar)
        self.loopButton.pack(side = LEFT, expand = 1, fill = X)

        controlFrame.pack(fill = X)

        # Execute option callbacks
        self.initialiseoptions(AnimPanel)

    def getActorControlAt(self, index):
        return self.actorControlList[index]

    def enableActorControlAt(self,index):
        self.getActorControlAt(index).enableControl()

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

    def resetAll(self):
        for actorControl in self.actorControlList:
            actorControl.reset()

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

class ActorControl(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        INITOPT = Pmw.INITOPT
        DEFAULT_FONT = (('MS', 'Sans', 'Serif'), 12, 'bold')
        DEFAULT_ANIMS = ('neutral', 'run', 'walk')
        optiondefs = (
            ('text',            'Actor',            self._updateLabelText),
            ('actor',           None,               None),
            ('animList',        DEFAULT_ANIMS,      None),
            ('sLabel_width',    5,                  None),
            ('sLabel_font',     DEFAULT_FONT,       None),
            )
        self.defineoptions(kw, optiondefs)
        self.addoptions(
            (('active',      self['animList'][0],    None),)
            )

        # Initialize the superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Handle to the toplevels hull
        interior = self.interior()
        interior.configure(relief = RAISED, bd = 2)

        # Instance variables
        self.offset = 0.0
        self.fps = 24.0
        self.maxFrame = 120
        self.maxSeconds = self.maxFrame / self.fps

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
        labelMenu.add_command(label = 'Reset', command = self.reset)
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
        fpsList = (1,2,4,8,12,15,24,30)
        fpsMenu = self.createcomponent(
            'fpsMenu', (), None,
            Pmw.ComboBox, (interior,),
            labelpos = W, label_text = 'at:',
            entry_width = 4, selectioncommand = self.setFrameRate,
            scrolledlist_items = fpsList)
        fpsMenu.selectitem('24')
        fpsMenu.pack(side = LEFT, padx = 5, expand = 0)

        # A label
        fpsLabel = Label(interior, text = "fps")
        fpsLabel.pack(side = LEFT)

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

    def _updateLabelText(self):
        self._label['text'] = self['text']

    def updateDisplay(self):
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
            print newMin, newMax
            self.frameControl.configure(to = newMax, resolution = 0.1)

    def selectAnimNamed(self, name):
        print 'Selected Anim: ' + name

    def setFrameRate(self, rate):
        self.fps = string.atof(rate)
        self.maxSeconds = self.maxFrame / self.fps
        self.updateDisplay()

    def setOffset(self):
        newOffset = askfloat(title = self['text'],
                             prompt = 'Start offset (seconds):')
        if newOffset:
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

    def reset(self):
        self.offset = 0.0
        self.frameControl.set(0.0)
        self.updateDisplay()
######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    widget = AnimPanel(actorList = (1,2,3))

