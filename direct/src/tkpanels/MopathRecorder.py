""" Mopath Recorder Panel Module """

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
from AppShell import *
from DirectGeometry import *
import Pmw
import Dial
import Floater
import EntryScale

"""
To Add
Num Segs lines between knots
Num Ticks
"""

class MopathRecorder(AppShell, PandaObject):
    # Override class variables here
    appname = 'Mopath Recorder Panel'
    frameWidth      = 450
    frameHeight     = 600
    usecommandarea = 0
    usestatusarea  = 0
    count = 0

    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        name = 'recorder-%d' % MopathRecorder.count
        MopathRecorder.count += 1
        optiondefs = (
            ('title',       self.appname,       None),
            ('nodePath',    direct.camera,      None),
            ('name',        name,               None)
            )
        self.defineoptions(kw, optiondefs)

        # Call superclass initialization function
        AppShell.__init__(self)
        
        self.initialiseoptions(MopathRecorder)

    def appInit(self):
        # Dictionary of widgets
        self.widgetDict = {}
        self.variableDict = {}
        # Initialize state
        self.tempCS = direct.group.attachNewNode('mopathRecorderTempCS')
        # Count of point sets recorded
        self.pointSet = []
        self.pointSetDict = {}
        self.pointSetCount = 0
        self.pointSetName = self['name'] + '-ps-' + `self.pointSetCount`
        # Hook to start/stop recording
        self.startStopHook = 'f6'
        # Curve fitter object
        self.xyzCurveFitter = CurveFitter()
        self.hprCurveFitter = CurveFitter()
        # Curve objects
        self.nurbsCurve = NurbsCurve()
        self.nurbsCurveDrawer = NurbsCurveDrawer(self.nurbsCurve)
        self.curveNodePath = None
        # Number of ticks per parametric unit
        self.numTicks = 1
        # Number of segments to represent each parametric unit
        # This just affects the visual appearance of the curve
        self.numSegs = 2
        # Set up event hooks
        self.undoEvents = [('undo', self.undoHook),
                           ('pushUndo', self.pushUndoHook),
                           ('undoListEmpty', self.undoListEmptyHook),
                           ('redo', self.redoHook),
                           ('pushRedo', self.pushRedoHook),
                           ('redoListEmpty', self.redoListEmptyHook)]
        for event, method in self.undoEvents:
            self.accept(event, method)

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        # Clean up things when you destroy the panel
        interior.bind('<Destroy>', self.onDestroy)

        # Add mopath recorder commands to menubar
        self.menuBar.addmenu('Recorder', 'Mopath Recorder Panel Operations')
        self.menuBar.addcascademenu(
            'Recorder', 'Show/Hide',
            statusHelp = 'Show/Hide Mopath Recorder Objects',
            tearoff = 1)
        self.traceVis = BooleanVar()
        self.traceVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Trace Visability',
                                 label = 'Toggle Trace Vis',
                                 variable = self.traceVis,
                                 command = self.setTraceVis)
        self.pathVis = BooleanVar()
        self.pathVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Path Visability',
                                 label = 'Toggle Path Vis',
                                 variable = self.pathVis,
                                 command = self.setPathVis)
        self.hullVis = BooleanVar()
        self.hullVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Hull Visability',
                                 label = 'Toggle Hull Vis',
                                 variable = self.hullVis,
                                 command = self.setHullVis)
        self.cvVis = BooleanVar()
        self.cvVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle CV Visability',
                                 label = 'Toggle CV Vis',
                                 variable = self.cvVis,
                                 command = self.setCvVis)
        self.knotVis = BooleanVar()
        self.knotVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Knot Visability',
                                 label = 'Toggle Knot Vis',
                                 variable = self.knotVis,
                                 command = self.setKnotVis)
        self.tickVis = BooleanVar()
        self.tickVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Tick Visability',
                                 label = 'Toggle Tick Vis',
                                 variable = self.tickVis,
                                 command = self.setTickVis)
        self.markerVis = BooleanVar()
        self.markerVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Marker Visability',
                                 label = 'Toggle Marker Vis',
                                 variable = self.markerVis,
                                 command = self.setMarkerVis)
        self.menuBar.addmenuitem(
            'Recorder', 'command',
            'Toggle widget visability',
            label = 'Toggle Widget Vis',
            command = direct.toggleWidgetVis)
        self.menuBar.addmenuitem(
            'Recorder', 'command',
            'Toggle widget manipulation mode',
            label = 'Toggle Widget Mode',
            command = direct.manipulationControl.toggleObjectHandlesMode)

        self.createComboBox(self.menuFrame, 'Mopath', 'Point Set',
                            'Select input points to fit curve to', '',
                            self.selectPointSetNamed)

        self.undoButton = Button(self.menuFrame, text = 'Undo',
                                 command = direct.undo)
        if direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = 'left', expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(self.menuFrame, text = 'Redo',
                                 command = direct.redo)
        if direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = 'left', expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # Create notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        self.recordPage = self.mainNotebook.add('Record')
        self.cvPage = self.mainNotebook.add('CV Controls')
        self.refinePage = self.mainNotebook.add('Refine')
        # Put this here so it isn't called right away
        self.mainNotebook['raisecommand'] = self.updateInfo

        ## RECORD PAGE ##
        recordFrame = Frame(self.recordPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(recordFrame, text = 'RECORD PATH',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = 'x')
        # Recording Buttons
        frame = Frame(recordFrame)
        self.createCheckbutton(frame, 'Recording', 'Recording Path',
                               'On: path is being recorded',
                               self.toggleRecord, 0,
                               side = 'left')
        self.getWidget('Recording', 'Recording Path').configure(
            foreground = 'Red', relief = RAISED, borderwidth = 2,
            anchor = CENTER)
        self.getWidget('Recording', 'Recording Path').pack(
            fill = 'x', expand = 1)
        self.createLabeledEntry(frame, 'Recording', 'Start/Stop Hook',
                                'Hook used to start/stop recording',
                                initialValue = self.startStopHook,
                                command = self.setStartStopHook)
        self.setStartStopHook(None)
        frame.pack(fill = 'x', expand = 1)
        recordFrame.pack(fill = 'x')
        # Playback controls
        playbackFrame = Frame(self.recordPage, relief = SUNKEN,
                              borderwidth = 2)
        Label(playbackFrame, text = 'PLAYBACK CONTROLS',
              font=('MSSansSerif', 12, 'bold')).pack(fill = 'x')
        self.createEntryScale(playbackFrame, 'Playback', 'Time',
                              'Set current playback time',
                              resolution = 0.01,
                              command = self.setPlaybackTime)
        frame = Frame(playbackFrame)
        self.createButton(frame, 'Playback', 'Stop', 'Stop playback',
                          self.stopPlayback, side = 'left', expand = 1)
        self.createButton(frame, 'Playback', 'Play', 'Start playback',
                          self.startPlayback, side = 'left', expand = 1)
        self.createButton(frame, 'Playback', 'Pause', 'Pause playback',
                          self.pausePlayback, side = 'left', expand = 1)
        frame.pack(fill = 'x', expand = 1)
        playbackFrame.pack(fill = 'x')

        ## CV PAGE ##
        cvFrame = Frame(self.cvPage, relief = SUNKEN,
                        borderwidth = 2)
        label = Label(cvFrame, text = 'CV CONTROLS',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = 'x')
        self.createEntryScale(cvFrame, 'CV Controls', 'Delta Pos',
                              'Position threshold between selected points',
                              resolution = 0.01,
                              command = self.setDeltaPos)
        self.createEntryScale(cvFrame, 'CV Controls', 'Delta Hpr',
                              'Orientation threshold between selected points',
                              resolution = 0.01,
                              command = self.setDeltaHpr)
        self.createEntryScale(cvFrame, 'CV Controls', 'Delta Time',
                              'Time threshold between selected points',
                              resolution = 0.01,
                              command = self.setDeltaTime)

        # Constant velocity frame
        frame = Frame(cvFrame)
        self.createCheckbutton(frame, 'CV Controls', 'Constant Velocity',
                               'On: Resulting path has constant velocity',
                               self.toggleConstantVelocity, 0,
                               side = 'left')
        self.getWidget('CV Controls', 'Constant Velocity').configure(
            relief = RAISED, borderwidth = 2, anchor = CENTER)
        self.getWidget('CV Controls', 'Constant Velocity').pack(
            fill = 'x', expand = 1)
        self.createLabeledEntry(frame, 'CV Controls', 'Path Duration',
                                'Set total curve duration',
                                command = self.setTotalTime)
        frame.pack(fill = 'x', expand = 1)
        cvFrame.pack(fill = 'x')

        ## REFINE PAGE ##
        refineFrame = Frame(self.refinePage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(refineFrame, text = 'REFINE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = 'x')
        self.createEntryScale(refineFrame, 'Refine Page', 'From',
                              'Begin time of refine pass',
                              resolution = 0.01,
                              command = self.setRefineStart)
        self.createEntryScale(refineFrame, 'Refine Page', 'To',
                              'Stop time of refine pass',
                              resolution = 0.01,
                              command = self.setRefineStop)
        refineFrame.pack(fill = 'x')

        offsetFrame = Frame(self.refinePage)
        self.createButton(offsetFrame, 'Refine Page', 'Offset',
                          'Zero refine curve offset',
                          self.resetOffset, side = 'left')
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'X',
                                'Refine pass X offset', width = 3, expand = 1)
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'Y',
                                'Refine pass Y offset', width = 3, expand = 1)
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'Z',
                                'Refine pass Z offset', width = 3, expand = 1)
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'H',
                                'Refine pass H offset', width = 3, expand = 1)
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'P',
                                'Refine pass P offset', width = 3, expand = 1)
        self.createLabeledEntry(offsetFrame, 'Refine Page', 'R',
                                'Refine pass R offset', width = 3, expand = 1)
        offsetFrame.pack(fill = 'x')

        frame = Frame(self.refinePage)
        self.createButton(frame, 'Refine Page', 'Speed',
                          'Reset refine speed',
                          self.resetRefineSpeed, side = 'left',)

        self.mainNotebook.setnaturalsize()        
        
    def updateInfo(self, page = 'System'):
        pass

    def pushUndo(self, fResetRedo = 1):
        direct.pushUndo([self['nodePath']])

    def undoHook(self):
        # Reflect new changes
        pass

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        direct.pushRedo([self['nodePath']])
        
    def redoHook(self):
        # Reflect new changes
        pass

    def pushRedoHook(self):
        # Make sure button is reactivated
        self.redoButton.configure(state = 'normal')

    def redoListEmptyHook(self):
        # Make sure button is deactivated
        self.redoButton.configure(state = 'disabled')
        
    def onDestroy(self, event):
        # Remove hooks
        for event, method in self.undoEvents:
            self.ignore(event)
        # remove start stop hook
        self.ignore(self.startStopHook)
        self.tempCS.removeNode()

    def createNewPointSet(self):
        self.pointSet = []
        self.pointSetName = self['name'] + '-ps-' + `self.pointSetCount`
        # Update dictionary
        self.pointSetDict[self.pointSetName] = self.pointSet
        # Update combo box
        comboBox = self.getWidget('Mopath', 'Point Set')
        scrolledList = comboBox.component('scrolledlist')
        listbox = scrolledList.component('listbox')
        names = list(listbox.get(0,'end'))
        names.append(self.pointSetName)
        scrolledList.setlist(names)
        # Update count
        self.pointSetCount += 1

    def selectPointSetNamed(self, name):
        self.pointSet = self.pointSetDict.get(name, None)

    def setTraceVis(self):
        print self.traceVis.get()
        
    def setHullVis(self):
        self.nurbsCurveDrawer.setShowHull(self.hullVis.get())
        
    def setPathVis(self):
        if self.pathVis.get():
            self.curveNodePath.show()
        else:
            self.curveNodePath.hide()
        
    def setCvVis(self):
        self.nurbsCurveDrawer.setShowCvs(self.cvVis.get())
        
    def setKnotVis(self):
        self.nurbsCurveDrawer.setShowKnots(self.knotVis.get())

    def setTickVis(self):
        if self.tickVis.get():
            self.nurbsCurveDrawer.setNumTicks(self.numTicks)
        else:
            self.nurbsCurveDrawer.setNumTicks(0)

    def setMarkerVis(self):
        print self.markerVis.get()
        
    def setStartStopHook(self, event):
        # Clear out old hook
        self.ignore(self.startStopHook)
        # Record new one
        hook = self.getVariable('Recording', 'Start/Stop Hook').get()
        self.startStopHook = hook
        # Add new one
        self.accept(self.startStopHook, self.toggleRecordVar)

    def toggleRecordVar(self):
        # Get recording variable
        v = self.getVariable('Recording', 'Recording Path')
        # Toggle it
        v.set(1 - v.get())
        # Call the command
        self.toggleRecord()

    def toggleRecord(self):
        if self.getVariable('Recording', 'Recording Path').get():
            # Kill old task
            taskMgr.removeTasksNamed(self['name'] + '-recordTask')
            # Reset curve fitters
            self.xyzCurveFitter.reset()
            self.hprCurveFitter.reset()
            # Create a new point set to hold raw data
            self.createNewPointSet()
            # Start new task
            t = taskMgr.spawnMethodNamed(
                self.recordTask, self['name'] + '-recordTask')
            t.startTime = globalClock.getTime()
            t.uponDeath = self.endRecordTask
        else:
            # Kill old task
            taskMgr.removeTasksNamed(self['name'] + '-recordTask')
            
    def recordTask(self, state):
        # Record raw data point
        time = globalClock.getTime() - state.startTime
        pos = self['nodePath'].getPos()
        hpr = self['nodePath'].getHpr()
        # Add it to the point set
        self.pointSet.append([time, pos, hpr])
        # Add it to the curve fitters
        self.xyzCurveFitter.addPoint(time, pos )
        self.hprCurveFitter.addPoint(time, hpr)
        return Task.cont

    def endRecordTask(self, state):
        # Create curve
        self.xyzCurveFitter.computeTangents(1)
        self.hprCurveFitter.computeTangents(1)
        self.nurbsCurve = self.xyzCurveFitter.makeNurbs()
        self.nurbsCurveDrawer.setCurve(self.nurbsCurve)
        self.nurbsCurveDrawer.setNumSegs(self.numSegs)
        self.nurbsCurveDrawer.draw()
        self.curveNodePath = render.attachNewNode(
            self.nurbsCurveDrawer.getGeomNode())
        
    def setPlaybackTime(self, time):
        print time

    def stopPlayback(self):
        print 'stop'

    def startPlayback(self):
        print 'start'

    def pausePlayback(self):
        print 'pause'

    def setDeltaPos(self, dPos):
        print dPos

    def setDeltaHpr(self, dHpr):
        print dHpr

    def setDeltaTime(self, dTime):
        print dTime

    def toggleConstantVelocity(self):
        print self.getWidget('CV Controls', 'Constant Velocity').get()

    def setTotalTime(self):
        print 'path duration'

    def setRefineStart(self,value):
        print 'refine start'

    def setRefineStop(self, value):
        print 'refine stop'

    def resetOffset(self):
        print 'reset offset'

    def resetRefineSpeed(self):
        pass
    
    ## WIDGET UTILITY FUNCTIONS ##
    def addWidget(self, widget, category, text):
        self.widgetDict[category + '-' + text] = widget
        
    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]

    def createLabeledEntry(self, parent, category, text, balloonHelp,
                           initialValue = '',
                           command = None, relief = 'sunken',
                           side = 'left', expand = 1, width = 12):
        variable = StringVar()
        variable.set(initialValue)
        frame = Frame(parent)
        Label(frame, text = text).pack(side = 'left', fill = 'x')
        entry = Entry(frame, width = width, relief = relief,
                      textvariable = variable)
        entry.pack(side = 'left', fill = 'x', expand = expand)
        self.widgetDict[category + '-' + text] = entry
        self.variableDict[category + '-' + text] = variable
        if command:
            entry.bind('<Return>', command)
        frame.pack(side = side, fill = 'x', expand = expand)

    def createButton(self, parent, category, text, balloonHelp, command,
                     side = 'top', expand = 0):
        widget = Button(parent, text = text)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = X, expand = expand)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget
        
    def createCheckbutton(self, parent, category, text,
                          balloonHelp, command, initialState,
                          side = 'top'):
        bool = BooleanVar()
        bool.set(initialState)
        widget = Checkbutton(parent, text = text, anchor = W,
                         variable = bool)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        self.variableDict[category + '-' + text] = bool
        return widget
        
    def createRadiobutton(self, parent, side, category, text,
                          balloonHelp, variable, value,
                          command):
        widget = Radiobutton(parent, text = text, anchor = W,
                             variable = variable, value = value)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget
        
    def createFloater(self, parent, category, text, balloonHelp,
                      command = None, min = 0.0, resolution = None,
                      maxVelocity = 10.0, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['maxVelocity'] = maxVelocity
        kw['resolution'] = resolution
        widget = apply(Floater.Floater, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createAngleDial(self, parent, category, text, balloonHelp,
                        command = None, **kw):
        kw['text'] = text
        widget = apply(Dial.AngleDial,(parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createEntryScale(self, parent, category, text, balloonHelp,
                         command = None, min = 0.0, max = 1.0,
                         resolution = None, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['max'] = max
        kw['resolution'] = resolution
        widget = apply(EntryScale.EntryScale, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createVector2Entry(self, parent, category, text, balloonHelp,
                           command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.Vector2Entry, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createVector3Entry(self, parent, category, text, balloonHelp,
                           command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.Vector3Entry, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createColorEntry(self, parent, category, text, balloonHelp,
                         command = None, **kw):
        # Set label's text
        kw['text'] = text
        widget = apply(VectorWidgets.ColorEntry, (parent,) ,kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createOptionMenu(self, parent, category, text, balloonHelp,
                         items, command):
        optionVar = StringVar()
        if len(items) > 0:
            optionVar.set(items[0])
        widget = Pmw.OptionMenu(parent, labelpos = W, label_text = text,
                                label_width = 12, menu_tearoff = 1,
                                menubutton_textvariable = optionVar,
                                items = items)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget.component('menubutton'), balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        self.variableDict[category + '-' + text] = optionVar
        return optionVar

    def createComboBox(self, parent, category, text, balloonHelp,
                         items, command, history = 0):
        widget = Pmw.ComboBox(parent,
                              labelpos = W,
                              label_text = text,
                              label_anchor = 'e',
                              label_width = 12,
                              entry_width = 16,
                              history = history,
                              scrolledlist_items = items)
        # Don't allow user to edit entryfield
        widget.configure(entryfield_entry_state = 'disabled')
        # Select first item if it exists
        if len(items) > 0:
            widget.selectitem(items[0])
        # Bind selection command
        widget['selectioncommand'] = command
        widget.pack(side = 'left', expand = 0)
        # Bind help
        self.bind(widget, balloonHelp)
        # Record widget
        self.widgetDict[category + '-' + text] = widget
        return widget

