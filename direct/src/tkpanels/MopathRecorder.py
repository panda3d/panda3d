""" Mopath Recorder Panel Module """

# Import Tkinter, Pmw, and the dial code from this directory tree.
from PandaObject import *
from Tkinter import *
from AppShell import *
from DirectGeometry import *
import Pmw
import Dial
import Floater

class MopathRecorder(AppShell):
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
        self.pointSetCount = 0
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
        self.pathVis = BooleanVar()
        self.pathVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Path Visability',
                                 label = 'Toggle Path Vis',
                                 variable = self.pathVis,
                                 command = self.setPathVis)
        self.traceVis = BooleanVar()
        self.traceVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Trace Visability',
                                 label = 'Toggle Trace Vis',
                                 variable = self.traceVis,
                                 command = self.setTraceVis)
        self.cvVis = BooleanVar()
        self.cvVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle CV Visability',
                                 label = 'Toggle CV Vis',
                                 variable = self.cvVis,
                                 command = self.setCvVis)
        self.markerVis = BooleanVar()
        self.markerVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Marker Visability',
                                 label = 'Toggle Marker Vis',
                                 variable = self.markerVis,
                                 command = self.setMarkerVis)
        self.tickVis = BooleanVar()
        self.tickVis.set(1)
        self.menuBar.addmenuitem('Show/Hide', 'checkbutton',
                                 'Toggle Tick Visability',
                                 label = 'Toggle Tick Vis',
                                 variable = self.tickVis,
                                 command = self.setTickVis)
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
                            'Select input points to fit curve to',
                            [self['name'] + '-ps-' + `self.pointSetCount`],
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
        label = Label(self.recordPage, text = 'RECORD PATH',
                      font=('MSSansSerif', 12, 'bold'))
        label.pack(fill = 'x')
        frame = Frame(self.recordPage)
        # Recording Button
        self.createCheckbutton(frame, 'Recording', 'Record Path',
                               'On: path is being recorded',
                               self.toggleRecord, 0)
        Label(frame, text = 'Start/Stop Hook').pack(side = 'left', fill = 'x')
        entry = Entry(frame)
        entry.pack(side = 'left', fill = 'x', expand = 1)
        
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
        self.tempCS.removeNode()
        self.orbitFromCS.removeNode()
        self.orbitToCS.removeNode()

    def selectPointSetNamed(self, setName):
        print setName

    def setPathVis(self):
        print self.pathVis.get()
        
    def setTraceVis(self):
        print self.traceVis.get()
        
    def setCvVis(self):
        print self.cvVis.get()
        
    def setMarkerVis(self):
        print self.markerVis.get()
        
    def setTickVis(self):
        print self.tickVis.get()

    def toggleRecord(self):
        print self.getVariable('Recording', 'Record Path').get()
    
    ## WIDGET UTILITY FUNCTIONS ##
    def addWidget(self, widget, category, text):
        self.widgetDict[category + '-' + text] = widget
        
    def getWidget(self, category, text):
        return self.widgetDict[category + '-' + text]

    def getVariable(self, category, text):
        return self.variableDict[category + '-' + text]
    
    def createCheckbutton(self, parent, category, text,
                          balloonHelp, command, initialState):
        bool = BooleanVar()
        bool.set(initialState)
        widget = Checkbutton(parent, text = text, anchor = W,
                         variable = bool)
        # Do this after the widget so command isn't called on creation
        widget['command'] = command
        widget.pack(fill = X)
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

