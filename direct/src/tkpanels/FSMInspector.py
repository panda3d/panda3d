"""Defines the `FSMInspector` class, which opens a Tkinter window for
inspecting :ref:`finite-state-machines`.

Using the Finite State Inspector
--------------------------------

1) In your Config.prc add::

    want-tk #t

2) Start up the show and create a Finite State Machine::

    from direct.showbase.ShowBaseGlobal import *

    from direct.fsm import ClassicFSM
    from direct.fsm import State

    def enterState():
        print('enterState')

    def exitState():
        print 'exitState'

    fsm = ClassicFSM.ClassicFSM('stopLight',
              [State.State('red', enterState, exitState, ['green']),
                State.State('yellow', enterState, exitState, ['red']),
                State.State('green', enterState, exitState, ['yellow'])],
              'red',
              'red')

    import FSMInspector

    inspector = FSMInspector.FSMInspector(fsm, title = fsm.getName())

    # Note, the inspectorPos argument is optional, the inspector will
    # automagically position states on startup
    fsm = ClassicFSM.ClassicFSM('stopLight', [
        State.State('yellow',
                    enterState,
                    exitState,
                    ['red'],
                    inspectorPos = [95.9, 48.0]),
        State.State('red',
                    enterState,
                    exitState,
                    ['green'],
                    inspectorPos = [0.0, 0.0]),
        State.State('green',
                    enterState,
                    exitState,
                    ['yellow'],
                    inspectorPos = [0.0, 95.9])],
            'red',
            'red')

3) Pop open a viewer::

    import FSMInspector
    insp = FSMInspector.FSMInspector(fsm)

or if you wish to be fancy::

    insp = FSMInspector.FSMInspector(fsm, title = fsm.getName())

Features:

  - Right mouse button over a state pops up a menu allowing you to
    request a transition to that state
  - Middle mouse button will grab the canvas and slide things around if
    your state machine is bigger than the viewing area
  - There are some self explanatory menu options up at the top, the most
    useful being: "print ClassicFSM layout" which will print out Python
    code which will create an ClassicFSM augmented with layout
    information for the viewer so everything shows up in the same place
    the next time you inspect the state machine

Caveat
------

There is an unexplained problem with using Tk and emacs right now which
occasionally results in everything locking up.  This procedure seems to
avoid the problem for me::

   # Start up the show
   from direct.showbase.ShowBaseGlobal import *

   # You will see the window and a Tk panel pop open

   # Type a number at the emacs prompt
   >>> 123

   # At this point everything will lock up and you won't get your prompt back

   # Hit a bunch of Control-C's in rapid succession, in most cases
   # this will break you out of whatever badness you were in and
   # from that point on everything will behave normally


   # This is how you pop up an inspector
   import FSMInspector
   inspector = FSMInspector.FSMInspector(fsm, title = fsm.getName())

"""

__all__ = ['FSMInspector', 'StateInspector']

from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *
import Pmw, math, operator, sys

if sys.version_info >= (3, 0):
    from tkinter.simpledialog import askstring
else:
    from tkSimpleDialog import askstring


DELTA = (5.0 / 360.) * 2.0 * math.pi


class FSMInspector(AppShell):
    # Override class variables
    appname = 'ClassicFSM Inspector'
    frameWidth  = 400
    frameHeight = 450
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, fsm, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title', fsm.getName(), None),
            ('gridSize', '0.25i', self._setGridSize),
            )
        self.defineoptions(kw, optiondefs)

        self.fsm = fsm
        # Tell the fsm we are inspecting it so it will send events
        # when it changes state
        self.fsm.inspecting = 1

        AppShell.__init__(self)

        self.initialiseoptions(FSMInspector)

    def appInit(self):
        # Initialize instance variables
        self.states = []
        self.stateInspectorDict = {}
        self.name = self.fsm.getName()

    def createInterface(self):
        # Create the components
        interior = self.interior()
        menuBar = self.menuBar

        # ClassicFSM Menu
        menuBar.addmenu('ClassicFSM', 'ClassicFSM Operations')
        menuBar.addmenuitem('ClassicFSM', 'command',
                                  'Input grid spacing',
                                  label = 'Grid spacing...',
                                  command = self.popupGridDialog)
        # Create the checkbutton variable
        self._fGridSnap = IntVar()
        self._fGridSnap.set(1)
        menuBar.addmenuitem('ClassicFSM', 'checkbutton',
                                  'Enable/disable grid',
                                  label = 'Snap to grid',
                                  variable = self._fGridSnap,
                                  command = self.toggleGridSnap)
        menuBar.addmenuitem('ClassicFSM', 'command',
                                  'Print out ClassicFSM layout',
                                  label = 'Print ClassicFSM layout',
                                  command = self.printLayout)

        # States Menu
        menuBar.addmenu('States', 'State Inspector Operations')
        menuBar.addcascademenu('States', 'Font Size',
                                     'Set state label size', tearoff = 1)
        for size in (8, 10, 12, 14, 18, 24):
            menuBar.addmenuitem('Font Size', 'command',
                'Set font to: ' + repr(size) + ' Pts', label = repr(size) + ' Pts',
                command = lambda s = self, sz = size: s.setFontSize(sz))
        menuBar.addcascademenu('States', 'Marker Size',
                                     'Set state marker size', tearoff = 1)
        for size in ('Small', 'Medium', 'Large'):
            sizeDict = {'Small': '0.25i', 'Medium': '0.375i', 'Large': '0.5i'}
            menuBar.addmenuitem('Marker Size', 'command',
                size + ' markers', label = size + ' Markers',
                command = lambda s = self, sz = size, d = sizeDict:
                    s.setMarkerSize(d[sz]))

        # The Scrolled Canvas
        self._scrolledCanvas = self.createcomponent('scrolledCanvas',
                (), None,
                Pmw.ScrolledCanvas, (interior,),
                hull_width = 400, hull_height = 400,
                usehullsize = 1)
        self._canvas = self._scrolledCanvas.component('canvas')
        self._canvas['scrollregion'] = ('-2i', '-2i', '2i', '2i')
        self._scrolledCanvas.resizescrollregion()
        self._scrolledCanvas.pack(padx = 5, pady = 5, expand=1, fill = BOTH)

        # Update lines
        self._canvas.bind('<B1-Motion>', self.drawConnections)
        self._canvas.bind('<ButtonPress-2>', self.mouse2Down)
        self._canvas.bind('<B2-Motion>', self.mouse2Motion)
        self._canvas.bind('<Configure>',
                          lambda e, sc = self._scrolledCanvas:
                          sc.resizescrollregion())

        self.createStateInspectors()

        self.initialiseoptions(FSMInspector)

    def scrolledCanvas(self):
        return self._scrolledCanvas

    def canvas(self):
        return self._canvas

    def setFontSize(self, size):
        self._canvas.itemconfigure('labels', font = ('MS Sans Serif', size))

    def setMarkerSize(self, size):
        for key in self.stateInspectorDict:
            self.stateInspectorDict[key].setRadius(size)
        self.drawConnections()

    def drawConnections(self, event = None):
        # Get rid of existing arrows
        self._canvas.delete('arrow')
        for key in self.stateInspectorDict:
            si = self.stateInspectorDict[key]
            state = si.state
            if state.getTransitions():
                for name in state.getTransitions():
                    self.connectStates(si, self.getStateInspector(name))

    def connectStates(self, fromState, toState):
        endpts = self.computeEndpoints(fromState, toState)
        line = self._canvas.create_line(endpts, tags = ('arrow',),
                                        arrow = 'last')

    def computeEndpoints(self, fromState, toState):
        # Compute angle between two points
        fromCenter = fromState.center()
        toCenter = toState.center()
        angle = self.findAngle(fromCenter, toCenter)

        # Compute offset fromState point
        newFromPt = map(operator.__add__,
                        fromCenter,
                        self.computePoint(fromState.radius,
                                           angle + DELTA))

        # Compute offset toState point
        newToPt = map(operator.__sub__,
                      toCenter,
                      self.computePoint(toState.radius,
                                         angle - DELTA))
        return list(newFromPt) + list(newToPt)

    def computePoint(self, radius, angle):
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        return (x, y)

    def findAngle(self, fromPoint, toPoint):
        dx = toPoint[0] - fromPoint[0]
        dy = toPoint[1] - fromPoint[1]
        return math.atan2(dy, dx)

    def mouse2Down(self, event):
        self._width = 1.0 * self._canvas.winfo_width()
        self._height = 1.0 * self._canvas.winfo_height()
        xview = self._canvas.xview()
        yview = self._canvas.yview()
        self._left = xview[0]
        self._top = yview[0]
        self._dxview = xview[1] - xview[0]
        self._dyview = yview[1] - yview[0]
        self._2lx = event.x
        self._2ly = event.y

    def mouse2Motion(self, event):
        newx = self._left - ((event.x - self._2lx)/self._width) * self._dxview
        self._canvas.xview_moveto(newx)
        newy = self._top - ((event.y - self._2ly)/self._height) * self._dyview
        self._canvas.yview_moveto(newy)
        self._2lx = event.x
        self._2ly = event.y
        self._left = self._canvas.xview()[0]
        self._top = self._canvas.yview()[0]

    def createStateInspectors(self):
        fsm = self.fsm
        self.states = fsm.getStates()
        # Number of rows/cols needed to fit inspectors in a grid
        dim = int(math.ceil(math.sqrt(len(self.states))))
        # Separation between nodes
        spacing = 2.5 * self._canvas.canvasx('0.375i')
        count = 0
        for state in self.states:
            si = self.addState(state)
            if state.getInspectorPos():
                si.setPos(state.getInspectorPos()[0],
                          state.getInspectorPos()[1])
            else:
                row = int(math.floor(count / dim))
                col = count % dim
                si.setPos(col * spacing, row * spacing +
                          0.5 * (0, spacing)[col % 2])
            # Add hooks
            self.accept(self.name + '_' + si.getName() + '_entered',
                        si.enteredState)
            self.accept(self.name + '_' + si.getName() + '_exited',
                        si.exitedState)
            count = count + 1
        self.drawConnections()
        if fsm.getCurrentState():
            self.enteredState(fsm.getCurrentState().getName())

    def getStateInspector(self, name):
        return self.stateInspectorDict.get(name, None)

    def addState(self, state):
        si = self.stateInspectorDict[state.getName()] = (
            StateInspector(self, state))
        return si

    def enteredState(self, stateName):
        si = self.stateInspectorDict.get(stateName, None)
        if si:
            si.enteredState()

    def exitedState(self, stateName):
        si = self.stateInspectorDict.get(stateName, None)
        if si:
            si.exitedState()

    def _setGridSize(self):
        self._gridSize = self['gridSize']
        self.setGridSize(self._gridSize)

    def setGridSize(self, size):
        for key in self.stateInspectorDict:
            self.stateInspectorDict[key].setGridSize(size)

    def popupGridDialog(self):
        spacing = askstring('ClassicFSM Grid Spacing', 'Grid Spacing:')
        if spacing:
            self.setGridSize(spacing)
            self._gridSize = spacing

    def toggleGridSnap(self):
        if self._fGridSnap.get():
            self.setGridSize(self._gridSize)
        else:
            self.setGridSize(0)

    def printLayout(self):
        dict = self.stateInspectorDict
        keys = list(dict.keys())
        keys.sort()
        print("ClassicFSM.ClassicFSM('%s', [" % self.name)
        for key in keys[:-1]:
            si = dict[key]
            center = si.center()
            print("    State.State('%s'," % si.state.getName())
            print("                %s," % si.state.getEnterFunc().__name__)
            print("                %s," % si.state.getExitFunc().__name__)
            print("                %s," % si.state.getTransitions())
            print("                inspectorPos = [%.1f, %.1f])," % (center[0], center[1]))
        for key in keys[-1:]:
            si = dict[key]
            center = si.center()
            print("    State.State('%s'," % si.state.getName())
            print("                %s," % si.state.getEnterFunc().__name__)
            print("                %s," % si.state.getExitFunc().__name__)
            print("                %s," % si.state.getTransitions())
            print("                inspectorPos = [%.1f, %.1f])]," % (center[0], center[1]))
        print("        '%s'," % self.fsm.getInitialState().getName())
        print("        '%s')" % self.fsm.getFinalState().getName())

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.balloon.configure(state = 'balloon')
        else:
            self.balloon.configure(state = 'none')

    def onDestroy(self, event):
        """ Called on ClassicFSM Panel shutdown """
        self.fsm.inspecting = 0
        for si in self.stateInspectorDict.values():
            self.ignore(self.name + '_' + si.getName() + '_entered')
            self.ignore(self.name + '_' + si.getName() + '_exited')

class StateInspector(Pmw.MegaArchetype):
    def __init__(self, inspector, state, **kw):

        # Record inspector and state
        self.inspector = inspector
        self.state = state
        # Create a unique tag which you can use to move a marker and
        # and its corresponding text around together
        self.tag = state.getName()
        self.fsm = inspector.fsm

        # Pointers to the inspector's components
        self.scrolledCanvas = inspector.component('scrolledCanvas')
        self._canvas = self.scrolledCanvas.component('canvas')

        #define the megawidget options
        optiondefs = (
            ('radius', '0.375i', self._setRadius),
            ('gridSize', '0.25i', self._setGridSize),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the parent class
        Pmw.MegaArchetype.__init__(self)

        # Draw the oval
        self.x = 0
        self.y = 0
        half = self._canvas.winfo_fpixels(self['radius'])
        self.marker = self._canvas.create_oval((self.x - half),
                                               (self.y - half),
                                               (self.x + half),
                                               (self.y + half),
                                              fill = 'CornflowerBlue',
                                              tags = (self.tag,'markers'))
        self.text = self._canvas.create_text(0, 0, text = state.getName(),
                                           justify = CENTER,
                                           tags = (self.tag,'labels'))
        # Is this state contain a sub machine?
        if state.hasChildren():
            # reduce half by sqrt of 2.0
            half = half * 0.707106
            self.rect = self._canvas.create_rectangle((- half), (- half),
                                                     half, half,
                                                     tags = (self.tag,))


        # The Popup State Menu
        self._popupMenu = Menu(self._canvas, tearoff = 0)
        self._popupMenu.add_command(label = 'Request transition to ' +
                                    state.getName(),
                                    command = self.transitionTo)
        if state.hasChildren():
            self._popupMenu.add_command(label = 'Inspect ' + state.getName() +
                                        ' submachine',
                                        command = self.inspectSubMachine)

        self.scrolledCanvas.resizescrollregion()

        # Add bindings
        self._canvas.tag_bind(self.tag, '<Enter>', self.mouseEnter)
        self._canvas.tag_bind(self.tag, '<Leave>', self.mouseLeave)
        self._canvas.tag_bind(self.tag, '<ButtonPress-1>', self.mouseDown)
        self._canvas.tag_bind(self.tag, '<B1-Motion>', self.mouseMotion)
        self._canvas.tag_bind(self.tag, '<ButtonRelease-1>', self.mouseRelease)
        self._canvas.tag_bind(self.tag, '<ButtonPress-3>', self.popupStateMenu)

        self.initialiseoptions(StateInspector)

    # Utility methods
    def _setRadius(self):
        self.setRadius(self['radius'])

    def setRadius(self, size):
        half = self.radius = self._canvas.winfo_fpixels(size)
        c = self.center()
        self._canvas.coords(self.marker,
                            c[0] - half, c[1] - half, c[0] + half, c[1] + half)
        if self.state.hasChildren():
            half = self.radius * 0.707106
            self._canvas.coords(self.rect,
                            c[0] - half, c[1] - half, c[0] + half, c[1] + half)

    def _setGridSize(self):
        self.setGridSize(self['gridSize'])

    def setGridSize(self, size):
        self.gridSize = self._canvas.winfo_fpixels(size)
        if self.gridSize == 0:
            self.fGridSnap = 0
        else:
            self.fGridSnap = 1

    def setText(self, text = None):
        self._canvas.itemconfigure(self.text, text = text)

    def setPos(self, x, y, snapToGrid = 0):
        if self.fGridSnap:
            self.x = round(x / self.gridSize) * self.gridSize
            self.y = round(y / self.gridSize) * self.gridSize
        else:
            self.x = x
            self.y = y
        # How far do we have to move?
        cx, cy = self.center()
        self._canvas.move(self.tag, self.x - cx, self.y - cy)

    def center(self):
        c = self._canvas.coords(self.marker)
        return (c[0] + c[2])/2.0, (c[1] + c[3])/2.0

    def getName(self):
        return self.tag

    # Event Handlers
    def mouseEnter(self, event):
        self._canvas.itemconfig(self.marker, width = 2)

    def mouseLeave(self, event):
        self._canvas.itemconfig(self.marker, width = 1)

    def mouseDown(self, event):
        self._canvas.lift(self.tag)
        self.startx, self.starty = self.center()
        self.lastx = self._canvas.canvasx(event.x)
        self.lasty = self._canvas.canvasy(event.y)

    def mouseMotion(self, event):
        dx = self._canvas.canvasx(event.x) - self.lastx
        dy = self._canvas.canvasy(event.y) - self.lasty
        newx, newy = map(operator.__add__, (self.startx, self.starty), (dx, dy))
        self.setPos(newx, newy)

    def mouseRelease(self, event):
        self.scrolledCanvas.resizescrollregion()

    def popupStateMenu(self, event):
        self._popupMenu.post(event.widget.winfo_pointerx(),
                             event.widget.winfo_pointery())

    def transitionTo(self):
        self.fsm.request(self.getName())

    def inspectSubMachine(self):
        print('inspect ' + self.tag + ' subMachine')
        for childFSM in self.state.getChildren():
            FSMInspector(childFSM)

    def enteredState(self):
        self._canvas.itemconfigure(self.marker, fill = 'Red')

    def exitedState(self):
        self._canvas.itemconfigure(self.marker, fill = 'CornflowerBlue')
