""" Mopath Recorder Panel Module """

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
from AppShell import *
from DirectGeometry import *
from DirectSelection import *
from tkFileDialog import *
import os
import Pmw
import Dial
import Floater
import EntryScale
import VectorWidgets
import __builtin__

PRF_UTILITIES = [
    'lambda: direct.camera.lookAt(render)',
    'lambda: direct.camera.setZ(render, 0.0)',
    'lambda s = self: s.playbackMarker.lookAt(render)',
    'lambda s = self: s.playbackMarker.setZ(render, 0.0)',
    'lambda s = self: s.followTerrain(10.0)']

class MopathRecorder(AppShell, PandaObject):
    # Override class variables here
    appname = 'Mopath Recorder Panel'
    frameWidth      = 450
    frameHeight     = 535
    usecommandarea = 0
    usestatusarea  = 0
    count = 0

    def __init__(self, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        name = 'recorder-%d' % MopathRecorder.count
        MopathRecorder.count += 1
        optiondefs = (
            ('title',       self.appname,         None),
            ('nodePath',    None,                 None),
            ('name',        name,                 None)
            )
        self.defineoptions(kw, optiondefs)

        # Call superclass initialization function
        AppShell.__init__(self)
        
        self.initialiseoptions(MopathRecorder)

        self.selectRecordNodePathNamed('marker')
        self.selectPlaybackNodePathNamed('marker')

    def appInit(self):
        self.name = self['name']
        # Dictionary of widgets
        self.widgetDict = {}
        self.variableDict = {}
        # Initialize state
        self.recorderNodePath = direct.group.attachNewNode(self.name)
        self.tempCS = self.recorderNodePath.attachNewNode(
            'mopathRecorderTempCS')
        self.nodePathParent = render
        self.playbackMarker = loader.loadModel('models/directmodels/smiley')
        self.playbackMarker.reparentTo(self.recorderNodePath)
        self.playbackMarker.hide()
        self.playbackMarker.setName('Playback Marker')
        # ID of selected object
        self.manipulandumId = None
        self.tangentGroup = self.playbackMarker.attachNewNode('Tangent Group')
        self.tangentGroup.hide()
        self.tangentMarker = loader.loadModel('models/directmodels/sphere')
        self.tangentMarker.reparentTo(self.tangentGroup)
        self.tangentMarker.setScale(0.5)
        self.tangentMarker.setColor(1,0,1,1)
        self.tangentMarker.setName('Tangent Marker')
        self.tangentLines = LineNodePath(self.tangentGroup)
	self.tangentLines.setColor(VBase4(1,0,1,1))
	self.tangentLines.setThickness(1)
	self.tangentLines.moveTo(0,0,0)
        self.tangentLines.drawTo(0,0,0)
        self.tangentLines.create()
        self.trace = LineNodePath(self.recorderNodePath)
        self.playbackNodePath = None
        self.lastPlaybackNodePath = None
        # For node path selectors
        self.recNodePathDict = {}
        self.recNodePathDict['marker'] = self.playbackMarker
        self.recNodePathDict['camera'] = direct.camera
        self.recNodePathDict['widget'] = direct.widget
        self.recNodePathDict['mopathRecorderTempCS'] = self.tempCS
        self.recNodePathNames = ['marker', 'camera', 'selected']
        self.pbNodePathDict = {}
        self.pbNodePathDict['marker'] = self.playbackMarker
        self.pbNodePathDict['camera'] = direct.camera
        self.pbNodePathDict['widget'] = direct.widget
        self.pbNodePathDict['mopathRecorderTempCS'] = self.tempCS
        self.pbNodePathNames = ['marker', 'camera', 'selected']
        # Count of point sets recorded
        self.pointSet = []
        self.prePoints = []
        self.postPoints = []
        self.pointSetDict = {}
        self.pointSetCount = 0
        self.pointSetName = self.name + '-ps-' + `self.pointSetCount`
        # User callback to call before recording point
        self.samplingMode = 'Continuous'
        self.preRecordFunc = None
        # Hook to start/stop recording
        self.startStopHook = 'f6'
        self.keyframeHook = 'f10'
        # Curve fitter object
        self.lastPos = Point3(0)
        self.xyzCurveFitter = CurveFitter()
        self.hprCurveFitter = CurveFitter()
        # Curve variables
        # Number of ticks per parametric unit
        self.numTicks = 1
        # Number of segments to represent each parametric unit
        # This just affects the visual appearance of the curve
        self.numSegs = 40
        # The nurbs curves
        self.xyzNurbsCurve = None
        self.hprNurbsCurve = None
        # Curve drawers
        self.xyzNurbsCurveDrawer = NurbsCurveDrawer(NurbsCurve())
        self.xyzNurbsCurveDrawer.setNumSegs(self.numSegs)
        self.xyzNurbsCurveDrawer.setShowHull(0)
        self.xyzNurbsCurveDrawer.setShowCvs(0)
        self.xyzNurbsCurveDrawer.setNumTicks(0)
        self.curveNodePath = self.recorderNodePath.attachNewNode(
            self.xyzNurbsCurveDrawer.getGeomNode())
        # Playback variables
        self.playbackTime = 0.0
        self.loopPlayback = 1
        self.playbackSF = 1.0
        # Sample variables
        self.fEven = 0
        self.fForward = 0
        self.desampleFrequency = 1
        self.numSamples = 100
        self.recordStart = 0.0
        self.deltaTime = 0.0
        self.controlStart = 0.0
        self.controlStop = 0.0
        self.recordStop = 0.0
        self.cropFrom = 0.0
        self.cropTo = 0.0
        self.fAdjustingValues = 0
        # For terrain following
        self.iRayCS = self.recorderNodePath.attachNewNode(
            'mopathRecorderIRayCS')
        self.iRay = SelectionRay(self.iRayCS)
        # Set up event hooks
        self.actionEvents = [
            ('undo', self.undoHook),
            ('pushUndo', self.pushUndoHook),
            ('undoListEmpty', self.undoListEmptyHook),
            ('redo', self.redoHook),
            ('pushRedo', self.pushRedoHook),
            ('redoListEmpty', self.redoListEmptyHook),
            ('selectedNodePath', self.selectedNodePathHook),
            ('deselectedNodePath', self.deselectedNodePathHook),
            ('manipulateObjectStart', self.manipulateObjectStartHook),
            ('manipulateObjectCleanup', self.manipulateObjectCleanupHook),
            ]
        for event, method in self.actionEvents:
            self.accept(event, method)

    def createInterface(self):
	interior = self.interior()
        # FILE MENU
        # Get a handle on the file menu so commands can be inserted
        # before quit item
        fileMenu = self.menuBar.component('File-menu')
        fileMenu.insert_command(
            fileMenu.index('Quit'),
            label = 'Load Curve',
            command = self.loadCurveFromFile)
        fileMenu.insert_command(
            fileMenu.index('Quit'),
            label = 'Save Curve',
            command = self.saveCurveToFile)

        # Add mopath recorder commands to menubar
        self.menuBar.addmenu('Recorder', 'Mopath Recorder Panel Operations')
        self.menuBar.addmenuitem(
            'Recorder', 'command',
            'Save current curve as a new point set',
            label = 'Save Point Set',
            command = self.savePointSet)
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
                            self.selectPointSetNamed, expand = 1)

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

        # Record button
        frame = Frame(interior, relief = SUNKEN, borderwidth = 2)
        widget = self.createCheckbutton(
            frame, 'Recording', 'Record',
            'On: path is being recorded', self.toggleRecord, 0,
            side = LEFT, fill = BOTH, expand = 1)
        widget.configure(foreground = 'Red', relief = RAISED, borderwidth = 2,
                         anchor = CENTER, width = 16)
        widget = self.createButton(frame, 'Recording', 'Add Keyframe',
                                   'Add Keyframe To Current Path',
                                   self.addKeyframe,
                                   side = LEFT, expand = 1)
        self.recordingType = StringVar()
        self.recordingType.set('New Curve')
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'New Curve',
            ('Next record session records a new path'),
            self.recordingType, 'New Curve',expand = 0)
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'Refine',
            ('Next record session refines existing path'),
            self.recordingType, 'Refine', expand = 0)
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'Extend',
            ('Next record session extends existing path'),
            self.recordingType, 'Extend', expand = 0)
        frame.pack(expand = 1, fill = X, pady = 3)
        
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
        self.pbNodePathMenu.selectitem('marker')
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
        
        widget = self.createEntryScale(
            playbackFrame, 'Playback', 'Time', 'Set current playback time',
            resolution = 0.01, command = self.playbackGoTo, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        # Kill playback task if drag slider
        widget.component('scale').bind(
            '<ButtonPress-1>', lambda e = None, s = self: s.stopPlayback())
        
        # Speed control
        frame = Frame(playbackFrame)
        Label(frame, text = 'PB Speed Vernier').pack(side = LEFT, expand = 0)
        self.speedScale = Scale(frame, from_ = -1, to = 1,
                                resolution = 0.01, showvalue = 0,
                                width = 10, orient = 'horizontal',
                                command = self.setPlaybackSF)
        self.speedScale.pack(fill = X, expand = 0)
        self.speedScale.bind('<ButtonRelease-1>',
                             lambda e = None, s = self: s.speedScale.set(0.0))
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
        self.createCheckbutton(frame, 'Playback', 'Loop',
                               'On: loop playback',
                               self.setLoopPlayback, self.loopPlayback,
                               side = LEFT, fill = BOTH, expand = 0)
        frame.pack(fill = X, expand = 1)

        playbackFrame.pack(fill = X, pady = 2)

        # Create notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        self.recordPage = self.mainNotebook.add('Record')
        self.resamplePage = self.mainNotebook.add('Resample')
        self.refinePage = self.mainNotebook.add('Refine')
        self.extendPage = self.mainNotebook.add('Extend')
        self.cropPage = self.mainNotebook.add('Crop')
        self.stylePage = self.mainNotebook.add('Style')

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
        self.recNodePathMenu.selectitem('marker')
        self.recNodePathMenuEntry = (
            self.recNodePathMenu.component('entryfield_entry'))
        self.recNodePathMenuBG = (
            self.recNodePathMenuEntry.configure('background')[3])
        self.bind(self.recNodePathMenu,
                  'Select node path to track when recording a new curve')
        self.recNodePathMenu.pack(side = LEFT, expand = 0)
        self.createButton(frame, 'Recording', 'Select',
                          'Select Current Record Node Path',
                          lambda s = self: direct.select(s['nodePath']))
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
        widget = self.createComboBox(
            frame, 'Recording', 'Pre-Record Func',
            'Function called before sampling each point',
            PRF_UTILITIES, self.setPreRecordFunc,
            history = 1, expand = 1)
        widget.configure(label_width = 16, label_anchor = W)
        widget.configure(entryfield_entry_state = 'normal')
        self.createCheckbutton(frame, 'Recording', 'PRF Active',
                               'On: Pre Record Func enabled',
                               None, 0,
                               side = LEFT, fill = BOTH, expand = 0)
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
            resolution = 1, min = 2, max = 1000, command = self.setNumSamples,
            side = LEFT)
        widget.component('hull')['relief'] = RIDGE
        widget.onRelease = widget.onReturnRelease = self.sampleCurve
        frame = Frame(resampleFrame)
        self.createCheckbutton(frame, 'Resample', 'Even',
                               'On: Resulting path has constant velocity',
                               self.setEven, self.fEven,
                               side = TOP, fill = BOTH, expand = 0)
        self.createCheckbutton(
            frame, 'Resample', 'Forward',
            'On: Resulting hpr curve faces along xyz tangent',
            self.setForward, self.fForward,
            side = TOP, fill = BOTH, expand = 0)
        frame.pack(fill = X, expand = 0)
        resampleFrame.pack(fill = X, expand = 0, pady = 2)
        # Desample
        desampleFrame = Frame(
            self.resamplePage, relief = SUNKEN, borderwidth = 2)
        Label(desampleFrame, text = 'DESAMPLE CURVE',
              font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createEntryScale(
            desampleFrame, 'Resample', 'Points Between Samples',
            'Specify number of points to skip between samples',
            min = 1, max = 100, resolution = 1,
            command = self.setDesampleFrequency)
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
                                       command = self.setRecordStart)
        widget.onPress = self.setRefineMode
        widget.onRelease = widget.onReturnRelease = (
            lambda s = self: s.getPrePoints('Refine'))
        widget = self.createEntryScale(
            refineFrame, 'Refine Page',
            'Control Start',
            'Time when full control of node path is given during refine pass',
            resolution = 0.01,
            command = self.setControlStart)
        widget.onPress = widget.onReturn = self.setRefineMode
        widget = self.createEntryScale(
            refineFrame, 'Refine Page',
            'Control Stop',
            'Time when node path begins transition back to original curve',
            resolution = 0.01,
            command = self.setControlStop)
        widget.onPress = widget.onReturn = self.setRefineMode
        widget = self.createEntryScale(refineFrame, 'Refine Page', 'Refine To',
                                       'Stop time of refine pass',
                                       resolution = 0.01,
                                       command = self.setRefineStop)
        widget.onPress = self.setRefineMode
        widget.onRelease = widget.onReturnRelease = self.getPostPoints
        refineFrame.pack(fill = X)

        ## EXTEND PAGE ##
        extendFrame = Frame(self.extendPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(extendFrame, text = 'EXTEND CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createEntryScale(extendFrame,
                                       'Extend Page', 'Extend From',
                                       'Begin time of extend pass',
                                       resolution = 0.01,
                                       command = self.setRecordStart)
        widget.onPress = self.setExtendMode
        widget.onRelease = widget.onReturnRelease = (
            lambda s = self: s.getPrePoints('Extend'))
        widget = self.createEntryScale(
            extendFrame, 'Extend Page',
            'Control Start',
            'Time when full control of node path is given during extend pass',
            resolution = 0.01,
            command = self.setControlStart)
        widget.onPress = widget.onReturn = self.setExtendMode
        extendFrame.pack(fill = X)

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

        ## STYLE PAGE ##
        styleFrame = Frame(self.stylePage, relief = SUNKEN,
                           borderwidth = 2)
        label = Label(styleFrame, text = 'CURVE RENDERINNG STYLE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        self.sf = Pmw.ScrolledFrame(styleFrame, horizflex = 'elastic')
        self.sf.pack(fill = 'both', expand = 1)
        sfFrame = self.sf.interior()
        frame = Frame(sfFrame)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show Path',
            'On: path is visible', self.setPathVis, 1,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show Knots',
            'On: path knots are visible', self.setKnotVis, 1,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show CVs',
            'On: path CVs are visible', self.setCvVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show Hull',
            'On: path hull is visible', self.setHullVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show Trace',
            'On: record is visible', self.setTraceVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Show Marker',
            'On: playback marker is visible', self.setMarkerVis, 0,
            side = LEFT, fill = X, expand = 1)
        frame.pack(fill = X, expand = 1)
        # Sliders
        widget = self.createEntryScale(
            sfFrame, 'Style', 'Num Segs',
            'Set number of segments used to approximate each parametric unit',
            min = 1.0, max = 400, resolution = 1.0,
            initialValue = 40, 
            command = self.setNumSegs, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        widget = self.createEntryScale(
            sfFrame, 'Style', 'Num Ticks',
            'Set number of tick marks drawn for each unit of time',
            min = 0.0, max = 10.0, resolution = 1.0,
            initialValue = 0.0,
            command = self.setNumTicks, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        widget = self.createEntryScale(
            sfFrame, 'Style', 'Tick Scale',
            'Set visible size of time tick marks',
            min = 0.01, max = 100.0, resolution = 0.01,
            initialValue = 1.0,
            command = self.setTickScale, side = TOP)
        widget.component('hull')['relief'] = RIDGE
	self.createColorEntry(
            sfFrame, 'Style', 'Path Color',
            'Color of curve',
            command = self.setPathColor,
            initialValue = [255.0,255.0,255.0,255.0])
	self.createColorEntry(
            sfFrame, 'Style', 'Knot Color',
            'Color of knots',
            command = self.setKnotColor,
            initialValue = [0,0,255.0,255.0])
	self.createColorEntry(
            sfFrame, 'Style', 'CV Color',
            'Color of CVs',
            command = self.setCvColor,
            initialValue = [255.0,0,0,255.0])
	self.createColorEntry(
            sfFrame, 'Style', 'Tick Color',
            'Color of Ticks',
            command = self.setTickColor,
            initialValue = [255.0,0,0,255.0])
	self.createColorEntry(
            sfFrame, 'Style', 'Hull Color',
            'Color of Hull',
            command = self.setHullColor,
            initialValue = [255.0,128.0,128.0,255.0])

        styleFrame.pack(fill = X)

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
        
    def selectedNodePathHook(self, nodePath):
        """
        Hook called upon selection of a node path used to select playback
        marker if subnode selected
        """
        taskMgr.removeTasksNamed(self.name + '-curveEditTask')
        if nodePath.id() == self.playbackMarker.getChild(0).id():
            direct.select(self.playbackMarker)
        elif nodePath.id() == self.tangentMarker.getChild(0).id():
            direct.select(self.tangentMarker)
        elif nodePath.id() == self.playbackMarker.id():
            self.tangentGroup.show()
            taskMgr.spawnMethodNamed(self.curveEditTask,
                                     self.name + '-curveEditTask')
        elif nodePath.id() == self.tangentMarker.id():
            self.tangentGroup.show()
            taskMgr.spawnMethodNamed(self.curveEditTask,
                                     self.name + '-curveEditTask')
        else:
            self.tangentGroup.hide()

    def deselectedNodePathHook(self, nodePath):
        """
        Hook called upon deselection of a node path used to select playback
        marker if subnode selected
        """
        if ((nodePath.id() == self.playbackMarker.id()) or
            (nodePath.id() == self.tangentMarker.id())):
            self.tangentGroup.hide()

    def curveEditTask(self,state):
        if self.xyzNurbsCurve != None:
            # Update curve position
            if self.manipulandumId == self.playbackMarker.id():
                # Show playback marker
                self.playbackMarker.getChild(0).show()
                pos = Point3(0)
                pos = self.playbackMarker.getPos(self.nodePathParent)
                self.xyzNurbsCurve.adjustPoint(
                    self.playbackTime,
                    pos[0], pos[1], pos[2])
                hpr = Point3(0)
                hpr = self.playbackMarker.getHpr(self.nodePathParent)
                self.hprNurbsCurve.adjustPoint(
                    self.playbackTime,
                    hpr[0], hpr[1], hpr[2])
                self.hprNurbsCurve.recompute()
                self.xyzNurbsCurveDrawer.draw()
            # Update tangent
            if self.manipulandumId == self.tangentMarker.id():
                # If manipulating marker, update tangent
                # Hide playback marker
                self.playbackMarker.getChild(0).hide()
                # Where is tangent marker relative to playback marker
                tan = self.tangentMarker.getPos()
                # Transform this vector to curve space
                tan2Curve = Vec3(
                    self.playbackMarker.getMat(
                    self.nodePathParent).xformVec(tan))
                # Update nurbs curve
                self.xyzNurbsCurve.adjustTangent(
                    self.playbackTime,
                    tan2Curve[0], tan2Curve[1], tan2Curve[2])
                self.xyzNurbsCurveDrawer.draw()
            else:
                # Show playback marker
                self.playbackMarker.getChild(0).show()
                # Update tangent marker line
                tan = Point3(0)
                self.xyzNurbsCurve.getTangent(self.playbackTime, tan)
                # Transform this point to playback marker space
                tan.assign(
                    self.nodePathParent.getMat(
                    self.playbackMarker).xformVec(tan))
                self.tangentMarker.setPos(tan)
            # In either case update tangent line
            self.tangentLines.setVertex(1, tan[0], tan[1], tan[2])
        return Task.cont

    def manipulateObjectStartHook(self):
        self.manipulandumId = None
        if direct.selected.last:
            if direct.selected.last.id() == self.playbackMarker.id():
                self.manipulandumId = self.playbackMarker.id()
            elif direct.selected.last.id() == self.tangentMarker.id():
                self.manipulandumId = self.tangentMarker.id()
              
    def manipulateObjectCleanupHook(self):
        # Clear flag
        self.manipulandumId = None
            
    def onDestroy(self, event):
        # Remove hooks
        for event, method in self.actionEvents:
            self.ignore(event)
        # remove start stop hook
        self.ignore(self.startStopHook)
        self.ignore(self.keyframeHook)
        self.curveNodePath.reparentTo(self.recorderNodePath)
        self.trace.reparentTo(self.recorderNodePath)
        self.recorderNodePath.removeNode()
        # Make sure markers are deselected
        direct.deselect(self.playbackMarker)
        direct.deselect(self.tangentMarker)
        # Remove tasks
        taskMgr.removeTasksNamed(self.name + '-recordTask')
        taskMgr.removeTasksNamed(self.name + '-playbackTask')
        taskMgr.removeTasksNamed(self.name + '-curveEditTask')

    def savePointSet(self):
        # Use curve to compute default point set at 30 fps
        # Keep a handle on the original curve
        xyzCurve = self.xyzNurbsCurve
        hprCurve = self.hprNurbsCurve
        # Sample curves
        self.maxT = self.xyzNurbsCurve.getMaxT()
        self.setNumSamples(self.maxT * 30.0)
        self.sampleCurve()
        # Restore curves to those loaded in
        self.xyzNurbsCurve = xyzCurve
        self.hprNurbsCurve = hprCurve
        # Redraw
        self.xyzNurbsCurveDrawer.setCurve(self.xyzNurbsCurve)
        self.xyzNurbsCurveDrawer.draw()

    def createNewPointSet(self):
        self.pointSetName = self.name + '-ps-' + `self.pointSetCount`
        # Update dictionary and record pointer to new point set
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
        # Compute curve
        self.computeCurves()

    def setPathVis(self):
        if self.getVariable('Style', 'Show Path').get():
            self.curveNodePath.show()
        else:
            self.curveNodePath.hide()
        
    def setKnotVis(self):
        self.xyzNurbsCurveDrawer.setShowKnots(
            self.getVariable('Style', 'Show Knots').get())

    def setCvVis(self):
        self.xyzNurbsCurveDrawer.setShowCvs(
            self.getVariable('Style', 'Show CVs').get())
        
    def setHullVis(self):
        self.xyzNurbsCurveDrawer.setShowHull(
            self.getVariable('Style', 'Show Hull').get())
        
    def setTraceVis(self):
        if self.getVariable('Style', 'Show Trace').get():
            self.trace.show()
        else:
            self.trace.hide()

    def setMarkerVis(self):
        if self.getVariable('Style', 'Show Marker').get():
            self.playbackMarker.reparentTo(self.recorderNodePath)
        else:
            self.playbackMarker.reparentTo(hidden)

    def setNumSegs(self, value):
        self.numSegs = int(value)
        self.xyzNurbsCurveDrawer.setNumSegs(self.numSegs)
        
    def setNumTicks(self, value):
        self.xyzNurbsCurveDrawer.setNumTicks(float(value))
        
    def setTickScale(self, value):
        self.xyzNurbsCurveDrawer.setTickScale(float(value))

    def setPathColor(self, color):
        self.xyzNurbsCurveDrawer.setColor(
            color[0]/255.0,color[1]/255.0,color[2]/255.0)
        self.xyzNurbsCurveDrawer.draw()

    def setKnotColor(self, color):
        self.xyzNurbsCurveDrawer.setKnotColor(
            color[0]/255.0,color[1]/255.0,color[2]/255.0)

    def setCvColor(self, color):
        self.xyzNurbsCurveDrawer.setCvColor(
            color[0]/255.0,color[1]/255.0,color[2]/255.0)

    def setTickColor(self, color):
        self.xyzNurbsCurveDrawer.setTickColor(
            color[0]/255.0,color[1]/255.0,color[2]/255.0)

    def setHullColor(self, color):
        self.xyzNurbsCurveDrawer.setHullColor(
            color[0]/255.0,color[1]/255.0,color[2]/255.0)

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
        self.ignore(self.keyframeHook)
        # Record new one
        hook = self.getVariable('Recording', 'Keyframe Hook').get()
        self.keyframeHook = hook
        # Add new one
        self.accept(self.keyframeHook, self.addKeyframe)

    def reset(self):
        self.pointSet = []
        self.hasPoints = 0
        self.xyzNurbsCurve = None
        self.hprNurbsCurve = None
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        self.xyzNurbsCurveDrawer.hide()
        
    def setSamplingMode(self, mode):
        self.samplingMode = mode

    def disableKeyframeButton(self):
        self.getWidget('Recording', 'Add Keyframe')['state'] = 'disabled'
    def enableKeyframeButton(self):
        self.getWidget('Recording', 'Add Keyframe')['state'] = 'normal'

    def setRecordingType(self, type):
        self.recordingType.set(type)

    def setNewCurveMode(self):
        self.setRecordingType('New Curve')

    def setRefineMode(self):
        self.setRecordingType('Refine')
        
    def setExtendMode(self):
        self.setRecordingType('Extend')

    def toggleRecordVar(self):
        # Get recording variable
        v = self.getVariable('Recording', 'Record')
        # Toggle it
        v.set(1 - v.get())
        # Call the command
        self.toggleRecord()

    def toggleRecord(self):
        if self.getVariable('Recording', 'Record').get():
            # Kill old tasks
            taskMgr.removeTasksNamed(self.name + '-recordTask')
            taskMgr.removeTasksNamed(self.name + '-curveEditTask')
            # Remove old curve
            self.xyzNurbsCurveDrawer.hide()
            # Reset curve fitters
            self.xyzCurveFitter.reset()
            self.hprCurveFitter.reset()
            # Update sampling mode button if necessary
            if self.samplingMode == 'Continuous':
                self.disableKeyframeButton()
            # Create a new point set to hold raw data
            self.createNewPointSet()
            # Record nopath's parent
            self.nodePathParent = self['nodePath'].getParent()
            # Put curve drawer under record node path's parent
            self.curveNodePath.reparentTo(self.nodePathParent)
            # Clear out old trace, get ready to draw new
            self.initTrace()
            # Keyframe mode?
            if (self.samplingMode == 'Keyframe'):
                # Record first point
                self.lastPos.assign(Point3(
                    self['nodePath'].getPos(self.nodePathParent)))
                # Init delta time
                self.deltaTime = 0.0
                # Record first point
                self.recordPoint(self.recordStart)
            # Everything else
            else:
                if ((self.recordingType.get() == 'Refine') or
                    (self.recordingType.get() == 'Extend')):
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
                    # Set playback start to self.recordStart
                    self.playbackGoTo(self.recordStart)
                    # start flying nodePath along path
                    self.startPlayback()
                # Start new task
                t = taskMgr.spawnMethodNamed(
                    self.recordTask, self.name + '-recordTask')
                t.startTime = globalClock.getFrameTime()
        else:
            if self.samplingMode == 'Continuous':
                # Kill old task
                taskMgr.removeTasksNamed(self.name + '-recordTask')
                if ((self.recordingType.get() == 'Refine') or
                    (self.recordingType.get() == 'Extend')):
                    # Reparent node path back to parent
                    self['nodePath'].wrtReparentTo(self.nodePathParent)
                    # Restore playback Node Path
                    self.playbackNodePath = self.lastPlaybackNodePath
                    # See if it was the playback marker
                    if self.playbackNodePath.id() == self.playbackMarker.id():
                        self.playbackMarker.show()
            else:
                # Add last point
                self.addKeyframe(0)
            # Reset sampling mode
            self.setSamplingMode('Continuous')
            self.enableKeyframeButton()
            # Clean up after refine or extend
            if ((self.recordingType.get() == 'Refine') or
                (self.recordingType.get() == 'Extend')):
                # Merge prePoints, pointSet, postPoints
                self.mergePoints()
                # Clear out pre and post list
                self.prePoints = []
                self.postPoints = []
                # Reset recording mode
                self.setNewCurveMode()
            # Compute curve
            self.computeCurves()
            
    def recordTask(self, state):
        # Record raw data point
        time = self.recordStart + (globalClock.getFrameTime() - state.startTime)
        self.recordPoint(time)
        return Task.cont

    def addKeyframe(self, fToggleRecord = 1):
        # Make sure we're in a recording mode!
        if (fToggleRecord and
            (not self.getVariable('Recording', 'Record').get())):
            # Set sampling mode
            self.setSamplingMode('Keyframe')
            # This will automatically add the first point
            self.toggleRecordVar()
        else:
            # Use distance as a time
            pos = self['nodePath'].getPos(self.nodePathParent)
            deltaPos = Vec3(pos - self.lastPos).length()
            if deltaPos != 0:
                # If we've moved at all, use delta Pos as time
                self.deltaTime = self.deltaTime + deltaPos
            else:
                # Otherwise add one second
                self.deltaTime = self.deltaTime + 1.0
            # Record point at new time
            self.recordPoint(self.recordStart + self.deltaTime)
            # Update last pos
            self.lastPos.assign(pos)

    def easeInOut(self, t):
        x = t * t
        return (3 * x) - (2 * t * x)

    def setPreRecordFunc(self, func):
        # Note: If func is one defined at command prompt, need to set
        # __builtins__.func = func at command line
        self.preRecordFunc = eval(func)
        # Update widget to reflect new value
        self.getVariable('Recording', 'PRF Active').set(1)

    def recordPoint(self, time):
        # Call user define callback before recording point
        if (self.getVariable('Recording', 'PRF Active').get() and
            (self.preRecordFunc != None)):
            self.preRecordFunc()
        # Get point
        pos = self['nodePath'].getPos(self.nodePathParent)
        hpr = self['nodePath'].getHpr(self.nodePathParent)
        # Blend between recordNodePath and self['nodePath']
        if ((self.recordingType.get() == 'Refine') or
            (self.recordingType.get() == 'Extend')):
            if ((time < self.controlStart) and
                ((self.controlStart - self.recordStart) != 0.0)):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                t = self.easeInOut(((time - self.recordStart)/
                                    (self.controlStart - self.recordStart)))
                # Transition between the recorded node path and the driven one
                pos = (rPos * (1 - t)) + (pos * t)
                hpr = (rHpr * (1 - t)) + (hpr * t)
            elif ((self.recordingType.get() == 'Refine') and
                  (time > self.controlStop) and
                  ((self.recordStop - self.controlStop) != 0.0)):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                t = self.easeInOut(((time - self.controlStop)/
                                    (self.recordStop - self.controlStop)))
                # Transition between the recorded node path and the driven one
                pos = (pos * (1 - t)) + (rPos * t)
                hpr = (hpr * (1 - t)) + (rHpr * t)
        # Add it to the point set
        self.pointSet.append([time, pos, hpr])
        # Add it to the curve fitters
        self.xyzCurveFitter.addPoint(time, pos )
        self.hprCurveFitter.addPoint(time, hpr)
        # Update trace now if recording keyframes
        if (self.samplingMode == 'Keyframe'):
            self.trace.reset()
            for t, p, h in self.pointSet:
                self.trace.drawTo(p[0], p[1], p[2])
            self.trace.create()

    def computeCurves(self):
        # Check to make sure curve fitters have points
        if ((self.xyzCurveFitter.getNumSamples() == 0) or
            (self.hprCurveFitter.getNumSamples() == 0)):
            print 'MopathRecorder.computeCurves: Must define curve first'
            return
        # Create curves
        # XYZ
        self.xyzCurveFitter.sortPoints()
        self.xyzCurveFitter.computeTangents(1)
        self.xyzNurbsCurve = self.xyzCurveFitter.makeNurbs()
        self.xyzNurbsCurve.setCurveType(PCTXYZ)
        self.xyzNurbsCurveDrawer.setCurve(self.xyzNurbsCurve)
        self.xyzNurbsCurveDrawer.draw()
        # HPR
        self.hprCurveFitter.sortPoints()
        self.hprCurveFitter.wrapHpr()
        self.hprCurveFitter.computeTangents(1)
        self.hprNurbsCurve = self.hprCurveFitter.makeNurbs()
        self.hprNurbsCurve.setCurveType(PCTHPR)
        # Update widget based on new curve
        self.updateWidgets()

    def initTrace(self):
        self.trace.reset()
        # Put trace line segs under node path's parent
        self.trace.reparentTo(self.nodePathParent)
        # Show it
        self.trace.show()

    def updateWidgets(self):
        if not self.xyzNurbsCurve:
            return
        self.fAdjustingValues = 1
        # Widgets depending on max T
        maxT = '%.2f' % self.xyzNurbsCurve.getMaxT()
        # Playback controls
        self.getWidget('Playback', 'Time').configure(max = maxT)
        self.getVariable('Resample', 'Path Duration').set(maxT)
        # Refine widgets
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
        # Extend widgets
        widget = self.getWidget('Extend Page', 'Extend From')
        widget.configure(max = maxT)
        widget.set(float(0.0))
        widget = self.getWidget('Extend Page', 'Control Start')
        widget.configure(max = maxT)
        widget.set(float(0.0))
        # Crop widgets
        widget = self.getWidget('Crop Page', 'Crop From')
        widget.configure(max = maxT)
        widget.set(float(0.0))
        widget = self.getWidget('Crop Page', 'Crop To')
        widget.configure(max = maxT)
        widget.set(float(maxT))
        self.maxT = float(maxT)
        # Widgets depending on number of samples
        numSamples = self.xyzCurveFitter.getNumSamples()
        widget = self.getWidget('Resample', 'Points Between Samples')
        widget.configure(max=numSamples)
        widget = self.getWidget('Resample', 'Num. Samples')
        widget.configure(max = 4 * numSamples)
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
                if name == 'marker':
                    self.playbackMarker.show()
                else:
                    self.playbackMarker.hide()
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
                if name == 'marker':
                    self.playbackMarker.show()
                    # Initialize tangent marker position
                    tan = Point3(0)
                    if self.xyzNurbsCurve != None:
                        self.xyzNurbsCurve.getTangent(self.playbackTime, tan)
                    self.tangentMarker.setPos(tan)
                else:
                    self.playbackMarker.hide()
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
        if (self.xyzNurbsCurve == None) and (self.hprNurbsCurve == None):
            return
        self.playbackTime = CLAMP(time, 0.0, self.maxT)
        if self.xyzNurbsCurve != None:
            pos = Point3(0)
            self.xyzNurbsCurve.getPoint(self.playbackTime, pos)
            self.playbackNodePath.setPos(self.nodePathParent, pos)
        if self.hprNurbsCurve != None:
            hpr = Point3(0)
            self.hprNurbsCurve.getPoint(self.playbackTime, hpr)
            self.playbackNodePath.setHpr(self.nodePathParent, hpr)

    def startPlayback(self):
        if (self.xyzNurbsCurve == None) and (self.hprNurbsCurve == None):
            return
        # Kill any existing tasks
        self.stopPlayback()
        # Make sure checkbutton is set
        self.getVariable('Playback', 'Play').set(1)
        # Start new playback task
        t = taskMgr.spawnMethodNamed(
            self.playbackTask, self.name + '-playbackTask')
        t.currentTime = self.playbackTime
        t.lastTime = globalClock.getFrameTime()

    def setSpeedScale(self, value):
        self.speedScale.set(value)

    def setPlaybackSF(self, value):
        self.playbackSF = pow(10.0, float(value))
        
    def playbackTask(self, state):
        time = globalClock.getFrameTime()
        dTime = self.playbackSF * (time - state.lastTime)
        state.lastTime = time
        if self.loopPlayback:
            cTime = (state.currentTime + dTime) % self.maxT
        else:
            cTime = state.currentTime + dTime
        # Stop task if not looping and at end of curve
        # Or if refining curve and past recordStop
        if ((self.recordingType.get() == 'Refine') and
              (cTime > self.recordStop)):
            # Go to recordStop
            self.getWidget('Playback', 'Time').set(self.recordStop)
            # Then stop playback
            self.stopPlayback()
            # Also kill record task
            self.toggleRecordVar()
            return Task.done
        elif (((self.loopPlayback == 0) and (cTime > self.maxT)) or
            ((self.recordingType.get() == 'Extend') and (cTime > self.maxT))):
            # Go to maxT
            self.getWidget('Playback', 'Time').set(self.maxT)
            # Then stop playback
            self.stopPlayback()
            return Task.done
        # Otherwise go to specified time and continue
        self.getWidget('Playback', 'Time').set(cTime)
        state.currentTime = cTime
        return Task.cont

    def stopPlayback(self):
        self.getVariable('Playback', 'Play').set(0)
        taskMgr.removeTasksNamed(self.name + '-playbackTask')

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
        if ((self.xyzCurveFitter.getNumSamples() == 0) or
            (self.hprCurveFitter.getNumSamples() == 0)):
            print 'MopathRecorder.desampleCurve: Must define curve first'
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
        
    def sampleCurve(self, even = 'None Given'):
        if (self.xyzNurbsCurve == None) and (self.hprNurbsCurve == None):
            print 'MopathRecorder.sampleCurve: Must define curve first'
            return
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Get new data points based on given curve
        if even == 'None Given':
            even = self.fEven
        self.xyzCurveFitter.sample(
            self.xyzNurbsCurve, self.numSamples, even)
        if self.fForward:
            # Use xyz curve tangent
            tanPt = Point3(0)
            lookAtCS = self.playbackMarker.attachNewNode('lookAt')
        # Now sample the hprNurbsCurve using the same delta T
        for i in range(self.numSamples):
            t = self.maxT * (i / float(self.numSamples - 1))
            hpr = Point3(0)
            if self.fForward:
                # Use xyz curve tangent
                self.xyzNurbsCurve.getTangent(t, tanPt)
                # Transform this point to playback marker space
                tanPt.assign(
                    self.nodePathParent.getMat(
                    self.playbackMarker).xformVec(tanPt))
                lookAtCS.lookAt(tanPt, Z_AXIS)
                hpr = lookAtCS.getHpr(self.nodePathParent)
            else:
                # Sample existing hpr curve
                self.hprNurbsCurve.getPoint(t, hpr)
            self.hprCurveFitter.addPoint(t, hpr)
        # Clean up
        if self.fForward:
            lookAtCS.removeNode()
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

    def setForward(self):
        self.fForward = self.getVariable('Resample', 'Forward').get()

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
        # Scale point set
        # Save handle to old point set
        oldPointSet = self.pointSet
        # Create new point set
        self.createNewPointSet()
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Now scale values
        for time, pos, hpr in oldPointSet:
            newTime = time * sf
            # Update point set
            self.pointSet.append([newTime, Point3(pos), Point3(hpr)])
            # Add it to the curve fitters
            self.xyzCurveFitter.addPoint(newTime, pos )
            self.hprCurveFitter.addPoint(newTime, hpr)
        # Compute curve
        self.computeCurves()

    def setRecordStart(self,value):
        self.recordStart = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        # Adjust refine widgets
        # Make sure we're in sync
        self.getWidget('Refine Page', 'Refine From').set(
            self.recordStart)
        self.getWidget('Extend Page', 'Extend From').set(
            self.recordStart)
        # Check bounds
        if self.recordStart > self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.recordStart)
            self.getWidget('Extend Page', 'Control Start').set(
                self.recordStart)
        if self.recordStart > self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.recordStart)
        if self.recordStart > self.recordStop:
            self.getWidget('Refine Page', 'Refine To').set(self.recordStart)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def getPrePoints(self, type = 'Refine'):
        # Switch to appropriate recording type
        self.setRecordingType(type)
        # Reset prePoints
        self.prePoints = []
        # See if we need to save any points before recordStart
        for i in range(len(self.pointSet)):
            # Have we passed recordStart?
            if self.recordStart < self.pointSet[i][0]:
                # Get a copy of the points prior to recordStart
                self.prePoints = self.pointSet[:i-1]
                break

    def setControlStart(self, value):
        self.controlStart = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        # Adjust refine widgets
        # Make sure both pages are in sync
        self.getWidget('Refine Page', 'Control Start').set(
            self.controlStart)
        self.getWidget('Extend Page', 'Control Start').set(
            self.controlStart)
        # Check bounds on other widgets
        if self.controlStart < self.recordStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.controlStart)
            self.getWidget('Extend Page', 'Extend From').set(
                self.controlStart)
        if self.controlStart > self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.controlStart)
        if self.controlStart > self.recordStop:
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
        if self.controlStop < self.recordStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.controlStop)
        if self.controlStop < self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.controlStop)
        if self.controlStop > self.recordStop:
            self.getWidget('Refine Page', 'Refine To').set(
                self.controlStop)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def setRefineStop(self, value):
        self.recordStop = value
        # Someone else is adjusting values, let them take care of it
        if self.fAdjustingValues:
            return
        self.fAdjustingValues = 1
        if self.recordStop < self.recordStart:
            self.getWidget('Refine Page', 'Refine From').set(
                self.recordStop)
        if self.recordStop < self.controlStart:
            self.getWidget('Refine Page', 'Control Start').set(
                self.recordStop)
        if self.recordStop < self.controlStop:
            self.getWidget('Refine Page', 'Control Stop').set(
                self.recordStop)
        # Move playback node path to specified time
        self.getWidget('Playback', 'Time').set(value)
        self.fAdjustingValues = 0

    def getPostPoints(self):
        # Set flag so we know to do a refine pass
        self.setRefineMode()
        # Reset postPoints
        self.postPoints = []
        # See if we need to save any points after recordStop
        for i in range(len(self.pointSet)):
            # Have we reached recordStop?
            if self.recordStop < self.pointSet[i][0]:
                # Get a copy of the points after recordStop
                self.postPoints = self.pointSet[i:]
                break

    def mergePoints(self):
        # prepend pre points
        self.pointSet[0:0] = self.prePoints
        for time, pos, hpr in self.prePoints:
            # Add it to the curve fitters
            self.xyzCurveFitter.addPoint(time, pos )
            self.hprCurveFitter.addPoint(time, hpr)
        # And post points
        # What is end time of pointSet?
        endTime = self.pointSet[-1][0]
        for time, pos, hpr in self.postPoints:
            adjustedTime = endTime + (time - self.recordStop)
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
        if self.pointSet == None:
            print 'Empty Point Set'
            return
        # Keep handle on old points
        oldPoints = self.pointSet
        # Create new point set
        self.createNewPointSet()
        # Copy over points between from/to
        # Reset curve fitters
        self.xyzCurveFitter.reset()
        self.hprCurveFitter.reset()
        # Add start point
        pos = Point3(0)
        self.xyzNurbsCurve.getPoint(self.cropFrom, pos)
        self.xyzCurveFitter.addPoint(0.0, pos)
        hpr = Point3(0)
        self.hprNurbsCurve.getPoint(self.cropFrom, hpr)
        self.hprCurveFitter.addPoint(0.0, hpr)
        # Get points within bounds
        for time, pos, hpr in oldPoints:
            # Is it within the time?
            if ((time > self.cropFrom) and
                (time < self.cropTo)):
                # Add it to the curve fitters
                t = time - self.cropFrom
                self.xyzCurveFitter.addPoint(t, pos )
                self.hprCurveFitter.addPoint(t, hpr)
                # And the point set
                self.pointSet.append([t, pos, hpr])
        # Add last point
        pos = Vec3(0)
        self.xyzNurbsCurve.getPoint(self.cropTo, pos)
        self.xyzCurveFitter.addPoint(self.cropTo - self.cropFrom, pos)
        hpr = Vec3(0)
        self.hprNurbsCurve.getPoint(self.cropTo, hpr)
        self.hprCurveFitter.addPoint(self.cropTo - self.cropFrom, hpr)
        # Compute curve
        self.computeCurves()

    def loadCurveFromFile(self):
        # Use first directory in model path
        mPath = getModelPath()
        if mPath.getNumDirectories() > 0:
            if `mPath.getDirectory(0)` == '.':
                path = '.'
            else:
                path = mPath.getDirectory(0).toOsSpecific()
        else:
            path = '.'
        if not os.path.isdir(path):
            print 'MopathRecorder Info: Empty Model Path!'
            print 'Using current directory'
            path = '.'
        mopathFilename = askopenfilename(
            defaultextension = '.egg',
            filetypes = (('Egg Files', '*.egg'),
                         ('Bam Files', '*.bam'),
                         ('All files', '*')),
            initialdir = path,
            title = 'Load Nurbs Curve',
            parent = self.parent)
        if mopathFilename:
            self.reset()
            nodePath = loader.loadModel(mopathFilename)
            if nodePath:
                self.extractCurves(nodePath)
                if ((self.xyzNurbsCurve != None) and 
                    (self.hprNurbsCurve != None)):
                    # Save a pointset for this curve
                    self.savePointSet()
                else:
                    if (self.xyzNurbsCurve != None):
                        print 'Mopath Recorder: HPR Curve not found'
                    elif (self.hprNurbsCurve != None):
                        print 'Mopath Recorder: XYZ Curve not found'
                    else:
                        print 'Mopath Recorder: No Curves found'
                    self.reset()

    def extractCurves(self, nodePath):
        node = nodePath.node()
        if isinstance(node, ParametricCurve):
            if node.getCurveType() == PCTXYZ:
                self.xyzNurbsCurve = node
            elif node.getCurveType() == PCTHPR:
                self.hprNurbsCurve = node
        else:
            # Iterate over children if any
            for child in nodePath.getChildrenAsList():
                self.extractCurves(child)
            
    def saveCurveToFile(self):
        # Use first directory in model path
        mPath = getModelPath()
        if mPath.getNumDirectories() > 0:
            if `mPath.getDirectory(0)` == '.':
                path = '.'
            else:
                path = mPath.getDirectory(0).toOsSpecific()
        else:
            path = '.'
        if not os.path.isdir(path):
            print 'MopathRecorder Info: Empty Model Path!'
            print 'Using current directory'
            path = '.'
        mopathFilename = asksaveasfilename(
            defaultextension = '.egg',
            filetypes = (('Egg Files', '*.egg'),
                         ('Bam Files', '*.bam'),
                         ('All files', '*')),
            initialdir = path,
            title = 'Save Nurbs Curve as',
            parent = self.parent)
        if mopathFilename:
            self.xyzNurbsCurve.writeEgg(mopathFilename)
            self.hprNurbsCurve.writeEgg(mopathFilename)

    def followTerrain(self, height = 1.0):
        self.iRay.rayCollisionNodePath.reparentTo(self['nodePath'])
        node, hitPt, hitPtDist = self.iRay.pickGeom3D()
        if node:
            self['nodePath'].setZ(self['nodePath'], height - hitPtDist)
        self.iRay.rayCollisionNodePath.reparentTo(self.recorderNodePath)


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
        self.bind(label, balloonHelp)
        self.widgetDict[category + '-' + text + '-Label'] = label
        entry = Entry(frame, width = width, relief = relief,
                      textvariable = variable)
        entry.pack(side = LEFT, fill = X, expand = expand)
        self.bind(entry, balloonHelp)
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
                          command = None, fill = X, expand = 0):
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
                       items, command, history = 0,
                       side = LEFT, expand = 0, fill = X):
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
        widget.pack(side = side, fill = fill, expand = expand)
        # Bind help
        self.bind(widget, balloonHelp)
        # Record widget
        self.widgetDict[category + '-' + text] = widget
        return widget

