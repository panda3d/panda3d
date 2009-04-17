"""DIRECT Animation Control Panel"""

__all__ = ['AnimPanel', 'ActorControl']



### SEE END OF FILE FOR EXAMPLE USEAGE ###

# Import Tkinter, Pmw, and the floater code from this directory tree.
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *
from tkSimpleDialog import askfloat
from Tkinter import *
import Pmw, string, math, types
from direct.task import Task

FRAMES = 0
SECONDS = 1

class AnimPanel(AppShell):
    # Override class variables
    appname = 'Anim Panel'
    frameWidth  = 675
    frameHeight = 250
    usecommandarea = 0
    usestatusarea  = 0
    index = 0

    def __init__(self, aList =  [], parent = None, session = None, **kw):
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

        # direct session that spawned me, if any, used
        # for certain interactions with the session such
        # as being able to see selected objects/actors
        self.session = session

        self.frameHeight = 60 + (50 * len(self['actorList']))
        self.playList =  []
        self.id = 'AnimPanel_%d' % AnimPanel.index
        AnimPanel.index += 1
        # current index used for creating new actor controls
        self.actorControlIndex = 0
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
                            'Set actor controls to t = 0.0',
                            label = 'Jump all to zero',
                            command = self.resetAllToZero)
        menuBar.addmenuitem('AnimPanel', 'command',
                            'Set Actor controls to end time',
                            label = 'Jump all to end time',
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

        self.actorFrame = None
        self.createActorControls()

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
            'playButton', (), None,
            Button, (controlFrame,),
            text = 'Play', width = 8,
            command = self.playActorControls)
        self.playButton.pack(side = LEFT, expand = 1, fill = X)

        self.stopButton = self.createcomponent(
            'stopButton', (), None,
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

        # add actors and animations, only allowed if a direct
        # session has been specified since these currently require
        # interaction with selected objects
        if (self.session):
            menuBar.addmenuitem('File', 'command',
                                'Set currently selected group of objects as actors to animate.',
                                label = 'Set Actors',
                                command = self.setActors)
            menuBar.addmenuitem('File', 'command',
                                'Load animation file',
                                label = 'Load Anim',
                                command = self.loadAnim)

        controlFrame.pack(fill = X)

    def createActorControls(self):
        # Create a frame to hold all the actor controls
        self.actorFrame = Frame(self.interior())
        # Create a control for each actor
        self.actorControlList = []
        for actor in self['actorList']:
            anims = actor.getAnimNames()
            print "actor animnames: %s"%anims
            topAnims = []
            if 'neutral' in anims:
                i = anims.index('neutral')
                del(anims[i])
                topAnims.append('neutral')
            if 'walk' in anims:
                i = anims.index('walk')
                del(anims[i])
                topAnims.append('walk')
            if 'run' in anims:
                i = anims.index('run')
                del(anims[i])
                topAnims.append('run')
            anims.sort()
            anims = topAnims + anims
            if (len(anims)== 0):
                # no animations set for this actor, don't
                # display the control panel
                continue
#            currComponents = self.components()
#            if ('actorControl%d' % index in currComponents):
#                self.destroycomponent('actorControl%d' % index)
#            ac = self.component('actorControl%d' % index)
#            if (ac == None):
            ac = self.createcomponent(
                'actorControl%d' % self.actorControlIndex, (), 'Actor',
                ActorControl, (self.actorFrame,),
                animPanel = self,
                text = actor.getName(),
                animList = anims,
                actor = actor)
            ac.pack(expand = 1, fill = X)
            self.actorControlList.append(ac)
            self.actorControlIndex = self.actorControlIndex + 1

        # Now pack the actor frame
        self.actorFrame.pack(expand = 1, fill = BOTH)

    def clearActorControls(self):
        if (self.actorFrame):
            self.actorFrame.forget()
            self.actorFrame.destroy()
            self.actorFrame = None

    def setActors(self):
        self.stopActorControls()
        actors = self.session.getSelectedActors()
        # make sure selected objects are actors, if not don't
        # use?
        aList = []
        for currActor in actors:
            aList.append(currActor)
        self['actorList'] = aList

        self.clearActorControls()
        self.createActorControls()

    def loadAnim(self):
        # bring up file open box to allow selection of an
        # animation file
        animFilename = askopenfilename(
            defaultextension = '.mb',
            filetypes = (('Maya Models', '*.mb'),
                         ('All files', '*')),
            initialdir = '/i/beta',
            title = 'Load Animation',
            parent = self.component('hull')
            )
        if (animFilename == ''):
            # no file selected, canceled
            return

        # add directory where animation was loaded from to the
        # current model path so any further searches for the file
        # can find it
        fileDirName = os.path.dirname(animFilename)
        fileBaseName = os.path.basename(animFilename)
        fileBaseNameBase = os.path.splitext(fileBaseName)[0]
        fileDirNameFN = Filename(fileDirName)
        fileDirNameFN.makeCanonical()
        getModelPath().prependDirectory(fileDirNameFN)
        for currActor in self['actorList']:
            # replace all currently loaded anims with specified one
#            currActor.unloadAnims(None, None, None)
            currActor.loadAnims({fileBaseNameBase:fileBaseNameBase})
        self.clearActorControls()
        self.createActorControls()


    def playActorControls(self):
        self.stopActorControls()
        self.lastT = globalClock.getFrameTime()
        self.playList = self.actorControlList[:]
        taskMgr.add(self.play, self.id + '_UpdateTask')

    def play(self, task):
        if not self.playList:
            return Task.done
        fLoop = self.loopVar.get()
        currT = globalClock.getFrameTime()
        deltaT = currT - self.lastT
        self.lastT = currT
        for actorControl in self.playList:
            # scale time by play rate value
            actorControl.play(deltaT * actorControl.playRate, fLoop)
        return Task.cont

    def stopActorControls(self):
        taskMgr.remove(self.id + '_UpdateTask')

    def getActorControlAt(self, index):
        return self.actorControlList[index]

    def enableActorControlAt(self, index):
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

    def disableActorControlAt(self, index):
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
                        
    def destroy(self):    
        # First clean up 
        taskMgr.remove(self.id + '_UpdateTask')
        AppShell.destroy(self)    

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
            ('animPanel',       None,               None),
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
        self.fps = 24
        self.offset = 0.0
        self.maxSeconds = 1.0
        self.currT = 0.0
        self.fScaleCommand = 0
        self.fOneShot = 0

        # Create component widgets
        self._label = self.createcomponent(
            'label', (), None,
            Menubutton, (interior,),
            font=('MSSansSerif', 14, 'bold'),
            relief = RAISED, bd = 1,
            activebackground = '#909090',
            text = self['text'])
        # Top level menu
        labelMenu = Menu(self._label, tearoff = 0)

        # Menu to select display mode
        self.unitsVar = IntVar()
        self.unitsVar.set(FRAMES)
        displayMenu = Menu(labelMenu, tearoff = 0)
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
        # labelMenu.add_command(label = 'Set Offset', command = self.setOffset)
        labelMenu.add_command(label = 'Jump To Zero',
                              command = self.resetToZero)
        labelMenu.add_command(label = 'Jump To End Time',
                              command = self.resetToEnd)
        # Now associate menu with menubutton
        self._label['menu'] = labelMenu
        self._label.pack(side = LEFT, fill = X)

        # Combo box to select current animation
        self.animMenu = self.createcomponent(
            'animMenu', (), None,
            Pmw.ComboBox, (interior,),
            labelpos = W, label_text = 'Anim:',
            entry_width = 12, selectioncommand = self.selectAnimNamed,
            scrolledlist_items = self['animList'])
        self.animMenu.selectitem(self['active'])
        self.animMenu.pack(side = 'left', padx = 5, expand = 0)

        # Combo box to select frame rate
        playRateList = ['1/24.0', '0.1', '0.5', '1.0', '2.0', '5.0', '10.0']
        playRate = '%0.1f' % self['actor'].getPlayRate(self['active'])
        if playRate not in playRateList:
            def strCmp(a, b):
                return cmp(eval(a), eval(b))
            playRateList.append(playRate)
            playRateList.sort(strCmp)
        playRateMenu = self.createcomponent(
            'playRateMenu', (), None,
            Pmw.ComboBox, (interior,),
            labelpos = W, label_text = 'Play Rate:',
            entry_width = 4, selectioncommand = self.setPlayRate,
            scrolledlist_items = playRateList)
        playRateMenu.selectitem(playRate)
        playRateMenu.pack(side = LEFT, padx = 5, expand = 0)

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
            from_ = 0, to = 24, resolution = 1.0,
            command = self.goTo,
            orient = HORIZONTAL, showvalue = 1)
        self.frameControl.pack(side = LEFT, expand = 1)
        self.frameControl.bind('<Button-1>', self.__onPress)
        self.frameControl.bind('<ButtonRelease-1>', self.__onRelease)

        self.maxLabel = self.createcomponent(
            'maxLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 24)
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
        self.playRate = 1.0
        self.updateDisplay()

    def _updateLabelText(self):
        self._label['text'] = self['text']

    def updateDisplay(self):
        actor = self['actor']
        active = self['active']
        self.fps = actor.getFrameRate(active)
        if (self.fps == None):
            # there was probably a problem loading the
            # active animation, set default anim properties
            print "unable to get animation fps, zeroing out animation info"
            self.fps = 24
            self.duration = 0
            self.maxFrame = 0
            self.maxSeconds = 0
        else:
            self.duration = actor.getDuration(active)
            self.maxFrame = actor.getNumFrames(active) - 1
            self.maxSeconds = self.offset + self.duration
        # switch between showing frame counts and seconds
        if self.unitsVar.get() == FRAMES:
            # these are approximate due to discrete frame size
            fromFrame = 0
            toFrame = self.maxFrame
            self.minLabel['text'] = fromFrame
            self.maxLabel['text'] = toFrame
            self.frameControl.configure(from_ = fromFrame,
                                        to = toFrame,
                                        resolution = 1.0)
        else:
            self.minLabel['text'] = '0.0'
            self.maxLabel['text'] = "%.2f" % self.duration
            self.frameControl.configure(from_ = 0.0,
                                        to = self.duration,
                                        resolution = 0.01)

    def __onPress(self, event):
        # Enable slider command
        self.fScaleCommand = 1

    def __onRelease(self, event):
        # Disable slider command
        self.fScaleCommand = 0

    def selectAnimNamed(self, name):
        # Update active anim
        self['active'] = name
        # Reset play rate
        self.component('playRateMenu').selectitem('1.0')
        self.setPlayRate('1.0')
        # Move slider to zero
        self.resetToZero()

    def setPlayRate(self, rate):
        # set play rate on the actor, although for the AnimPanel
        # purpose we dont use the actor's play rate, but rather
        # the self.playRate value since we drive the animation
        # playback ourselves
        self['actor'].setPlayRate(eval(rate), self['active'])
        self.playRate = eval(rate)
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

    def play(self, deltaT, fLoop):
        if self.frameActiveVar.get():
            # Compute new time
            self.currT = self.currT + deltaT
            if fLoop and self.duration:
                # If its looping compute modulo
                loopT = self.currT % self.duration                
                self.goToT(loopT)
            else:
                if (self.currT > self.maxSeconds):
                    # Clear this actor control from play list
                    taskMgr.remove()
                else:
                    self.goToT(self.currT)
        else:
            # Clear this actor control from play list
            self['animPanel'].playList.remove(self)

    def goToF(self, f):
        if self.unitsVar.get() == FRAMES:
            self.frameControl.set(f)
        else:
            self.frameControl.set(f/self.fps)

    def goToT(self, t):
        if self.unitsVar.get() == FRAMES:
            self.frameControl.set(t * self.fps)
        else:
            self.frameControl.set(t)

    def goTo(self, t):
        # Convert scale value to float
        t = string.atof(t)
        # Now convert t to seconds for offset calculations
        if self.unitsVar.get() == FRAMES:
            t = t / self.fps
        # Update currT
        if self.fScaleCommand or self.fOneShot:
            self.currT = t
            self.fOneShot = 0
        # Now update actor (pose specifed as frame count)
        self['actor'].pose(self['active'],
                           min(self.maxFrame, int(t * self.fps)))

    def resetToZero(self):
        # This flag forces self.currT to be updated to new value
        self.fOneShot = 1
        self.goToT(0)

    def resetToEnd(self):
        # This flag forces self.currT to be updated to new value
        self.fOneShot = 1
        self.goToT(self.duration)


"""
# EXAMPLE CODE
from direct.actor import Actor
import AnimPanel

a = Actor.Actor({250:{"head":"phase_3/models/char/dogMM_Shorts-head-250",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-250",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-250"},
                 500:{"head":"phase_3/models/char/dogMM_Shorts-head-500",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-500",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-500"},
                 1000:{"head":"phase_3/models/char/dogMM_Shorts-head-1000",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-1000",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-1000"}},
                {"head":{"walk":"phase_3/models/char/dogMM_Shorts-head-walk", \
                         "run":"phase_3/models/char/dogMM_Shorts-head-run"}, \
                 "torso":{"walk":"phase_3/models/char/dogMM_Shorts-torso-walk", \
                          "run":"phase_3/models/char/dogMM_Shorts-torso-run"}, \
                 "legs":{"walk":"phase_3/models/char/dogMM_Shorts-legs-walk", \
                         "run":"phase_3/models/char/dogMM_Shorts-legs-run"}})
a.attach("head", "torso", "joint-head", 250)
a.attach("torso", "legs", "joint-hips", 250)
a.attach("head", "torso", "joint-head", 500)
a.attach("torso", "legs", "joint-hips", 500)
a.attach("head", "torso", "joint-head", 1000)
a.attach("torso", "legs", "joint-hips", 1000)
a.drawInFront("joint-pupil?", "eyes*", -1, lodName=250)
a.drawInFront("joint-pupil?", "eyes*", -1, lodName=500)
a.drawInFront("joint-pupil?", "eyes*", -1, lodName=1000)
a.setLOD(250, 250, 75)
a.setLOD(500, 75, 15)
a.setLOD(1000, 15, 1)
a.fixBounds()
a.reparentTo(render)


a2 = Actor.Actor({250:{"head":"phase_3/models/char/dogMM_Shorts-head-250",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-250",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-250"},
                 500:{"head":"phase_3/models/char/dogMM_Shorts-head-500",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-500",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-500"},
                 1000:{"head":"phase_3/models/char/dogMM_Shorts-head-1000",
                      "torso":"phase_3/models/char/dogMM_Shorts-torso-1000",
                      "legs":"phase_3/models/char/dogMM_Shorts-legs-1000"}},
                {"head":{"walk":"phase_3/models/char/dogMM_Shorts-head-walk", \
                         "run":"phase_3/models/char/dogMM_Shorts-head-run"}, \
                 "torso":{"walk":"phase_3/models/char/dogMM_Shorts-torso-walk", \
                          "run":"phase_3/models/char/dogMM_Shorts-torso-run"}, \
                 "legs":{"walk":"phase_3/models/char/dogMM_Shorts-legs-walk", \
                         "run":"phase_3/models/char/dogMM_Shorts-legs-run"}})
a2.attach("head", "torso", "joint-head", 250)
a2.attach("torso", "legs", "joint-hips", 250)
a2.attach("head", "torso", "joint-head", 500)
a2.attach("torso", "legs", "joint-hips", 500)
a2.attach("head", "torso", "joint-head", 1000)
a2.attach("torso", "legs", "joint-hips", 1000)
a2.drawInFront("joint-pupil?", "eyes*", -1, lodName=250)
a2.drawInFront("joint-pupil?", "eyes*", -1, lodName=500)
a2.drawInFront("joint-pupil?", "eyes*", -1, lodName=1000)
a2.setLOD(250, 250, 75)
a2.setLOD(500, 75, 15)
a2.setLOD(1000, 15, 1)
a2.fixBounds()
a2.reparentTo(render)

ap = AnimPanel.AnimPanel([a, a2])

# Alternately
ap = a.animPanel()
ap2 = a2.animPanel()

"""
