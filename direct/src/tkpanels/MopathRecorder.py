""" Mopath Recorder Panel Module """

__all__ = ['MopathRecorder']

# Import Tkinter, Pmw, and the dial code from this directory tree.
from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.showbase.TkGlobal import *
from direct.tkwidgets.AppShell import *
from direct.directtools.DirectGlobals import *
from direct.directtools.DirectUtil import *
from direct.directtools.DirectGeometry import *
from direct.directtools.DirectSelection import *
from tkFileDialog import *
from Tkinter import *
import Pmw, os, string
from direct.tkwidgets import Dial
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
from direct.tkwidgets import EntryScale
from direct.tkwidgets import VectorWidgets
import __builtin__

PRF_UTILITIES = [
    'lambda: base.direct.camera.lookAt(render)',
    'lambda: base.direct.camera.setZ(render, 0.0)',
    'lambda s = self: s.playbackMarker.lookAt(render)',
    'lambda s = self: s.playbackMarker.setZ(render, 0.0)',
    'lambda s = self: s.followTerrain(10.0)']

class MopathRecorder(AppShell, DirectObject):
    # Override class variables here
    appname = 'Mopath Recorder Panel'
    frameWidth      = 450
    frameHeight     = 550
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

        self.selectNodePathNamed('camera')

    def appInit(self):
        self.name = self['name']
        # Dictionary of widgets
        self.widgetDict = {}
        self.variableDict = {}
        # Initialize state
        # The active node path
        self.nodePath = self['nodePath']
        self.playbackNodePath = self.nodePath
        # The active node path's parent
        self.nodePathParent = render
        # Top level node path
        self.recorderNodePath = base.direct.group.attachNewNode(self.name)
        # Temp CS for use in refinement/path extension
        self.tempCS = self.recorderNodePath.attachNewNode(
            'mopathRecorderTempCS')
        # Marker for use in playback
        self.playbackMarker = loader.loadModel('models/misc/smiley')
        self.playbackMarker.setName('Playback Marker')
        self.playbackMarker.reparentTo(self.recorderNodePath)
        self.playbackMarkerIds = self.getChildIds(
            self.playbackMarker.getChild(0))
        self.playbackMarker.hide()
        # Tangent marker
        self.tangentGroup = self.playbackMarker.attachNewNode('Tangent Group')
        self.tangentGroup.hide()
        self.tangentMarker = loader.loadModel('models/misc/sphere')
        self.tangentMarker.reparentTo(self.tangentGroup)
        self.tangentMarker.setScale(0.5)
        self.tangentMarker.setColor(1, 0, 1, 1)
        self.tangentMarker.setName('Tangent Marker')
        self.tangentMarkerIds = self.getChildIds(
            self.tangentMarker.getChild(0))
        self.tangentLines = LineNodePath(self.tangentGroup)
        self.tangentLines.setColor(VBase4(1, 0, 1, 1))
        self.tangentLines.setThickness(1)
        self.tangentLines.moveTo(0, 0, 0)
        self.tangentLines.drawTo(0, 0, 0)
        self.tangentLines.create()
        # Active node path dictionary
        self.nodePathDict = {}
        self.nodePathDict['marker'] = self.playbackMarker
        self.nodePathDict['camera'] = base.direct.camera
        self.nodePathDict['widget'] = base.direct.widget
        self.nodePathDict['mopathRecorderTempCS'] = self.tempCS
        self.nodePathNames = ['marker', 'camera', 'selected']
        # ID of selected object
        self.manipulandumId = None
        self.trace = LineNodePath(self.recorderNodePath)
        self.oldPlaybackNodePath = None
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
        self.curveFitter = CurveFitter()
        # Curve variables
        # Number of ticks per parametric unit
        self.numTicks = 1
        # Number of segments to represent each parametric unit
        # This just affects the visual appearance of the curve
        self.numSegs = 40
        # The nurbs curves
        self.curveCollection = None
        # Curve drawers
        self.nurbsCurveDrawer = NurbsCurveDrawer()
        self.nurbsCurveDrawer.setCurves(ParametricCurveCollection())
        self.nurbsCurveDrawer.setNumSegs(self.numSegs)
        self.nurbsCurveDrawer.setShowHull(0)
        self.nurbsCurveDrawer.setShowCvs(0)
        self.nurbsCurveDrawer.setNumTicks(0)
        self.nurbsCurveDrawer.setTickScale(5.0)
        self.curveNodePath = self.recorderNodePath.attachNewNode(
            self.nurbsCurveDrawer.getGeomNode())
        useDirectRenderStyle(self.curveNodePath)
        # Playback variables
        self.maxT = 0.0
        self.playbackTime = 0.0
        self.loopPlayback = 1
        self.playbackSF = 1.0
        # Sample variables
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
            ('DIRECT_undo', self.undoHook),
            ('DIRECT_pushUndo', self.pushUndoHook),
            ('DIRECT_undoListEmpty', self.undoListEmptyHook),
            ('DIRECT_redo', self.redoHook),
            ('DIRECT_pushRedo', self.pushRedoHook),
            ('DIRECT_redoListEmpty', self.redoListEmptyHook),
            ('DIRECT_selectedNodePath', self.selectedNodePathHook),
            ('DIRECT_deselectedNodePath', self.deselectedNodePathHook),
            ('DIRECT_manipulateObjectStart', self.manipulateObjectStartHook),
            ('DIRECT_manipulateObjectCleanup',
             self.manipulateObjectCleanupHook),
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
            command = self.extractPointSetFromCurveCollection)
        self.menuBar.addmenuitem(
            'Recorder', 'command',
            'Toggle widget visability',
            label = 'Toggle Widget Vis',
            command = base.direct.toggleWidgetVis)
        self.menuBar.addmenuitem(
            'Recorder', 'command',
            'Toggle widget manipulation mode',
            label = 'Toggle Widget Mode',
            command = base.direct.manipulationControl.toggleObjectHandlesMode)

        self.createComboBox(self.menuFrame, 'Mopath', 'History',
                            'Select input points to fit curve to', '',
                            self.selectPointSetNamed, expand = 1)

        self.undoButton = Button(self.menuFrame, text = 'Undo',
                                 command = base.direct.undo)
        if base.direct.undoList:
            self.undoButton['state'] = 'normal'
        else:
            self.undoButton['state'] = 'disabled'
        self.undoButton.pack(side = LEFT, expand = 0)
        self.bind(self.undoButton, 'Undo last operation')

        self.redoButton = Button(self.menuFrame, text = 'Redo',
                                 command = base.direct.redo)
        if base.direct.redoList:
            self.redoButton['state'] = 'normal'
        else:
            self.redoButton['state'] = 'disabled'
        self.redoButton.pack(side = LEFT, expand = 0)
        self.bind(self.redoButton, 'Redo last operation')

        # Record button
        mainFrame = Frame(interior, relief = SUNKEN, borderwidth = 2)
        frame = Frame(mainFrame)
        # Active node path
        # Button to select active node path
        widget = self.createButton(frame, 'Recording', 'Node Path:',
                                   'Select Active Mopath Node Path',
                                   lambda s = self: base.direct.select(s.nodePath),
                                   side = LEFT, expand = 0)
        widget['relief'] = FLAT
        self.nodePathMenu = Pmw.ComboBox(
            frame, entry_width = 20,
            selectioncommand = self.selectNodePathNamed,
            scrolledlist_items = self.nodePathNames)
        self.nodePathMenu.selectitem('camera')
        self.nodePathMenuEntry = (
            self.nodePathMenu.component('entryfield_entry'))
        self.nodePathMenuBG = (
            self.nodePathMenuEntry.configure('background')[3])
        self.nodePathMenu.pack(side = LEFT, fill = X, expand = 1)
        self.bind(self.nodePathMenu,
                  'Select active node path used for recording and playback')
        # Recording type
        self.recordingType = StringVar()
        self.recordingType.set('New Curve')
        widget = self.createRadiobutton(
            frame, 'left',
            'Recording', 'New Curve',
            ('Next record session records a new path'),
            self.recordingType, 'New Curve', expand = 0)
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
        frame.pack(fill = X, expand = 1)

        frame = Frame(mainFrame)
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
        frame.pack(fill = X, expand = 1)

        mainFrame.pack(expand = 1, fill = X, pady = 3)

        # Playback controls
        playbackFrame = Frame(interior, relief = SUNKEN,
                              borderwidth = 2)
        Label(playbackFrame, text = 'PLAYBACK CONTROLS',
              font=('MSSansSerif', 12, 'bold')).pack(fill = X)
        # Main playback control slider
        widget = self.createEntryScale(
            playbackFrame, 'Playback', 'Time', 'Set current playback time',
            resolution = 0.01, command = self.playbackGoTo, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        # Kill playback task if drag slider
        widget['preCallback'] = self.stopPlayback
        # Jam duration entry into entry scale
        self.createLabeledEntry(widget.labelFrame, 'Resample', 'Path Duration',
                                'Set total curve duration',
                                command = self.setPathDuration,
                                side = LEFT, expand = 0)
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

        # Speed control
        frame = Frame(playbackFrame)
        widget = Button(frame, text = 'PB Speed Vernier', relief = FLAT,
                        command = lambda s = self: s.setSpeedScale(1.0))
        widget.pack(side = LEFT, expand = 0)
        self.speedScale = Scale(frame, from_ = -1, to = 1,
                                resolution = 0.01, showvalue = 0,
                                width = 10, orient = 'horizontal',
                                command = self.setPlaybackSF)
        self.speedScale.pack(side = LEFT, fill = X, expand = 1)
        self.speedVar = StringVar()
        self.speedVar.set("0.00")
        self.speedEntry = Entry(frame, textvariable = self.speedVar,
                                width = 8)
        self.speedEntry.bind(
            '<Return>',
            lambda e = None, s = self: s.setSpeedScale(
            string.atof(s.speedVar.get())))
        self.speedEntry.pack(side = LEFT, expand = 0)
        frame.pack(fill = X, expand = 1)

        playbackFrame.pack(fill = X, pady = 2)

        # Create notebook pages
        self.mainNotebook = Pmw.NoteBook(interior)
        self.mainNotebook.pack(fill = BOTH, expand = 1)
        self.resamplePage = self.mainNotebook.add('Resample')
        self.refinePage = self.mainNotebook.add('Refine')
        self.extendPage = self.mainNotebook.add('Extend')
        self.cropPage = self.mainNotebook.add('Crop')
        self.drawPage = self.mainNotebook.add('Draw')
        self.optionsPage = self.mainNotebook.add('Options')

        ## RESAMPLE PAGE
        label = Label(self.resamplePage, text = 'RESAMPLE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        # Resample
        resampleFrame = Frame(
            self.resamplePage, relief = SUNKEN, borderwidth = 2)
        label = Label(resampleFrame, text = 'RESAMPLE CURVE',
                      font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createSlider(
            resampleFrame, 'Resample', 'Num. Samples',
            'Number of samples in resampled curve',
            resolution = 1, min = 2, max = 1000, command = self.setNumSamples)
        widget.component('hull')['relief'] = RIDGE
        widget['postCallback'] = self.sampleCurve

        frame = Frame(resampleFrame)
        self.createButton(
            frame, 'Resample', 'Make Even',
            'Apply timewarp so resulting path has constant velocity',
            self.makeEven, side = LEFT, fill = X, expand = 1)
        self.createButton(
            frame, 'Resample', 'Face Forward',
            'Compute HPR so resulting hpr curve faces along xyz tangent',
            self.faceForward, side = LEFT, fill = X, expand = 1)
        frame.pack(fill = X, expand = 0)
        resampleFrame.pack(fill = X, expand = 0, pady = 2)

        # Desample
        desampleFrame = Frame(
            self.resamplePage, relief = SUNKEN, borderwidth = 2)
        Label(desampleFrame, text = 'DESAMPLE CURVE',
              font=('MSSansSerif', 12, 'bold')).pack()
        widget = self.createSlider(
            desampleFrame, 'Resample', 'Points Between Samples',
            'Specify number of points to skip between samples',
            min = 1, max = 100, resolution = 1,
            command = self.setDesampleFrequency)
        widget.component('hull')['relief'] = RIDGE
        widget['postCallback'] = self.desampleCurve
        desampleFrame.pack(fill = X, expand = 0, pady = 2)

        ## REFINE PAGE ##
        refineFrame = Frame(self.refinePage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(refineFrame, text = 'REFINE CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createSlider(refineFrame,
                                       'Refine Page', 'Refine From',
                                       'Begin time of refine pass',
                                       resolution = 0.01,
                                       command = self.setRecordStart)
        widget['preCallback'] = self.setRefineMode
        widget['postCallback'] = lambda s = self: s.getPrePoints('Refine')
        widget = self.createSlider(
            refineFrame, 'Refine Page',
            'Control Start',
            'Time when full control of node path is given during refine pass',
            resolution = 0.01,
            command = self.setControlStart)
        widget['preCallback'] = self.setRefineMode
        widget = self.createSlider(
            refineFrame, 'Refine Page',
            'Control Stop',
            'Time when node path begins transition back to original curve',
            resolution = 0.01,
            command = self.setControlStop)
        widget['preCallback'] = self.setRefineMode
        widget = self.createSlider(refineFrame, 'Refine Page', 'Refine To',
                                       'Stop time of refine pass',
                                       resolution = 0.01,
                                       command = self.setRefineStop)
        widget['preCallback'] = self.setRefineMode
        widget['postCallback'] = self.getPostPoints
        refineFrame.pack(fill = X)

        ## EXTEND PAGE ##
        extendFrame = Frame(self.extendPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(extendFrame, text = 'EXTEND CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createSlider(extendFrame,
                                       'Extend Page', 'Extend From',
                                       'Begin time of extend pass',
                                       resolution = 0.01,
                                       command = self.setRecordStart)
        widget['preCallback'] = self.setExtendMode
        widget['postCallback'] = lambda s = self: s.getPrePoints('Extend')
        widget = self.createSlider(
            extendFrame, 'Extend Page',
            'Control Start',
            'Time when full control of node path is given during extend pass',
            resolution = 0.01,
            command = self.setControlStart)
        widget['preCallback'] = self.setExtendMode
        extendFrame.pack(fill = X)

        ## CROP PAGE ##
        cropFrame = Frame(self.cropPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(cropFrame, text = 'CROP CURVE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        widget = self.createSlider(
            cropFrame,
            'Crop Page', 'Crop From',
            'Delete all curve points before this time',
            resolution = 0.01,
            command = self.setCropFrom)

        widget = self.createSlider(
            cropFrame,
            'Crop Page', 'Crop To',
            'Delete all curve points after this time',
            resolution = 0.01,
            command = self.setCropTo)

        self.createButton(cropFrame, 'Crop Page', 'Crop Curve',
                          'Crop curve to specified from to times',
                          self.cropCurve, fill = NONE)
        cropFrame.pack(fill = X)

        ## DRAW PAGE ##
        drawFrame = Frame(self.drawPage, relief = SUNKEN,
                           borderwidth = 2)

        self.sf = Pmw.ScrolledFrame(self.drawPage, horizflex = 'elastic')
        self.sf.pack(fill = 'both', expand = 1)
        sfFrame = self.sf.interior()

        label = Label(sfFrame, text = 'CURVE RENDERING STYLE',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)

        frame = Frame(sfFrame)
        Label(frame, text = 'SHOW:').pack(side = LEFT, expand = 0)
        widget = self.createCheckbutton(
            frame, 'Style', 'Path',
            'On: path is visible', self.setPathVis, 1,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Knots',
            'On: path knots are visible', self.setKnotVis, 1,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'CVs',
            'On: path CVs are visible', self.setCvVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Hull',
            'On: path hull is visible', self.setHullVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Trace',
            'On: record is visible', self.setTraceVis, 0,
            side = LEFT, fill = X, expand = 1)
        widget = self.createCheckbutton(
            frame, 'Style', 'Marker',
            'On: playback marker is visible', self.setMarkerVis, 0,
            side = LEFT, fill = X, expand = 1)
        frame.pack(fill = X, expand = 1)
        # Sliders
        widget = self.createSlider(
            sfFrame, 'Style', 'Num Segs',
            'Set number of segments used to approximate each parametric unit',
            min = 1.0, max = 400, resolution = 1.0,
            value = 40,
            command = self.setNumSegs, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        widget = self.createSlider(
            sfFrame, 'Style', 'Num Ticks',
            'Set number of tick marks drawn for each unit of time',
            min = 0.0, max = 10.0, resolution = 1.0,
            value = 0.0,
            command = self.setNumTicks, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        widget = self.createSlider(
            sfFrame, 'Style', 'Tick Scale',
            'Set visible size of time tick marks',
            min = 0.01, max = 100.0, resolution = 0.01,
            value = 5.0,
            command = self.setTickScale, side = TOP)
        widget.component('hull')['relief'] = RIDGE
        self.createColorEntry(
            sfFrame, 'Style', 'Path Color',
            'Color of curve',
            command = self.setPathColor,
            value = [255.0, 255.0, 255.0, 255.0])
        self.createColorEntry(
            sfFrame, 'Style', 'Knot Color',
            'Color of knots',
            command = self.setKnotColor,
            value = [0, 0, 255.0, 255.0])
        self.createColorEntry(
            sfFrame, 'Style', 'CV Color',
            'Color of CVs',
            command = self.setCvColor,
            value = [255.0, 0, 0, 255.0])
        self.createColorEntry(
            sfFrame, 'Style', 'Tick Color',
            'Color of Ticks',
            command = self.setTickColor,
            value = [255.0, 0, 0, 255.0])
        self.createColorEntry(
            sfFrame, 'Style', 'Hull Color',
            'Color of Hull',
            command = self.setHullColor,
            value = [255.0, 128.0, 128.0, 255.0])

        #drawFrame.pack(fill = X)

        ## OPTIONS PAGE ##
        optionsFrame = Frame(self.optionsPage, relief = SUNKEN,
                            borderwidth = 2)
        label = Label(optionsFrame, text = 'RECORDING OPTIONS',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = X)
        # Hooks
        frame = Frame(optionsFrame)
        widget = self.createLabeledEntry(
            frame, 'Recording', 'Record Hook',
            'Hook used to start/stop recording',
            value = self.startStopHook,
            command = self.setStartStopHook)[0]
        label = self.getWidget('Recording', 'Record Hook-Label')
        label.configure(width = 16, anchor = W)
        self.setStartStopHook()
        widget = self.createLabeledEntry(
            frame, 'Recording', 'Keyframe Hook',
            'Hook used to add a new keyframe',
            value = self.keyframeHook,
            command = self.setKeyframeHook)[0]
        label = self.getWidget('Recording', 'Keyframe Hook-Label')
        label.configure(width = 16, anchor = W)
        self.setKeyframeHook()
        frame.pack(expand = 1, fill = X)
        # PreRecordFunc
        frame = Frame(optionsFrame)
        widget = self.createComboBox(
            frame, 'Recording', 'Pre-Record Func',
            'Function called before sampling each point',
            PRF_UTILITIES, self.setPreRecordFunc,
            history = 1, expand = 1)
        widget.configure(label_width = 16, label_anchor = W)
        widget.configure(entryfield_entry_state = 'normal')
        # Initialize preRecordFunc
        self.preRecordFunc = eval(PRF_UTILITIES[0])
        self.createCheckbutton(frame, 'Recording', 'PRF Active',
                               'On: Pre Record Func enabled',
                               None, 0,
                               side = LEFT, fill = BOTH, expand = 0)
        frame.pack(expand = 1, fill = X)
        # Pack record frame
        optionsFrame.pack(fill = X, pady = 2)

        self.mainNotebook.setnaturalsize()

    def pushUndo(self, fResetRedo = 1):
        base.direct.pushUndo([self.nodePath])

    def undoHook(self, nodePathList = []):
        # Reflect new changes
        pass

    def pushUndoHook(self):
        # Make sure button is reactivated
        self.undoButton.configure(state = 'normal')

    def undoListEmptyHook(self):
        # Make sure button is deactivated
        self.undoButton.configure(state = 'disabled')

    def pushRedo(self):
        base.direct.pushRedo([self.nodePath])

    def redoHook(self, nodePathList = []):
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
        taskMgr.remove(self.name + '-curveEditTask')
        print nodePath.id()
        if nodePath.id() in self.playbackMarkerIds:
            base.direct.select(self.playbackMarker)
        elif nodePath.id() in self.tangentMarkerIds:
            base.direct.select(self.tangentMarker)
        elif nodePath.id() == self.playbackMarker.id():
            self.tangentGroup.show()
            taskMgr.add(self.curveEditTask,
                                     self.name + '-curveEditTask')
        elif nodePath.id() == self.tangentMarker.id():
            self.tangentGroup.show()
            taskMgr.add(self.curveEditTask,
                                     self.name + '-curveEditTask')
        else:
            self.tangentGroup.hide()

    def getChildIds(self, nodePath):
        ids = [nodePath.id()]
        kids = nodePath.getChildren()
        for kid in kids:
            ids += self.getChildIds(kid)
        return ids

    def deselectedNodePathHook(self, nodePath):
        """
        Hook called upon deselection of a node path used to select playback
        marker if subnode selected
        """
        if ((nodePath.id() == self.playbackMarker.id()) or
            (nodePath.id() == self.tangentMarker.id())):
            self.tangentGroup.hide()

    def curveEditTask(self, state):
        if self.curveCollection != None:
            # Update curve position
            if self.manipulandumId == self.playbackMarker.id():
                # Show playback marker
                self.playbackMarker.getChild(0).show()
                pos = Point3(0)
                hpr = Point3(0)
                pos = self.playbackMarker.getPos(self.nodePathParent)
                hpr = self.playbackMarker.getHpr(self.nodePathParent)
                self.curveCollection.adjustXyz(
                    self.playbackTime, VBase3(pos[0], pos[1], pos[2]))
                self.curveCollection.adjustHpr(
                    self.playbackTime, VBase3(hpr[0], hpr[1], hpr[2]))
                # Note: this calls recompute on the curves
                self.nurbsCurveDrawer.draw()
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
                self.curveCollection.getXyzCurve().adjustTangent(
                    self.playbackTime,
                    tan2Curve[0], tan2Curve[1], tan2Curve[2])
                # Note: this calls recompute on the curves
                self.nurbsCurveDrawer.draw()
            else:
                # Show playback marker
                self.playbackMarker.getChild(0).show()
                # Update tangent marker line
                tan = Point3(0)
                self.curveCollection.getXyzCurve().getTangent(
                    self.playbackTime, tan)
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
        if base.direct.selected.last:
            if base.direct.selected.last.id() == self.playbackMarker.id():
                self.manipulandumId = self.playbackMarker.id()
            elif base.direct.selected.last.id() == self.tangentMarker.id():
                self.manipulandumId = self.tangentMarker.id()

    def manipulateObjectCleanupHook(self, nodePathList = []):
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
        base.direct.deselect(self.playbackMarker)
        base.direct.deselect(self.tangentMarker)
        # Remove tasks
        taskMgr.remove(self.name + '-recordTask')
        taskMgr.remove(self.name + '-playbackTask')
        taskMgr.remove(self.name + '-curveEditTask')

    def createNewPointSet(self):
        self.pointSetName = self.name + '-ps-' + `self.pointSetCount`
        # Update dictionary and record pointer to new point set
        self.pointSet = self.pointSetDict[self.pointSetName] = []
        # Update combo box
        comboBox = self.getWidget('Mopath', 'History')
        scrolledList = comboBox.component('scrolledlist')
        listbox = scrolledList.component('listbox')
        names = list(listbox.get(0,'end'))
        names.append(self.pointSetName)
        scrolledList.setlist(names)
        comboBox.selectitem(self.pointSetName)
        # Update count
        self.pointSetCount += 1

    def extractPointSetFromCurveFitter(self):
        # Get new point set based on newly created curve
        self.createNewPointSet()
        for i in range(self.curveFitter.getNumSamples()):
            time = self.curveFitter.getSampleT(i)
            pos = Point3(self.curveFitter.getSampleXyz(i))
            hpr = Point3(self.curveFitter.getSampleHpr(i))
            self.pointSet.append([time, pos, hpr])

    def extractPointSetFromCurveCollection(self):
        # Use curve to compute new point set
        # Record maxT
        self.maxT = self.curveCollection.getMaxT()
        # Determine num samples
        # Limit point set to 1000 points and samples per second to 30
        samplesPerSegment = min(30.0, 1000.0/self.curveCollection.getMaxT())
        self.setNumSamples(self.maxT * samplesPerSegment)
        # Sample the curve but don't create a new curve collection
        self.sampleCurve(fCompute = 0)
        # Update widgets based on new data
        self.updateWidgets()

    def selectPointSetNamed(self, name):
        self.pointSet = self.pointSetDict.get(name, None)
        # Reload points into curve fitter
        # Reset curve fitters
        self.curveFitter.reset()
        for time, pos, hpr in self.pointSet:
            # Add it to the curve fitters
            self.curveFitter.addXyzHpr(time, pos, hpr)
        # Compute curve
        self.computeCurves()

    def setPathVis(self):
        if self.getVariable('Style', 'Path').get():
            self.curveNodePath.show()
        else:
            self.curveNodePath.hide()

    def setKnotVis(self):
        self.nurbsCurveDrawer.setShowKnots(
            self.getVariable('Style', 'Knots').get())

    def setCvVis(self):
        self.nurbsCurveDrawer.setShowCvs(
            self.getVariable('Style', 'CVs').get())

    def setHullVis(self):
        self.nurbsCurveDrawer.setShowHull(
            self.getVariable('Style', 'Hull').get())

    def setTraceVis(self):
        if self.getVariable('Style', 'Trace').get():
            self.trace.show()
        else:
            self.trace.hide()

    def setMarkerVis(self):
        if self.getVariable('Style', 'Marker').get():
            self.playbackMarker.reparentTo(self.recorderNodePath)
        else:
            self.playbackMarker.reparentTo(hidden)

    def setNumSegs(self, value):
        self.numSegs = int(value)
        self.nurbsCurveDrawer.setNumSegs(self.numSegs)

    def setNumTicks(self, value):
        self.nurbsCurveDrawer.setNumTicks(float(value))

    def setTickScale(self, value):
        self.nurbsCurveDrawer.setTickScale(float(value))

    def setPathColor(self, color):
        self.nurbsCurveDrawer.setColor(
            color[0]/255.0, color[1]/255.0, color[2]/255.0)
        self.nurbsCurveDrawer.draw()

    def setKnotColor(self, color):
        self.nurbsCurveDrawer.setKnotColor(
            color[0]/255.0, color[1]/255.0, color[2]/255.0)

    def setCvColor(self, color):
        self.nurbsCurveDrawer.setCvColor(
            color[0]/255.0, color[1]/255.0, color[2]/255.0)

    def setTickColor(self, color):
        self.nurbsCurveDrawer.setTickColor(
            color[0]/255.0, color[1]/255.0, color[2]/255.0)

    def setHullColor(self, color):
        self.nurbsCurveDrawer.setHullColor(
            color[0]/255.0, color[1]/255.0, color[2]/255.0)

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
        self.curveCollection = None
        self.curveFitter.reset()
        self.nurbsCurveDrawer.hide()

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
            taskMgr.remove(self.name + '-recordTask')
            taskMgr.remove(self.name + '-curveEditTask')
            # Remove old curve
            self.nurbsCurveDrawer.hide()
            # Reset curve fitters
            self.curveFitter.reset()
            # Update sampling mode button if necessary
            if self.samplingMode == 'Continuous':
                self.disableKeyframeButton()
            # Create a new point set to hold raw data
            self.createNewPointSet()
            # Clear out old trace, get ready to draw new
            self.initTrace()
            # Keyframe mode?
            if (self.samplingMode == 'Keyframe'):
                # Record first point
                self.lastPos.assign(Point3(
                    self.nodePath.getPos(self.nodePathParent)))
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
                    self.oldPlaybackNodePath = self.playbackNodePath
                    self.setPlaybackNodePath(self.tempCS)
                    # Parent record node path to temp
                    self.nodePath.reparentTo(self.playbackNodePath)
                    # Align with temp
                    self.nodePath.setPosHpr(0, 0, 0, 0, 0, 0)
                    # Set playback start to self.recordStart
                    self.playbackGoTo(self.recordStart)
                    # start flying nodePath along path
                    self.startPlayback()
                # Start new task
                t = taskMgr.add(
                    self.recordTask, self.name + '-recordTask')
                t.startTime = globalClock.getFrameTime()
        else:
            if self.samplingMode == 'Continuous':
                # Kill old task
                taskMgr.remove(self.name + '-recordTask')
                if ((self.recordingType.get() == 'Refine') or
                    (self.recordingType.get() == 'Extend')):
                    # Reparent node path back to parent
                    self.nodePath.wrtReparentTo(self.nodePathParent)
                    # Restore playback Node Path
                    self.setPlaybackNodePath(self.oldPlaybackNodePath)
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
        time = self.recordStart + (
            globalClock.getFrameTime() - state.startTime)
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
            pos = self.nodePath.getPos(self.nodePathParent)
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
        pos = self.nodePath.getPos(self.nodePathParent)
        hpr = self.nodePath.getHpr(self.nodePathParent)
        qNP = Quat()
        qNP.setHpr(hpr)
        # Blend between recordNodePath and self.nodePath
        if ((self.recordingType.get() == 'Refine') or
            (self.recordingType.get() == 'Extend')):
            if ((time < self.controlStart) and
                ((self.controlStart - self.recordStart) != 0.0)):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                qR = Quat()
                qR.setHpr(rHpr)
                t = self.easeInOut(((time - self.recordStart)/
                                    (self.controlStart - self.recordStart)))
                # Transition between the recorded node path and the driven one
                pos = (rPos * (1 - t)) + (pos * t)
                q = qSlerp(qR, qNP, t)
                hpr.assign(q.getHpr())
            elif ((self.recordingType.get() == 'Refine') and
                  (time > self.controlStop) and
                  ((self.recordStop - self.controlStop) != 0.0)):
                rPos = self.playbackNodePath.getPos(self.nodePathParent)
                rHpr = self.playbackNodePath.getHpr(self.nodePathParent)
                qR = Quat()
                qR.setHpr(rHpr)
                t = self.easeInOut(((time - self.controlStop)/
                                    (self.recordStop - self.controlStop)))
                # Transition between the recorded node path and the driven one
                pos = (pos * (1 - t)) + (rPos * t)
                q = qSlerp(qNP, qR, t)
                hpr.assign(q.getHpr())
        # Add it to the point set
        self.pointSet.append([time, pos, hpr])
        # Add it to the curve fitters
        self.curveFitter.addXyzHpr(time, pos, hpr)
        # Update trace now if recording keyframes
        if (self.samplingMode == 'Keyframe'):
            self.trace.reset()
            for t, p, h in self.pointSet:
                self.trace.drawTo(p[0], p[1], p[2])
            self.trace.create()

    def computeCurves(self):
        # Check to make sure curve fitters have points
        if (self.curveFitter.getNumSamples() == 0):
            print 'MopathRecorder.computeCurves: Must define curve first'
            return
        # Create curves
        # XYZ
        self.curveFitter.sortPoints()
        self.curveFitter.wrapHpr()
        self.curveFitter.computeTangents(1)
        # This is really a collection
        self.curveCollection = self.curveFitter.makeNurbs()
        self.nurbsCurveDrawer.setCurves(self.curveCollection)
        self.nurbsCurveDrawer.draw()
        # Update widget based on new curve
        self.updateWidgets()

    def initTrace(self):
        self.trace.reset()
        # Put trace line segs under node path's parent
        self.trace.reparentTo(self.nodePathParent)
        # Show it
        self.trace.show()

    def updateWidgets(self):
        if not self.curveCollection:
            return
        self.fAdjustingValues = 1
        # Widgets depending on max T
        maxT = self.curveCollection.getMaxT()
        maxT_text = '%0.2f' % maxT
        # Playback controls
        self.getWidget('Playback', 'Time').configure(max = maxT_text)
        self.getVariable('Resample', 'Path Duration').set(maxT_text)
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
        numSamples = self.curveFitter.getNumSamples()
        widget = self.getWidget('Resample', 'Points Between Samples')
        widget.configure(max=numSamples)
        widget = self.getWidget('Resample', 'Num. Samples')
        widget.configure(max = 4 * numSamples)
        widget.set(numSamples, 0)
        self.fAdjustingValues = 0

    def selectNodePathNamed(self, name):
        nodePath = None
        if name == 'init':
            nodePath = self.nodePath
            # Add Combo box entry for the initial node path
            self.addNodePath(nodePath)
        elif name == 'selected':
            nodePath = base.direct.selected.last
            # Add Combo box entry for this selected object
            self.addNodePath(nodePath)
        else:
            nodePath = self.nodePathDict.get(name, None)
            if (nodePath == None):
                # See if this evaluates into a node path
                try:
                    nodePath = eval(name)
                    if isinstance(nodePath, NodePath):
                        self.addNodePath(nodePath)
                    else:
                        # Good eval but not a node path, give up
                        nodePath = None
                except:
                    # Bogus eval
                    nodePath = None
                    # Clear bogus entry from listbox
                    listbox = self.nodePathMenu.component('scrolledlist')
                    listbox.setlist(self.nodePathNames)
            else:
                if name == 'widget':
                    # Record relationship between selected nodes and widget
                    base.direct.selected.getWrtAll()
                if name == 'marker':
                    self.playbackMarker.show()
                    # Initialize tangent marker position
                    tan = Point3(0)
                    if self.curveCollection != None:
                        self.curveCollection.getXyzCurve().getTangent(
                            self.playbackTime, tan)
                    self.tangentMarker.setPos(tan)
                else:
                    self.playbackMarker.hide()
        # Update active node path
        self.setNodePath(nodePath)

    def setNodePath(self, nodePath):
        self.playbackNodePath = self.nodePath = nodePath
        if self.nodePath:
            # Record nopath's parent
            self.nodePathParent = self.nodePath.getParent()
            # Put curve drawer under record node path's parent
            self.curveNodePath.reparentTo(self.nodePathParent)
            # Set entry color
            self.nodePathMenuEntry.configure(
                background = self.nodePathMenuBG)
        else:
            # Flash entry
            self.nodePathMenuEntry.configure(background = 'Pink')

    def setPlaybackNodePath(self, nodePath):
        self.playbackNodePath = nodePath

    def addNodePath(self, nodePath):
        self.addNodePathToDict(nodePath, self.nodePathNames,
                               self.nodePathMenu, self.nodePathDict)

    def addNodePathToDict(self, nodePath, names, menu, dict):
        if not nodePath:
            return
        # Get node path's name
        name = nodePath.getName()
        if name in ['mopathRecorderTempCS', 'widget', 'camera', 'marker']:
            dictName = name
        else:
            # Generate a unique name for the dict
            dictName = name + '-' + `nodePath.id()`
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
        if self.curveCollection == None:
            return
        self.playbackTime = CLAMP(time, 0.0, self.maxT)
        if self.curveCollection != None:
            pos = Point3(0)
            hpr = Point3(0)
            self.curveCollection.evaluate(self.playbackTime, pos, hpr)
            self.playbackNodePath.setPosHpr(self.nodePathParent, pos, hpr)

    def startPlayback(self):
        if self.curveCollection == None:
            return
        # Kill any existing tasks
        self.stopPlayback()
        # Make sure checkbutton is set
        self.getVariable('Playback', 'Play').set(1)
        # Start new playback task
        t = taskMgr.add(
            self.playbackTask, self.name + '-playbackTask')
        t.currentTime = self.playbackTime
        t.lastTime = globalClock.getFrameTime()

    def setSpeedScale(self, value):
        self.speedScale.set(math.log10(value))

    def setPlaybackSF(self, value):
        self.playbackSF = pow(10.0, float(value))
        self.speedVar.set('%0.2f' % self.playbackSF)

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
        elif ((self.loopPlayback == 0) and (cTime > self.maxT)):
            # Go to maxT
            self.getWidget('Playback', 'Time').set(self.maxT)
            # Then stop playback
            self.stopPlayback()
            return Task.done
        elif ((self.recordingType.get() == 'Extend') and
              (cTime > self.controlStart)):
            # Go to final point
            self.getWidget('Playback', 'Time').set(self.controlStart)
            # Stop playback
            self.stopPlayback()
            return Task.done
        # Otherwise go to specified time and continue
        self.getWidget('Playback', 'Time').set(cTime)
        state.currentTime = cTime
        return Task.cont

    def stopPlayback(self):
        self.getVariable('Playback', 'Play').set(0)
        taskMgr.remove(self.name + '-playbackTask')

    def jumpToStartOfPlayback(self):
        self.stopPlayback()
        self.getWidget('Playback', 'Time').set(0.0)

    def jumpToEndOfPlayback(self):
        self.stopPlayback()
        if self.curveCollection != None:
            self.getWidget('Playback', 'Time').set(self.maxT)

    def startStopPlayback(self):
        if self.getVariable('Playback', 'Play').get():
            self.startPlayback()
        else:
            self.stopPlayback()

    def setDesampleFrequency(self, frequency):
        self.desampleFrequency = frequency

    def desampleCurve(self):
        if (self.curveFitter.getNumSamples() == 0):
            print 'MopathRecorder.desampleCurve: Must define curve first'
            return
        # NOTE: This is destructive, points will be deleted from curve fitter
        self.curveFitter.desample(self.desampleFrequency)
        # Compute new curve based on desampled data
        self.computeCurves()
        # Get point set from the curve fitter
        self.extractPointSetFromCurveFitter()

    def setNumSamples(self, numSamples):
        self.numSamples = int(numSamples)

    def sampleCurve(self, fCompute = 1):
        if self.curveCollection == None:
            print 'MopathRecorder.sampleCurve: Must define curve first'
            return
        # Reset curve fitters
        self.curveFitter.reset()
        # Sample curve using specified number of samples
        self.curveFitter.sample(self.curveCollection, self.numSamples)
        if fCompute:
            # Now recompute curves
            self.computeCurves()
        # Get point set from the curve fitter
        self.extractPointSetFromCurveFitter()

    def makeEven(self):
        # Note: segments_per_unit = 2 seems to give a good fit
        self.curveCollection.makeEven(self.maxT, 2)
        # Get point set from curve
        self.extractPointSetFromCurveCollection()

    def faceForward(self):
        # Note: segments_per_unit = 2 seems to give a good fit
        self.curveCollection.faceForward(2)
        # Get point set from curve
        self.extractPointSetFromCurveCollection()

    def setPathDuration(self, event):
        newMaxT = float(self.getWidget('Resample', 'Path Duration').get())
        self.setPathDurationTo(newMaxT)

    def setPathDurationTo(self, newMaxT):
        # Compute scale factor
        sf = newMaxT/self.maxT
        # Scale curve collection
        self.curveCollection.resetMaxT(newMaxT)
        # Scale point set
        # Save handle to old point set
        oldPointSet = self.pointSet
        # Create new point set
        self.createNewPointSet()
        # Reset curve fitters
        self.curveFitter.reset()
        # Now scale values
        for time, pos, hpr in oldPointSet:
            newTime = time * sf
            # Update point set
            self.pointSet.append([newTime, Point3(pos), Point3(hpr)])
            # Add it to the curve fitters
            self.curveFitter.addXyzHpr(newTime, pos, hpr)
        # Update widgets
        self.updateWidgets()
        # Compute curve
        #self.computeCurves()

    def setRecordStart(self, value):
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
            self.curveFitter.addXyzHpr(time, pos, hpr)
        # And post points
        # What is end time of pointSet?
        endTime = self.pointSet[-1][0]
        for time, pos, hpr in self.postPoints:
            adjustedTime = endTime + (time - self.recordStop)
            # Add it to point set
            self.pointSet.append([adjustedTime, pos, hpr])
            # Add it to the curve fitters
            self.curveFitter.addXyzHpr(adjustedTime, pos, hpr)

    def setCropFrom(self, value):
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

    def setCropTo(self, value):
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
        self.curveFitter.reset()
        # Add start point
        pos = Point3(0)
        hpr = Point3(0)
        self.curveCollection.evaluate(self.cropFrom, pos, hpr)
        self.curveFitter.addXyzHpr(0.0, pos, hpr)
        # Get points within bounds
        for time, pos, hpr in oldPoints:
            # Is it within the time?
            if ((time > self.cropFrom) and
                (time < self.cropTo)):
                # Add it to the curve fitters
                t = time - self.cropFrom
                self.curveFitter.addXyzHpr(t, pos, hpr)
                # And the point set
                self.pointSet.append([t, pos, hpr])
        # Add last point
        pos = Vec3(0)
        hpr = Vec3(0)
        self.curveCollection.evaluate(self.cropTo, pos, hpr)
        self.curveFitter.addXyzHpr(self.cropTo - self.cropFrom, pos, hpr)
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
            nodePath = loader.loadModel(
                Filename.fromOsSpecific(mopathFilename))
            self.curveCollection = ParametricCurveCollection()
            # MRM: Add error check
            self.curveCollection.addCurves(nodePath.node())
            nodePath.removeNode()
            if self.curveCollection:
                # Draw the curve
                self.nurbsCurveDrawer.setCurves(self.curveCollection)
                self.nurbsCurveDrawer.draw()
                # Save a pointset for this curve
                self.extractPointSetFromCurveCollection()
            else:
                self.reset()

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
            self.curveCollection.writeEgg(Filename(mopathFilename))

    def followTerrain(self, height = 1.0):
        self.iRay.rayCollisionNodePath.reparentTo(self.nodePath)
        entry = self.iRay.pickGeom3D()
        if entry:
            hitPtDist = Vec3(entry.getFromIntersectionPoint()).length()
            self.nodePath.setZ(self.nodePath, height - hitPtDist)
        self.iRay.rayCollisionNodePath.reparentTo(self.recorderNodePath)

    ## WIDGET UTILITY FUNCTIONS ##
    def addWidget(self, widget, category, text):
        self.widgetDict[category + '-' + text] = widget

    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]

    def createLabeledEntry(self, parent, category, text, balloonHelp,
                           value = '', command = None,
                           relief = 'sunken', side = LEFT,
                           expand = 1, width = 12):
        frame = Frame(parent)
        variable = StringVar()
        variable.set(value)
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
        widget = apply(Dial.AngleDial, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
        self.bind(widget, balloonHelp)
        self.widgetDict[category + '-' + text] = widget
        return widget

    def createSlider(self, parent, category, text, balloonHelp,
                         command = None, min = 0.0, max = 1.0,
                         resolution = None,
                         side = TOP, fill = X, expand = 1, **kw):
        kw['text'] = text
        kw['min'] = min
        kw['max'] = max
        kw['resolution'] = resolution
        #widget = apply(EntryScale.EntryScale, (parent,), kw)
        from direct.tkwidgets import Slider
        widget = apply(Slider.Slider, (parent,), kw)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(side = side, fill = fill, expand = expand)
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
        widget = apply(VectorWidgets.ColorEntry, (parent,), kw)
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

    def makeCameraWindow(self):
        # First, we need to make a new layer on the window.
        chan = base.win.getChannel(0)
        self.cLayer = chan.makeLayer(1)
        self.layerIndex = 1
        self.cDr = self.cLayer.makeDisplayRegion(0.6, 1.0, 0, 0.4)
        self.cDr.setClearDepthActive(1)
        self.cDr.setClearColorActive(1)
        self.cDr.setClearColor(Vec4(0))

        # It gets its own camera
        self.cCamera = render.attachNewNode('cCamera')
        self.cCamNode = Camera('cCam')
        self.cLens = PerspectiveLens()
        self.cLens.setFov(40, 40)
        self.cLens.setNear(0.1)
        self.cLens.setFar(100.0)
        self.cCamNode.setLens(self.cLens)
        self.cCamNode.setScene(render)
        self.cCam = self.cCamera.attachNewNode(self.cCamNode)

        self.cDr.setCamera(self.cCam)

