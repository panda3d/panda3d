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
        self.playbackMarker = loader.loadModel('models/directmodels/happy')
        self.playbackMarker.reparentTo(self.recorderNodePath)
        # For node path selectors
        self.recNodePathDict = {}
        self.recNodePathDict['marker'] = self.playbackMarker
        self.recNodePathDict['camera'] = direct.camera
        self.recNodePathDict['widget'] = direct.widget
        self.recNodePathNames = ['marker', 'camera', 'widget', 'selected']
        self.pbNodePathDict = {}
        self.pbNodePathDict['marker'] = self.playbackMarker
        self.pbNodePathDict['camera'] = direct.camera
        self.pbNodePathDict['widget'] = direct.widget
        self.pbNodePathNames = ['marker', 'camera', 'widget', 'selected']
        # Count of point sets recorded
        self.pointSet = []
        self.pointSetDict = {}
        self.pointSetCount = 0
        self.pointSetName = self['name'] + '-ps-' + `self.pointSetCount`
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
        self.numSegs = 2
        # The nurbs curves
        self.xyzNurbsCurve = None
        self.hprNurbsCurve = None
        # Curve drawers
        self.nurbsCurveDrawer = NurbsCurveDrawer(NurbsCurve())
        self.nurbsCurveDrawer.setNumSegs(self.numSegs)
        self.curveNodePath = self.recorderNodePath.attachNewNode(
            self.nurbsCurveDrawer.getGeomNode())
        # Playback variables
        self.playbackTime = 0.0
        self.loopPlayback = 1
        # Sample variables
        self.fEven = 0
        self.desampleFrequency = 1
        self.numSamples = 100
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

        # Create notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        self.recordPage = self.mainNotebook.add('Record')
        self.refinePage = self.mainNotebook.add('Refine')
        # Put this here so it isn't called right away
        self.mainNotebook['raisecommand'] = self.updateInfo

        ## RECORD PAGE ##
        recordFrame = Frame(self.recordPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(recordFrame, text = 'RECORD PATH',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)
        # Recording Buttons
        # Record node path
        self.gridFrame = Frame(recordFrame)        
        self.recNodePathMenu = Pmw.ComboBox(
            self.gridFrame, labelpos = W, label_text = 'Record Node Path:',
            entry_width = 20,
            selectioncommand = self.selectRecordNodePathNamed,
            scrolledlist_items = self.recNodePathNames)
        self.recNodePathMenu.selectitem('camera')
        self.recNodePathMenuEntry = (
            self.recNodePathMenu.component('entryfield_entry'))
        self.recNodePathMenuBG = (
            self.recNodePathMenuEntry.configure('background')[3])
        self.recNodePathMenu.grid(row = 0, col = 0, sticky = NSEW)
        self.bind(self.recNodePathMenu,
                  'Select node path to track when recording a new curve')
        # Record type
        self.recordType = StringVar()
        self.recordType.set('Continuous')
        widget = self.createRadiobutton(
            self.gridFrame, 'left',
            'Recording', 'Continuous Recording',
            ('New point added to curve fitter every frame'),
            self.recordType, 'Continuous', self.setRecordType,
            expand = 1)
        widget['anchor'] = 'center'
        widget.pack_forget()
        widget.grid(row = 1, col = 0, sticky = NSEW)
        widget = self.createRadiobutton(
            self.gridFrame, 'left',
            'Recording', 'Keyframe Recording',
            ('Add new point to curve fitter by pressing keyframe button'),
            self.recordType, 'Keyframe', self.setRecordType,
            expand = 1)
        widget['anchor'] = 'center'
        widget.pack_forget()
        widget.grid(row = 1, col = 1, sticky = NSEW)
        # Record button
        widget = self.createCheckbutton(
            self.gridFrame, 'Recording', 'Recording Path',
            'On: path is being recorded', self.toggleRecord, 0,
            side = LEFT, fill = BOTH, expand = 1)
        widget.pack_forget()
        widget.grid(row=2, column=0, sticky = NSEW)
        widget.configure(foreground = 'Red', relief = RAISED, borderwidth = 2,
                         anchor = CENTER, width = 10)
        widget = self.createButton(self.gridFrame, 'Recording', 'Add Key Frame',
                                   'Add Keyframe To Current Path',
                                   self.addKeyframe,
                                   side = LEFT, expand = 1)
        widget.configure(state = 'disabled', width = 24)
        widget.pack_forget()
        widget.grid(row=2, column=1, sticky = NSEW)
        # Keyframe button
        # Hook
        widget = self.createLabeledEntry(
            self.gridFrame, 'Recording', 'Record Hook',
            'Hook used to start/stop recording',
            initialValue = self.startStopHook,
            command = self.setStartStopHook)[0]
        label['width'] = 14
        self.setStartStopHook()
        widget.pack_forget()
        widget.grid(row=3, column=0, sticky = NSEW)
        widget = self.createLabeledEntry(
            self.gridFrame, 'Recording', 'Keyframe Hook',
            'Hook used to add a new keyframe',
            initialValue = self.keyframeHook,
            command = self.setKeyframeHook)[0]
        label['width'] = 14
        self.setKeyframeHook()
        widget.pack_forget()
        widget.grid(row=3, column=1, sticky = NSEW)
        self.gridFrame.pack(fill = X, expand = 1)
        # This gets the widgets to spread out
        self.gridFrame.grid_columnconfigure(1,weight = 1)        
        recordFrame.pack(fill = X, pady = 2)
        # Playback controls
        playbackFrame = Frame(self.recordPage, relief = SUNKEN,
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
            resolution = 0.01, command = self.setPlaybackTime, side = LEFT)
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
        playbackFrame.pack(fill = X, pady = 2)
        # Desample
        desampleFrame = Frame(
            self.recordPage, relief = SUNKEN, borderwidth = 2)
        Label(desampleFrame, text = 'DESAMPLE CURVE',
              font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createEntryScale(
            desampleFrame, 'Resample', 'Points Between Samples',
            'Recompute curve using every nth point',
            resolution = 1, max = 100, command = self.setDesampleFrequency)
        widget.component('hull')['relief'] = RIDGE
        widget.onRelease = widget.onReturnRelease = self.desampleCurve
        desampleFrame.pack(fill = X, expand = 0, pady = 2)
        # Resample
        resampleFrame = Frame(
            self.recordPage, relief = SUNKEN, borderwidth = 2)
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

        ## REFINE PAGE ##
        refineFrame = Frame(self.refinePage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(refineFrame, text = 'REFINE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)
        self.createEntryScale(refineFrame, 'Refine Page', 'From',
                              'Begin time of refine pass',
                              resolution = 0.01,
                              command = self.setRefineStart)
        self.createEntryScale(refineFrame, 'Refine Page', 'To',
                              'Stop time of refine pass',
                              resolution = 0.01,
                              command = self.setRefineStop)
        refineFrame.pack(fill = X)

        offsetFrame = Frame(self.refinePage)
        self.createButton(offsetFrame, 'Refine Page', 'Offset',
                          'Zero refine curve offset',
                          self.resetOffset, side = LEFT)
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
        offsetFrame.pack(fill = X)

        frame = Frame(self.refinePage)
        self.createButton(frame, 'Refine Page', 'Speed',
                          'Reset refine speed',
                          self.resetRefineSpeed, side = LEFT,)

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
        self.ignore(self.keyframeHook)
        self.recorderNodePath.removeNode()

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
            # Continuous or keyframe?
            if self.recordType.get() == 'Continuous':
                # Start new task
                t = taskMgr.spawnMethodNamed(
                    self.recordTask, self['name'] + '-recordTask')
                t.startTime = globalClock.getTime()
                t.uponDeath = self.endRecordTask
            else:
                # Add hook
                self.acceptKeyframeHook()
                # Record first point
                self.startPos = self['nodePath'].getPos()
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
                self.addKeyframe()
                # Ignore hook
                self.ignoreKeyframeHook()
                # Compute curve
                self.computeCurves()
            # Now you can change record modes
            self.getWidget('Recording', 'Continuous Recording')['state'] = (
                'normal')
            self.getWidget('Recording', 'Keyframe Recording')['state'] = (
                'normal')
            
    def recordTask(self, state):
        # Record raw data point
        time = globalClock.getTime() - state.startTime
        self.recordPoint(time)
        return Task.cont

    def endRecordTask(self, state):
        self.computeCurves()

    def recordPoint(self, time):
        pos = self['nodePath'].getPos()
        hpr = self['nodePath'].getHpr()
        # Add it to the point set
        self.pointSet.append([time, pos, hpr])
        # Add it to the curve fitters
        self.xyzCurveFitter.addPoint(time, pos )
        self.hprCurveFitter.addPoint(time, hpr)
        self.fHasPoints = 1

    def addKeyframe(self):
        time = Vec3(self['nodePath'].getPos() - self.startPos).length()
        self.recordPoint(time)

    def computeCurves(self):
        # MRM: Would be better if curvefitter had getNumPoints
        if not self.fHasPoints:
            print 'MopathRecorder: Must define curve first'
            return
        # Create curves
        # XYZ
        self.xyzCurveFitter.computeTangents(1)
        self.xyzNurbsCurve = self.xyzCurveFitter.makeNurbs()
        self.nurbsCurveDrawer.setCurve(self.xyzNurbsCurve)
        self.nurbsCurveDrawer.draw()
        # HPR
        self.hprCurveFitter.wrapHpr()
        self.hprCurveFitter.computeTangents(1)
        self.hprNurbsCurve = self.hprCurveFitter.makeNurbs()
        # Update widget based on new curve
        self.updateCurveInfo()

    def updateCurveInfo(self):
        if not self.xyzNurbsCurve:
            return
        # Widgets depending on max T
        maxT = '%.2f' % self.xyzNurbsCurve.getMaxT()
        self.getWidget('Playback', 'Time').configure(max = maxT)
        self.getVariable('Resample', 'Path Duration').set(maxT)
        self.getWidget('Refine Page', 'From').configure(max = maxT)
        self.getWidget('Refine Page', 'To').configure(max = maxT)
        self.maxT = float(maxT)
        # Widgets depending on number of knots
        numKnots = self.xyzNurbsCurve.getNumKnots()
        self.getWidget('Resample', 'Points Between Samples')['max'] = numKnots
        self.getWidget('Resample', 'Num. Samples')['max'] = 2 * numKnots

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
        if name in ['parent', 'render', 'camera', 'marker']:
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

    def setPlaybackTime(self, time):
        self.playbackGoTo(time)

    def playbackGoTo(self, time):
        if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
            return
        self.playbackTime = CLAMP(time, 0.0, self.maxT)
        if self.xyzNurbsCurve != None:
            pos = Vec3(0)
            self.xyzNurbsCurve.getPoint(self.playbackTime, pos)
            self.playbackNodePath.setPos(pos)
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
            cTime = CLAMP(state.startOffset + dTime, 0.0, self.maxT)
        self.getWidget('Playback', 'Time').set(cTime)
        # Stop task if not looping and at end of curve
        if ((self.loopPlayback == 0) & ((cTime + 0.01) > self.maxT)):
            self.stopPlayback()
            return Task.done
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
        # Record current curve length
        maxT = self.maxT
        # NOTE: This is destructive, points will be deleted from curve fitter
        self.xyzCurveFitter.desample(self.desampleFrequency)
        self.hprCurveFitter.desample(self.desampleFrequency)
        self.computeCurves()
        # Resize curve to original duration
        self.setPathDurationTo(maxT)

    def setNumSamples(self, numSamples):
        self.numSamples = int(numSamples)
        
    def sampleCurve(self):
        if (self.xyzNurbsCurve == None) & (self.hprNurbsCurve == None):
            print 'MopathRecorder: Must define curve first'
            return
        # Record current curve length
        maxT = self.maxT
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Get new data points based on given curve
        self.xyzCurveFitter.sample(
            self.xyzNurbsCurve, self.numSamples, self.fEven)
        self.hprCurveFitter.sample(
            self.hprNurbsCurve, self.numSamples, self.fEven)
        self.computeCurves()
        # Resize curve to original duration
        self.setPathDurationTo(maxT)

    def setEven(self):
        self.fEven = self.getVariable('Resample', 'Even').get()

    def setPathDuration(self, event):
        newMaxT = float(self.getWidget('Resample', 'Path Duration').get())
        self.setPathDurationTo(newMaxT)
        
    def setPathDurationTo(self, newMaxT):
        sf = newMaxT/self.maxT
        # Scale knots
        for i in range(self.xyzNurbsCurve.getNumKnots()):
            self.xyzNurbsCurve.setKnot(i, sf * self.xyzNurbsCurve.getKnot(i))
        self.xyzNurbsCurve.recompute()
        for i in range(self.hprNurbsCurve.getNumKnots()):
            self.hprNurbsCurve.setKnot(i, sf * self.hprNurbsCurve.getKnot(i))
        self.hprNurbsCurve.recompute()
        # Update info
        self.updateCurveInfo()

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

