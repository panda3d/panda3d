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
import __builtin__

"""
To Add
Num Segs lines between knots
Num Ticks
"""

class MopathRecorder(AppShell, PandaObject):
    # Override class variables here
    appname = 'Mopath Recorder Panel'
    frameWidth      = 450
    frameHeight     = 510
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

        self.selectRecordNodePathNamed('init')
        self.selectPlaybackNodePathNamed('init')

    def appInit(self):
        # Dictionary of widgets
        self.widgetDict = {}
        self.variableDict = {}
        # Initialize state
        self.recorderNodePath = direct.group.attachNewNode(self['name'])
        self.tempCS = self.recorderNodePath.attachNewNode(
            'mopathRecorderTempCS')
        self.editCS = self.recorderNodePath.attachNewNode(
            'mopathRecorderEditCS')
        self.playbackMarker = loader.loadModel('models/directmodels/smiley')
        self.playbackMarker.reparentTo(self.recorderNodePath)
        self.playbackNodePath = None
        self.lastPlaybackNodePath = None
        # For node path selectors
        self.recNodePathDict = {}
        self.recNodePathDict['marker'] = self.playbackMarker
        self.recNodePathDict['camera'] = direct.camera
        self.recNodePathDict['widget'] = direct.widget
        self.recNodePathDict['mopathRecorderTempCS'] = self.tempCS
        self.recNodePathDict['edit CS'] = self.editCS
        self.recNodePathNames = ['marker', 'camera', 'widget', 'selected']
        self.pbNodePathDict = {}
        self.pbNodePathDict['marker'] = self.playbackMarker
        self.pbNodePathDict['camera'] = direct.camera
        self.pbNodePathDict['widget'] = direct.widget
        self.pbNodePathDict['mopathRecorderEditCS'] = self.tempCS
        self.pbNodePathDict['edit CS'] = self.editCS
        self.pbNodePathNames = ['marker', 'camera', 'widget', 'selected']
        # Count of point sets recorded
        self.pointSet = []
        self.prePoints = []
        self.postPoints = []
        self.pointSetDict = {}
        self.pointSetCount = 0
        self.pointSetName = self['name'] + '-ps-' + `self.pointSetCount`
        # User callback to call before recording point
        self.preRecordFunc = None
        # Hook to start/stop recording
        self.startStopHook = 'f6'
        self.keyframeHook = 'f12'
        # Curve fitter object
        self.fHasPoints = 0
        self.startPos = Point3(0)
        self.xyzCurveFitter = CurveFitter()
        self.hprCurveFitter = CurveFitter()
        # Curve variables
        # Number of ticks per parametric unit
        self.numTicks = 1
        # Number of segments to represent each parametric unit
        # This just affects the visual appearance of the curve
        self.numSegs = 5
        # The nurbs curves
        self.xyzNurbsCurve = None
        self.hprNurbsCurve = None
        # Curve drawers
        self.nurbsCurveDrawer = NurbsCurveDrawer(NurbsCurve())
        self.nurbsCurveDrawer.setNumSegs(self.numSegs)
        self.nurbsCurveDrawer.setShowHull(0)
        self.curveNodePath = self.recorderNodePath.attachNewNode(
            self.nurbsCurveDrawer.getGeomNode())
        # Playback variables
        self.playbackTime = 0.0
        self.loopPlayback = 1
        # Sample variables
        self.fEven = 0
        self.desampleFrequency = 1
        self.numSamples = 100
        # Refining curves
        self.fRefine = 0
        self.refineStart = 0.0
        self.controlStart = 0.0
        self.controlStop = 0.0
        self.refineStop = 0.0
        self.cropFrom = 0.0
        self.cropTo = 0.0
        self.fAdjustingValues = 0
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
        self.hullVis.set(0)
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
        self.undoButton.pack(side = LEFT, expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(self.menuFrame, text = 'Redo',
                                 command = direct.redo)
        if direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = LEFT, expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # Playback controls
        playbackFrame = Frame(interior, relief = SUNKEN,
                              borderwidth = 2)
        Label(playbackFrame, text = 'PLAYBACK CONTROLS',
              font=('MSSansSerif', 12, 'bold')).pack(fill = X)
        # Playback modifiers
        frame = Frame(playbackFrame)
        # Playback node path
        self.pbNodePathMenu = Pmw.ComboBox(
            frame, labelpos = W, label_text = 'Playback Node Path:',
            entry_width = 20,
            selectioncommand = self.selectPlaybackNodePathNamed,
            scrolledlist_items = self.pbNodePathNames)
        self.pbNodePathMenu.selectitem('camera')
        self.pbNodePathMenuEntry = (
            self.pbNodePathMenu.component('entryfield_entry'))
        self.pbNodePathMenuBG = (
            self.pbNodePathMenuEntry.configure('background')[3])
        self.pbNodePathMenu.pack(side = LEFT, fill = X, expand = 1)
        self.bind(self.pbNodePathMenu,
                  'Select node path to fly along path during playback')
        # Duration entry
        self.createLabeledEntry(frame, 'Resample', 'Path Duration',
                                'Set total curve duration',
                                command = self.setPathDuration)
        frame.pack(fill = X, expand = 1)
        frame = Frame(playbackFrame)
        widget = self.createEntryScale(
            frame, 'Playback', 'Time', 'Set current playback time',
            resolution = 0.01, command = self.playbackGoTo, side = LEFT)
        widget.component('hull')['relief'] = RIDGE
        # Kill playback task if drag slider
        widget.component('scale').bind(
            '<ButtonPress-1>', lambda e = None, s = self: s.stopPlayback())
        self.createCheckbutton(frame, 'Playback', 'Loop',
                               'On: loop playback',
                               self.setLoopPlayback, self.loopPlayback,
                               side = LEFT, fill = BOTH, expand = 0)
        frame.pack(fill = X, expand = 1)
        # Start stop buttons
        frame = Frame(playbackFrame)
        widget = self.createButton(frame, 'Playback', '<<',
                                   'Jump to start of playback',
                                   self.jumpToStartOfPlayback,
                                   side = LEFT, expand = 1)
        widget['font'] = (('MSSansSerif', 12, 'bold'))
        widget = self.createCheckbutton(frame, 'Playback', 'Play',
                                        'Start/Stop playback',
                                        self.startStopPlayback, 0,
                                        side = LEFT, fill = BOTH, expand = 1)
        widget.configure(anchor = 'center', justify = 'center',
                         relief = RAISED, font = ('MSSansSerif', 12, 'bold'))
        widget = self.createButton(frame, 'Playback', '>>',
                                   'Jump to end of playback',
                                   self.jumpToEndOfPlayback,
                                   side = LEFT, expand = 1)
        widget['font'] = (('MSSansSerif', 12, 'bold'))
        frame.pack(fill = X, expand = 1)

        # Record button
        frame = Frame(playbackFrame)
        widget = self.createCheckbutton(
            frame, 'Recording', 'Record',
            'On: path is being recorded', self.toggleRecord, 0,
            side = LEFT, fill = BOTH, expand = 1)
        widget.configure(foreground = 'Red', relief = RAISED, borderwidth = 2,
                         anchor = CENTER, width = 16)
        widget = self.createButton(frame, 'Recording', 'Add Key Frame',
                                   'Add Keyframe To Current Path',
                                   self.addKeyframe,
                                   side = LEFT, expand = 1)
        widget.configure(state = 'disabled')
        widget = self.createCheckbutton(
            frame, 'Refine Page', 'Refining Path',
            ('On: Next record session refines current path ' +
             'Off: Next record session records a new path'),
            self.toggleRefine, 0,
            side = LEFT, expand = 0)
        frame.pack(expand = 1, fill = X)
        
        playbackFrame.pack(fill = X, pady = 2)

        # Create notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        self.recordPage = self.mainNotebook.add('Record')
        self.resamplePage = self.mainNotebook.add('Resample')
        self.refinePage = self.mainNotebook.add('Refine')
        self.cropPage = self.mainNotebook.add('Crop')

        ## RECORD PAGE ##
        recordFrame = Frame(self.recordPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(recordFrame, text = 'RECORDING OPTIONS',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)
        # Recording Buttons
        # Record node path
        frame = Frame(recordFrame)        
        self.recNodePathMenu = Pmw.ComboBox(
            frame, labelpos = W, label_text = 'Record Node Path:',
            label_width = 16, label_anchor = W, entry_width = 20, 
            selectioncommand = self.selectRecordNodePathNamed,
            scrolledlist_items = self.recNodePathNames)
        self.recNodePathMenu.selectitem('camera')
        self.recNodePathMenuEntry = (
            self.recNodePathMenu.component('entryfield_entry'))
        self.recNodePathMenuBG = (
            self.recNodePathMenuEntry.configure('background')[3])
        self.bind(self.recNodePathMenu,
                  'Select node path to track when recording a new curve')
        self.recNodePathMenu.pack(side = LEFT, expand = 0)
        frame.pack(expand = 1, fill = X)

        # Hooks
        frame = Frame(recordFrame)
        widget = self.createLabeledEntry(
            frame, 'Recording', 'Record Hook',
            'Hook used to start/stop recording',
            initialValue = self.startStopHook,
            command = self.setStartStopHook)[0]
        label = self.getWidget('Recording', 'Record Hook-Label')
        label.configure(width = 16, anchor = W)
        self.setStartStopHook()
        widget = self.createLabeledEntry(
            frame, 'Recording', 'Keyframe Hook',
            'Hook used to add a new keyframe',
            initialValue = self.keyframeHook,
            command = self.setKeyframeHook)[0]
        label = self.getWidget('Recording', 'Keyframe Hook-Label')
        label.configure(width = 16, anchor = W)
        self.setKeyframeHook()
        frame.pack(expand = 1, fill = X)
        # PreRecordFunc
        frame = Frame(recordFrame)
        widget = self.createLabeledEntry(
            frame, 'Recording', 'Pre Record Func',
            'Function called before recording each point',
            command = self.setPreRecordFunc)[0]
        label = self.getWidget('Recording', 'Pre Record Func-Label')
        label.configure(width = 16, anchor = W)
        self.createCheckbutton(frame, 'Recording', 'PRF Active',
                               'On: Pre Record Func enabled',
                               None, 0,
                               side = LEFT, fill = BOTH, expand = 0)
        frame.pack(expand = 1, fill = X)

        # Record type
        frame = Frame(recordFrame)
        self.recordType = StringVar()
        self.recordType.set('Continuous')
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'Continuous Recording',
            ('New point added to curve fitter every frame'),
            self.recordType, 'Continuous', self.setRecordType,
            expand = 1)
        widget['anchor'] = 'center'
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'Keyframe Recording',
            ('Add new point to curve fitter by pressing keyframe button'),
            self.recordType, 'Keyframe', self.setRecordType,
            expand = 1)
        widget['anchor'] = 'center'
        frame.pack(expand = 1, fill = X)

        # Pack record frame
        recordFrame.pack(fill = X, pady = 2)

        ## RESAMPLE PAGE
        label = Label(self.resamplePage, text = 'RESAMPLE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)
        
        # Resample
        resampleFrame = Frame(
            self.resamplePage, relief = SUNKEN, borderwidth = 2)
        label = Label(resampleFrame, text = 'RESAMPLE CURVE',
                      font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createEntryScale(
            resampleFrame, 'Resample', 'Num. Samples',
            'Number of samples in resampled curve',
            resolution = 1, max = 1000, command = self.setNumSamples,
            side = LEFT)
        widget.component('hull')['relief'] = RIDGE
        widget.onRelease = widget.onReturnRelease = self.sampleCurve
        self.createCheckbutton(resampleFrame, 'Resample', 'Even',
                               'On: Resulting path has constant velocity',
                               self.setEven, self.fEven,
                               side = LEFT, fill = BOTH, expand = 0)
        resampleFrame.pack(fill = X, expand = 0, pady = 2)
        # Desample
        desampleFrame = Frame(
            self.resamplePage, relief = SUNKEN, borderwidth = 2)
        Label(desampleFrame, text = 'DESAMPLE CURVE',
              font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createEntryScale(
            desampleFrame, 'Resample', 'Points Between Samples',
            'Recompute curve using every nth point',
            resolution = 1, max = 100, command = self.setDesampleFrequency)
        widget.component('hull')['relief'] = RIDGE
        widget.onRelease = widget.onReturnRelease = self.desampleCurve
        desampleFrame.pack(fill = X, expand = 0, pady = 2)

        ## REFINE PAGE ##
        refineFrame = Frame(self.refinePage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(refineFrame, text = 'REFINE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createEntryScale(refineFrame,
                                       'Refine Page', 'Refine From',
                                       'Begin time of refine pass',
                                       resolution = 0.01,
                                       command = self.setRefineStart)
        widget.onRelease = widget.onReturnRelease = self.getPrePoints
        widget = self.createEntryScale(
            refineFrame, 'Refine Page',
            'Control Start',
            'Time when full control of node path is given during refine pass',
            resolution = 0.01,
            command = self.setControlStart)
        widget.onRelease = widget.onReturnRelease = self.setRefineFlag
        widget = self.createEntryScale(
            refineFrame, 'Refine Page',
            'Control Stop',
            'Time when node path begins transition back to original curve',
            resolution = 0.01,
            command = self.setControlStop)
        widget.onRelease = widget.onReturnRelease = self.setRefineFlag
        widget = self.createEntryScale(refineFrame, 'Refine Page', 'Refine To',
                                       'Stop time of refine pass',
                                       resolution = 0.01,
                                       command = self.setRefineStop)
        widget.onRelease = widget.onReturnRelease = self.getPostPoints
        refineFrame.pack(fill = X)


        ## CROP PAGE ##
        cropFrame = Frame(self.cropPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(cropFrame, text = 'CROP CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createEntryScale(
            cropFrame,
            'Crop Page', 'Crop From',
            'Delete all curve points before this time',
            resolution = 0.01,
            command = self.setCropFrom)

        widget = self.createEntryScale(
            cropFrame,
            'Crop Page', 'Crop To',
            'Delete all curve points after this time',
            resolution = 0.01,
            command = self.setCropTo)

        self.createButton(cropFrame, 'Crop Page', 'Crop Curve',
                          'Crop curve to specified from to times',
                          self.cropCurve, fill = NONE)
        cropFrame.pack(fill = X)

        self.mainNotebook.setnaturalsize()        
        
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
        self.ignore(self.keyframeHook)
        self.recorderNodePath.removeNode()

    def createNewPointSet(self):
        self.pointSetName = self['name'] + '-ps-' + `self.pointSetCount`
        # Update dictionary
        self.pointSet = self.pointSetDict[self.pointSetName] = []
        # Update combo box
        comboBox = self.getWidget('Mopath', 'Point Set')
        scrolledList = comboBox.component('scrolledlist')
        listbox = scrolledList.component('listbox')
        names = list(listbox.get(0,'end'))
        names.append(self.pointSetName)
        scrolledList.setlist(names)
        comboBox.selectitem(self.pointSetName)
        # Update count
        self.pointSetCount += 1

    def selectPointSetNamed(self, name):
        self.pointSet = self.pointSetDict.get(name, None)
        # Reload points into curve fitter
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        for time, pos, hpr in self.pointSet:
            # Add it to the curve fitters
            self.xyzCurveFitter.addPoint(time, pos )
            self.hprCurveFitter.addPoint(time, hpr)
        self.fHasPoints = 1
        # Compute curve
        self.computeCurves()

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
        if self.markerVis.get():
            self.playbackMarker.reparentTo(self.recorderNodePath)
        else:
            self.playbackMarker.reparentTo(hidden)
        
    def setStartStopHook(self, event = None):
        # Clear out old hook
        self.ignore(self.startStopHook)
        # Record new one
        hook = self.getVariable('Recording', 'Record Hook').get()
        self.startStopHook = hook
        # Add new one
        self.accept(self.startStopHook, self.toggleRecordVar)

    def setKeyframeHook(self, event = None):
        # Clear out old hook
        self.ignoreKeyframeHook()
        # Record new one
        hook = self.getVariable('Recording', 'Keyframe Hook').get()
        self.keyframeHook = hook

    def acceptKeyframeHook(self):
        # Add new one
        self.accept(self.keyframeHook, self.addKeyframe)

    def ignoreKeyframeHook(self):
        # Clear out old hook
        self.ignore(self.keyframeHook)

    def setRecordType(self):
        if self.recordType.get() == 'Keyframe':
            self.getWidget('Recording', 'Add Key Frame')['state'] = 'normal'
        else:
            self.getWidget('Recording', 'Add Key Frame')['state'] = 'disabled'

    def toggleRecordVar(self):
        # Get recording variable
        v = self.getVariable('Recording', 'Record')
        # Toggle it
        v.set(1 - v.get())
        # Call the command
        self.toggleRecord()

    def toggleRecord(self):
        if self.getVariable('Recording', 'Record').get():
            # Kill old task
            taskMgr.removeTasksNamed(self['name'] + '-recordTask')
            # Reset curve fitters
            self.xyzCurveFitter.reset()
            self.hprCurveFitter.reset()
            # Remove old curve
            self.nurbsCurveDrawer.hide()
            # Create a new point set to hold raw data
            self.createNewPointSet()
            # Record nopath's parent
            self.nodePathParent = self['nodePath'].getParent()
            # Refine, Continuous or keyframe?
            if self.fRefine | (self.recordType.get() == 'Continuous'):
                if self.fRefine:
                    # Turn off looping playback
                    self.loopPlayback = 0
                    # Update widget to reflect new value
                    self.getVariable('Playback', 'Loop').set(0)
                    # Select tempCS as playback nodepath
                    self.lastPlaybackNodePath = self.playbackNodePath
                    self.selectPlaybackNodePathNamed('mopathRecorderTempCS')
                    # Parent record node path to temp
                    self['nodePath'].reparentTo(self.tempCS)
                    # Align with temp
                    self['nodePath'].setPosHpr(0,0,0,0,0,0)
                    # Set playback start to refineStart
                    self.playbackGoTo(self.refineStart)
                    # start flying nodePath along path
                    self.startPlayback()
                # Start new task
                t = taskMgr.spawnMethodNamed(
                    self.recordTask, self['name'] + '-recordTask')
                t.startTime = globalClock.getTime()
            else:
                # Add hook
                self.acceptKeyframeHook()
                # Record first point
                self.startPos = Point3(
                    self['nodePath'].getPos(self.nodePathParent))
                self.recordPoint(0.0)

            # Don't want to change record modes
            self.getWidget('Recording', 'Continuous Recording')['state'] = (
                'disabled')
            self.getWidget('Recording', 'Keyframe Recording')['state'] = (
                'disabled')
        else:
            if self.recordType.get() == 'Continuous':
                # Kill old task
                taskMgr.removeTasksNamed(self['name'] + '-recordTask')
            else:
                # Add last point
                self.addKeyframe(0)
                # Ignore hook
                self.ignoreKeyframeHook()
            if self.fRefine:
                # Reparent node path back to parent
                self['nodePath'].wrtReparentTo(self.nodePathParent)
                # Restore playback Node Path
                self.playbackNodePath = self.lastPlaybackNodePath
                # Merge prePoints, pointSet, postPoints
                self.mergePoints()
                # Clear out pre and post list
                self.prePoints = []
                self.postPoints = []
                # Reset flag
                self.setRefineFlag(0)
            # Compute curve
            self.computeCurves()
            # Now you can change record modes
            self.getWidget('Recording', 'Continuous Recording')['state'] = (
                'normal')
            self.getWidget('Recording', 'Keyframe Recording')['state'] = (
                'normal')
            
    def recordTask(self, state):
        # Record raw data point
        time = self.refineStart + (globalClock.getTime() - state.startTime)
        self.recordPoint(time)
        return Task.cont

    def addKeyframe(self, fToggleRecord = 1):
        # Make sure we're in a recording mode!
        if (fToggleRecord &
            (not self.getVariable('Recording', 'Record').get())):
            # This will automatically add the first point
            self.toggleRecordVar()
        else:
            time = (self.refineStart +
                    (Vec3(self['nodePath'].getPos(self.nodePathParent) -
                          self.startPos).length()))
            self.recordPoint(time)

    def easeInOut(self, t):
        x = t * t
        return (3 * x) - (2 * t * x)

    def setPreRecordFunc(self, event):
        # Note: If func is one defined at command prompt, need to set
        # __builtins__.func = func at command line
        self.preRecordFunc = eval(
            self.getVariable('Recording', 'Pre Record Func').get())
        # Update widget to reflect new value
        self.getVariable('Recording', 'PRF Active').set(1)

    def recordPoint(self, time):
        # Call user define callback before recording point
        if (self.getVariable('Recording', 'PRF Active').get() &
            (self.preRecordFunc != None)):
            self.preRecordFunc()
        # Get point
        pos = self['nodePath'].getPos(self.nodePathParent)
        hpr = self['nodePath'].getHpr(self.nodePathParent)
        # Blend between recordNodePath and self['nodePath']
        if self.fRefine:
            if (time < self.controlStart):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                t = self.easeInOut(((time - self.refineStart)/
                                    (self.controlStart - self.refineStart)))
                # Transition between the recorded node path and the driven one
                pos = (rPos * (1 - t)) + (pos * t)
                hpr = (rHpr * (1 - t)) + (hpr * t)
            elif (time > self.controlStop):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                t = self.easeInOut(((time - self.controlStop)/
                                    (self.refineStop - self.controlStop)))
                # Transition between the recorded node path and the driven one
                pos = (pos * (1 - t)) + (rPos * t)
                hpr = (hpr * (1 - t)) + (rHpr * t)
        # Add it to the point set
        self.pointSet.append([time, pos, hpr])
        # Add it to the curve fitters
        self.xyzCurveFitter.addPoint(time, pos )
        self.hprCurveFitter.addPoint(time, hpr)
        self.fHasPoints = 1

    def computeCurves(self):
        # MRM: Would be better if curvefitter had getNumPoints
        if not self.fHasPoints:
            print 'MopathRecorder: Must define curve first'
            return
        # Create curves
        # XYZ
        self.xyzCurveFitter.sortPoints()
        self.xyzCurveFitter.computeTangents(1)
        self.xyzNurbsCurve = self.xyzCurveFitter.makeNurbs()
        self.nurbsCurveDrawer.setCurve(self.xyzNurbsCurve)
        self.nurbsCurveDrawer.draw()
        # HPR
        self.hprCurveFitter.sortPoints()
        self.hprCurveFitter.wrapHpr()
        self.hprCurveFitter.computeTangents(1)
        self.hprNurbsCurve = self.hprCurveFitter.makeNurbs()
        # Update widget based on new curve
        self.updateWidgets()

    def updateWidgets(self):
        if not self.xyzNurbsCurve:
            return
        self.fAdjustingValues = 1
        # Widgets depending on max T
        maxT = '%.2f' % self.xyzNurbsCurve.getMaxT()
        self.getWidget('Playback', 'Time').configure(max = maxT)
        self.getVariable('Resample', 'Path Duration').set(maxT)
        widget = self.getWidget('Refine Page', 'Refine From')
        widget.configure(max = maxT)
        widget.set(0.0)
        widget = self.getWidget('Refine Page', 'Control Start')
        widget.configure(max = maxT)
        widget.set(0.0)
        widget = self.getWidget('Refine Page', 'Control Stop')
        widget.configure(max = maxT)
        widget.set(float(maxT))
        widget = self.getWidget('Refine Page', 'Refine To')
        widget.configure(max = maxT)
        widget.set(float(maxT))
        widget = self.getWidget('Crop Page', 'Crop From')
        widget.configure(max = maxT)
        widget.set(float(0.0))
        widget = self.getWidget('Crop Page', 'Crop To')
        widget.configure(max = maxT)
        widget.set(float(maxT))
        self.maxT = float(maxT)
        print 'new maxT', self.maxT
        # Widgets depending on number of samples
        numSamples = self.xyzCurveFitter.getNumSamples()
        widget = self.getWidget('Resample', 'Points Between Samples')
        widget.configure(max=numSamples)
        widget = self.getWidget('Resample', 'Num. Samples')
        widget.configure(max = 3 * numSamples)
        widget.set(numSamples, 0)
        self.fAdjustingValues = 0

    def selectRecordNodePathNamed(self, name):
        nodePath = None
        if name == 'init':
            nodePath = self['nodePath']
            # Add Combo box entry for the initial node path
            self.addRecordNodePath(nodePath)
        elif name == 'selected':
            nodePath = direct.selected.last
            # Add Combo box entry for this selected object
            self.addRecordNodePath(nodePath)
        else:
            nodePath = self.recNodePathDict.get(name, None)
            if (nodePath == None):
                # See if this evaluates into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addRecordNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.recNodePathMenu.component('scrolledlist')
                    listbox.setlist(self.recNodePathNames)
            else:
                if name == 'widget':
                    # Record relationship between selected nodes and widget
                    direct.selected.getWrtAll()                    
        # Update active node path
        self.setRecordNodePath(nodePath)

    def setRecordNodePath(self, nodePath):
        self['nodePath'] = nodePath
        if self['nodePath']:
            self.recNodePathMenuEntry.configure(
                background = self.recNodePathMenuBG)
        else:
            # Flash entry
            self.recNodePathMenuEntry.configure(background = 'Pink')

    def selectPlaybackNodePathNamed(self, name):
        nodePath = None
        if name == 'init':
            nodePath = self['nodePath']
            # Add Combo box entry for the initial node path
            self.addPlaybackNodePath(nodePath)
        elif name == 'selected':
            nodePath = direct.selected.last
            # Add Combo box entry for this selected object
            self.addPlaybackNodePath(nodePath)
        else:
            nodePath = self.pbNodePathDict.get(name, None)
            if (nodePath == None):
                # See if this evaluates into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addPlaybackNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.pbNodePathMenu.component('scrolledlist')
                    listbox.setlist(self.pbNodePathNames)
            else:
                if name == 'widget':
                    # Record relationship between selected nodes and widget
                    direct.selected.getWrtAll()                    
        # Update active node path
        self.setPlaybackNodePath(nodePath)

    def setPlaybackNodePath(self, nodePath):
        self.playbackNodePath = nodePath
        if self.playbackNodePath:
            self.pbNodePathMenuEntry.configure(
                background = self.pbNodePathMenuBG)
        else:
            # Flash entry
            self.pbNodePathMenuEntry.configure(background = 'Pink')

    def addRecordNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.recNodePathNames,
                               self.recNodePathMenu, self.recNodePathDict)

    def addPlaybackNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.pbNodePathNames,
                               self.pbNodePathMenu, self.pbNodePathDict)

    def addNodePathToDict(self, nodePath, names, menu, dict):
        if not nodePath:
            return
        # Get node path's name
        name = nodePath.getName()
        if name in ['mopathRecorderTempCS', 'widget', 'camera', 'marker']:
            dictName = name
        else:
            # Generate a unique name for the dict
            dictName = name + '-' + `nodePath.id().this`
        if not dict.has_key(dictName):
            # Update combo box to include new item
            names.append(dictName)
            listbox = menu.component('scrolledlist')
            listbox.setlist(names)
            # Add new item to dictionary
            dict[dictName] = nodePath
        menu.selectitem(dictName)

    def setLoopPlayback(self):
        self.loopPlayback = self.getVariable('Playback', 'Loop').get()

    def playbackGoTo(self, time):
        if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
            return
        self.playbackTime = CLAMP(time, 0.0, self.maxT)
        if self.xyzNurbsCurve != None:
            pos = Vec3(0)
            self.xyzNurbsCurve.getPoint(self.playbackTime, pos)
            self.playbackNodePath.setPos(pos)
        if self.hprNurbsCurve != None:
            hpr = Vec3(0)
            self.hprNurbsCurve.getPoint(self.playbackTime, hpr)
            self.playbackNodePath.setHpr(hpr)

    def startPlayback(self):
        if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
            return
        # Kill any existing tasks
        self.stopPlayback()
        # Make sure checkbutton is set
        self.getVariable('Playback', 'Play').set(1)
        # Start new playback task
        t = taskMgr.spawnMethodNamed(
            self.playbackTask, self['name'] + '-playbackTask')
        t.startOffset = self.playbackTime
        t.startTime = globalClock.getTime()
        
    def playbackTask(self, state):
        dTime = globalClock.getTime() - state.startTime
        if self.loopPlayback:
            cTime = (state.startOffset + dTime) % self.maxT
        else:
            cTime = state.startOffset + dTime
        # Stop task if not looping and at end of curve
        # Or if refining curve and past refineStop
        if (((self.loopPlayback == 0) & (cTime > self.maxT)) |
            (self.fRefine & (cTime > self.refineStop))):
            self.stopPlayback()
            if self.fRefine:
                # Kill record task
                self.toggleRecordVar()
            return Task.done
        # Otherwise go to specified time
        self.getWidget('Playback', 'Time').set(cTime)
        return Task.cont

    def stopPlayback(self):
        self.getVariable('Playback', 'Play').set(0)
        taskMgr.removeTasksNamed(self['name'] + '-playbackTask')

    def jumpToStartOfPlayback(self):
        self.stopPlayback()
        self.getWidget('Playback', 'Time').set(0.0)

    def jumpToEndOfPlayback(self):
        self.stopPlayback()
        if self.xyzNurbsCurve != None:
            self.getWidget('Playback', 'Time').set(self.maxT)

    def startStopPlayback(self):
        if self.getVariable('Playback', 'Play').get():
            self.startPlayback()
        else:
            self.stopPlayback()

    def setDesampleFrequency(self, frequency):
        self.desampleFrequency = frequency
        
    def desampleCurve(self):
        if not self.fHasPoints:
            print 'MopathRecorder: Must define curve first'
            return
        # NOTE: This is destructive, points will be deleted from curve fitter
        self.xyzCurveFitter.desample(self.desampleFrequency)
        self.hprCurveFitter.desample(self.desampleFrequency)
        self.computeCurves()
        # Get new point set based on newly created curve
        self.createNewPointSet()
        for i in range(self.xyzCurveFitter.getNumSamples()):
            time = self.xyzCurveFitter.getSampleT(i)
            pos = Point3(self.xyzCurveFitter.getSamplePoint(i))
            hpr = Point3(self.hprCurveFitter.getSamplePoint(i))
            self.pointSet.append([time, pos, hpr])

    def setNumSamples(self, numSamples):
        self.numSamples = int(numSamples)
        
    def sampleCurve(self):
        if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
            print 'MopathRecorder: Must define curve first'
            return
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Get new data points based on given curve
        self.xyzCurveFitter.sample(
            self.xyzNurbsCurve, self.numSamples, self.fEven)
        # Now sample the hprNurbsCurve using the resulting times
        for i in range(self.xyzCurveFitter.getNumSamples()):
            t = self.xyzCurveFitter.getSampleT(i)
            hpr = Point3(0)
            self.hprNurbsCurve.getPoint(t, hpr)
            self.hprCurveFitter.addPoint(t, hpr)
        # Now recompute curves
        self.computeCurves()
        # Get new point set based on newly created curve
        self.createNewPointSet()
        for i in range(self.xyzCurveFitter.getNumSamples()):
            time = self.xyzCurveFitter.getSampleT(i)
            pos = Point3(self.xyzCurveFitter.getSamplePoint(i))
            hpr = Point3(self.hprCurveFitter.getSamplePoint(i))
            self.pointSet.append([time, pos, hpr])

    def setEven(self):
        self.fEven = self.getVariable('Resample', 'Even').get()

    def setPathDuration(self, event):
        newMaxT = float(self.getWidget('Resample', 'Path Duration').get())
        self.setPathDurationTo(newMaxT)
        
    def setPathDurationTo(self, newMaxT):
        sf = newMaxT/self.maxT
        # Scale xyz curve knots
        for i in range(self.xyzNurbsCurve.getNumKnots()):
            self.xyzNurbsCurve.setKnot(i, sf * self.xyzNurbsCurve.getKnot(i))
        self.xyzNurbsCurve.recompute()
        # Scale hpr curve knots
        for i in range(self.hprNurbsCurve.getNumKnots()):
            self.hprNurbsCurve.setKnot(i, sf * self.hprNurbsCurve.getKnot(i))
        self.hprNurbsCurve.recompute()
        # Update info
        self.updateWidgets()

    def toggleRefine(self):
        self.fRefine = self.getVariable('Refine Page', 'Refining Path').get()

    def setRefineFlag(self, val = 1):
        self.fRefine = val
        self.getVariable('Refine Page', 'Refining Path').set(val)

    def setRefineStart(self,value):
        self.refineStart = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.refineStart > self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.refineStart)
        if self.refineStart > self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.refineStart)
        if self.refineStart > self.refineStop:
            self.getWidget('Refine Page', 'Refine To').set(self.refineStart)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def getPrePoints(self):
        # Set flag so we know to do a refine pass
        self.setRefineFlag(1)
        # Reset prePoints
        self.prePoints = []
        # See if we need to save any points before refineStart
        for i in range(len(self.pointSet)):
            # Have we passed refineStart?
            if self.refineStart < self.pointSet[i][0]:
                # Get a copy of the points prior to refineStart
                self.prePoints = self.pointSet[:i-1]
                break

    def setControlStart(self, value):
        self.controlStart = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.controlStart < self.refineStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.controlStart)
        if self.controlStart > self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.controlStart)
        if self.controlStart > self.refineStop:
            self.getWidget('Refine Page', 'Refine To').set(
                self.controlStart)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def setControlStop(self, value):
        self.controlStop = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.controlStop < self.refineStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.controlStop)
        if self.controlStop < self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.controlStop)
        if self.controlStop > self.refineStop:
            self.getWidget('Refine Page', 'Refine To').set(
                self.controlStop)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def setRefineStop(self, value):
        self.refineStop = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.refineStop < self.refineStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.refineStop)
        if self.refineStop < self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.refineStop)
        if self.refineStop < self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.refineStop)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def getPostPoints(self):
        # Set flag so we know to do a refine pass
        self.setRefineFlag(1)
        # Reset postPoints
        self.postPoints = []
        # See if we need to save any points after refineStop
        for i in range(len(self.pointSet)):
            # Have we reached refineStop?
            if self.refineStop < self.pointSet[i][0]:
                # Get a copy of the points after refineStop
                self.postPoints = self.pointSet[i:]
                break

    def mergePoints(self):
        # Merge in pre points
        self.pointSet = self.prePoints + self.pointSet
        for time, pos, hpr in self.prePoints:
            # Add it to the curve fitters
            self.xyzCurveFitter.addPoint(time, pos )
            self.hprCurveFitter.addPoint(time, hpr)
        # And post points
        # What is end time of pointSet?
        endTime = self.pointSet[-1][0]
        for time, pos, hpr in self.postPoints:
            adjustedTime = endTime + (time - self.refineStop)
            # Add it to point set
            self.pointSet.append([adjustedTime, pos, hpr])
            # Add it to the curve fitters
            self.xyzCurveFitter.addPoint(adjustedTime, pos)
            self.hprCurveFitter.addPoint(adjustedTime, hpr)

    def setCropFrom(self,value):
        self.cropFrom = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.cropFrom > self.cropTo:
            self.getWidget('Crop Page', 'Crop To').set(
                self.cropFrom)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def setCropTo(self,value):
        self.cropTo = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.cropTo < self.cropFrom:
            self.getWidget('Crop Page', 'Crop From').set(
                self.cropTo)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def cropCurve(self):
        # Keep handle on old points
        oldPoints = self.pointSet
        # Create new point set
        self.createNewPointSet()
        # Copy over points between from/to
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Set flag
        self.fHasPoints = 0
        for time, pos, hpr in oldPoints:
            # Is it within the time?
            if ((time > self.cropFrom) &
                (time < self.cropTo)):
                # Add it to the curve fitters
                t = time - self.cropFrom
                self.xyzCurveFitter.addPoint(t, pos )
                self.hprCurveFitter.addPoint(t, hpr)
                # And the point set
                self.pointSet.append([t, pos, hpr])
                self.fHasPoints = 1
        # Compute curve
        self.computeCurves()

    ## WIDGET UTILITY FUNCTIONS ##
    def addWidget(self, widget, category, text):
        self.widgetDict[category + '-' + text] = widget
        
    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]

    def createLabeledEntry(self, parent, category, text, balloonHelp,
                           initialValue = '', command = None,
                           relief = 'sunken', side = LEFT,
                           expand = 1, width = 12):
        frame = Frame(parent)
        variable = StringVar()
        variable.set(initialValue)
        label = Label(frame, text = text)
        label.pack(side = LEFT, fill = X)
        self.widgetDict[category + '-' + text + '-Label'] = label
        entry = Entry(frame, width = width, relief = relief,
                      textvariable = variable)
        entry.pack(side = LEFT, fill = X, expand = expand)
        self.widgetDict[category + '-' + text] = entry
        self.variableDict[category + '-' + text] = variable
        if command:
            entry.bind('<Return>', command)
        frame.pack(side = side, fill = X, expand = expand)
        return (frame, label, entry)

    def createButton(self, parent, category, text, balloonHelp, command,
                     side = 'top', expand = 0, fill = X):
        widget = Button(parent, text = text)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = fill, expand = expand)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget
        
    def createCheckbutton(self, parent, category, text,
                          balloonHelp, command, initialState,
                          side = 'top', fill = X, expand = 0):
        bool = BooleanVar()
        bool.set(initialState)
        widget = Checkbutton(parent, text = text, anchor = W,
                         variable = bool)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = fill, expand = expand)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        self.variableDict[category + '-' + text] = bool
        return widget
        
    def createRadiobutton(self, parent, side, category, text,
                          balloonHelp, variable, value,
                          command, fill = X, expand = 0):
        widget = Radiobutton(parent, text = text, anchor = W,
                             variable = variable, value = value)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = fill, expand = expand)
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
                         resolution = None,
                         side = TOP, fill = X, expand = 1, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['max'] = max
        kw['resolution'] = resolution
        widget = apply(EntryScale.EntryScale, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = fill, expand = expand)
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
        widget.pack(side = LEFT, expand = 0)
        # Bind help
        self.bind(widget, balloonHelp)
        # Record widget
        self.widgetDict[category + '-' + text] = widget
        return widget

