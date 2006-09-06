"""
AppShell provides a GUI application framework.
This is an adaption of AppShell.py found in Python and Tkinter Programming
by John E. Grayson which is a streamlined adaptation of GuiAppD.py, originally
created by Doug Hellmann (doughellmann@mindspring.com).
"""

__all__ = ['AppShell']

from direct.showbase.DirectObject import DirectObject
from direct.showbase.TkGlobal import *
from tkFileDialog import *
from Tkinter import *
import Pmw
import Dial
import Floater
import Slider
import EntryScale
import VectorWidgets
import sys, string
import ProgressBar

"""
TO FIX:
Radiobutton ordering change
"""

# Create toplevel widget dictionary
try:
    __builtins__["widgetDict"]
except KeyError:
    __builtins__["widgetDict"] = {}
# Create toplevel variable dictionary
try:
    __builtins__["variableDict"]
except KeyError:
    __builtins__["variableDict"] = {}

def resetWidgetDict():
    __builtins__["widgetDict"] = {}
def resetVariableDict():
    __builtins__["variableDict"] = {}

# Inherit from MegaWidget instead of Toplevel so you can pass in a toplevel
# to use as a container if you wish.  If no toplevel passed in, create one
class AppShell(Pmw.MegaWidget, DirectObject):
    appversion      = '1.0'
    appname         = 'Generic Application Frame'
    copyright       = ('Copyright 2004 Walt Disney Imagineering.' +
                       ' All Rights Reserved')
    contactname     = 'Mark R. Mine'
    contactphone    = '(818) 544-2921'
    contactemail    = 'Mark.Mine@disney.com'

    frameWidth      = 450
    frameHeight     = 320
    padx            = 5
    pady            = 5
    usecommandarea  = 0
    usestatusarea   = 0
    balloonState    = 'none'
    panelCount      = 0

    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('title',          self.appname,        None),
            ('padx',           1,                   Pmw.INITOPT),
            ('pady',           1,                   Pmw.INITOPT),
            ('framewidth',     self.frameWidth,     Pmw.INITOPT),
            ('frameheight',    self.frameHeight,    Pmw.INITOPT),
            ('usecommandarea', self.usecommandarea, Pmw.INITOPT),
            ('usestatusarea',  self.usestatusarea,  Pmw.INITOPT),
            )
        self.defineoptions(kw, optiondefs)
        # If no toplevel passed in, create one
        if parent == None:
            self.parent = Toplevel()
        else:
            self.parent = parent
        # Initialize the base class
        Pmw.MegaWidget.__init__(self, self.parent)
        # Set window size
        self.parent.geometry('%dx%d' % (self.frameWidth, self.frameHeight))
        self.parent.title(self['title'])
        # Create unique id
        AppShell.panelCount += 1
        self.id = self.appname + '-' + `AppShell.panelCount`
        # Create a dictionary in the widgetDict to hold this panel's widgets
        self.widgetDict = widgetDict[self.id] = {}
        # And one to hold this panel's variables
        self.variableDict = variableDict[self.id] = {}
        # Get handle to the toplevels hull
        self._hull = self.component('hull')
        # Initialize the application
        self.appInit()
        # create the interface
        self.__createInterface()
        # Set focus to ourselves
        self.focus_set()
        # initialize our options
        self.initialiseoptions(AppShell)

        self.pack(fill = BOTH, expand = 1)

    def __createInterface(self):
        self.__createBalloon()
        self.__createMenuBar()
        self.__createDataArea()
        self.__createCommandArea()
        self.__createMessageBar()
        self.__createAboutBox()
        # Add binding for panel cleanup code
        self.interior().bind('<Destroy>', self.onDestroy)
        #
        # Create the parts of the interface
        # which can be modified by subclasses
        #
        self.createMenuBar()
        self.createInterface()

    def __createBalloon(self):
        # Create the balloon help manager for the frame.
        # Create the manager for the balloon help
        self.__balloon = self.createcomponent('balloon', (), None,
                                              Pmw.Balloon, (self._hull,))
        self.__balloon.configure(state = self.balloonState)

    def __createMenuBar(self):
        self.menuFrame = Frame(self._hull)
        self.menuBar = self.createcomponent('menubar', (), None,
                                            Pmw.MenuBar,
                                            (self.menuFrame,),
                                            hull_relief=FLAT,
                                            hull_borderwidth=0,
                                            balloon=self.balloon())

        self.menuBar.addmenu('Help', 'About %s' % self.appname, side = 'right')
        self.menuBar.addmenu('File', 'File commands and Quit')
        self.menuBar.pack(fill=X, side = LEFT)

        # Force some space between pull down menus and other widgets
        spacer = Label(self.menuFrame, text = '   ')
        spacer.pack(side = LEFT, expand = 0)

        self.menuFrame.pack(fill = X)

    def __createDataArea(self):
        # Create data area where data entry widgets are placed.
        self.dataArea = self.createcomponent('dataarea',
                                             (), None,
                                             Frame, (self._hull,),
                                             relief=GROOVE,
                                             bd=1)
        self.dataArea.pack(side=TOP, fill=BOTH, expand=YES,
                           padx=self['padx'], pady=self['pady'])

    def __createCommandArea(self):
        # Create a command area for application-wide buttons.
        self.__commandFrame = self.createcomponent('commandframe', (), None,
                                                   Frame,
                                                   (self._hull,),
                                                   relief=SUNKEN,
                                                   bd=1)
        self.__buttonBox = self.createcomponent('buttonbox', (), None,
                                                Pmw.ButtonBox,
                                                (self.__commandFrame,),
                                                padx=0, pady=0)
        self.__buttonBox.pack(side=TOP, expand=NO, fill=X)
        if self['usecommandarea']:
            self.__commandFrame.pack(side=TOP,
                                     expand=NO,
                                     fill=X,
                                     padx=self['padx'],
                                     pady=self['pady'])


    def __createMessageBar(self):
        # Create the message bar area for help and status messages.
        frame = self.createcomponent('bottomtray', (), None,
                                     Frame, (self._hull,), relief=SUNKEN)
        self.__messageBar = self.createcomponent('messagebar',
                                                  (), None,
                                                 Pmw.MessageBar,
                                                 (frame,),
                                                 #entry_width = 40,
                                                 entry_relief=SUNKEN,
                                                 entry_bd=1,
                                                 labelpos=None)
        self.__messageBar.pack(side=LEFT, expand=YES, fill=X)

        self.__progressBar = ProgressBar.ProgressBar(
            frame,
            fillColor='slateblue',
            doLabel=1,
            width=150)
        self.__progressBar.frame.pack(side=LEFT, expand=NO, fill=NONE)

        self.updateProgress(0)
        if self['usestatusarea']:
            frame.pack(side=BOTTOM, expand=NO, fill=X)

        self.__balloon.configure(statuscommand = \
                                 self.__messageBar.helpmessage)

    def __createAboutBox(self):
        Pmw.aboutversion(self.appversion)
        Pmw.aboutcopyright(self.copyright)
        Pmw.aboutcontact(
          'For more information, contact:\n %s\n Phone: %s\n Email: %s' %\
                      (self.contactname, self.contactphone,
                       self.contactemail))
        self.about = Pmw.AboutDialog(self._hull,
                                     applicationname=self.appname)
        self.about.withdraw()

    def toggleBalloon(self):
        if self.toggleBalloonVar.get():
            self.__balloon.configure(state = 'both')
        else:
            self.__balloon.configure(state = 'status')

    def showAbout(self):
        # Create the dialog to display about and contact information.
        self.about.show()
        self.about.focus_set()

    def quit(self):
        self.parent.destroy()

    ### USER METHODS ###
    # To be overridden
    def appInit(self):
        # Called before interface is created (should be overridden).
        pass

    def createInterface(self):
        # Override this method to create the interface for the app.
        pass

    def onDestroy(self, event):
        # Override this method with actions to be performed on panel shutdown
        pass

    def createMenuBar(self):
        # Creates default menus.  Can be overridden or simply augmented
        # Using button Add below
        self.menuBar.addmenuitem('Help', 'command',
                                 'Get information on application',
                                 label='About...', command=self.showAbout)
        self.toggleBalloonVar = IntVar()
        if self.balloonState == 'none':
            self.toggleBalloonVar.set(0)
        else:
            self.toggleBalloonVar.set(1)
        self.menuBar.addmenuitem('Help', 'checkbutton',
                                 'Toggle balloon help',
                                 label='Balloon help',
                                 variable = self.toggleBalloonVar,
                                 command=self.toggleBalloon)

        self.menuBar.addmenuitem('File', 'command', 'Quit this application',
                                label='Quit',
                                command=self.quit)

    # Getters
    def interior(self):
        # Retrieve the interior site where widgets should go.
        return self.dataArea

    def balloon(self):
        # Retrieve the panel's balloon widget
        return self.__balloon

    def buttonBox(self):
        # Retrieve the button box.
        return self.__buttonBox

    def messageBar(self):
        # Retieve the message bar
        return self.__messageBar

    # Utility functions
    def buttonAdd(self, buttonName, helpMessage=None,
                  statusMessage=None, **kw):
        # Add a button to the button box.
        newBtn = self.__buttonBox.add(buttonName)
        newBtn.configure(kw)
        if helpMessage:
             self.bind(newBtn, helpMessage, statusMessage)
        return newBtn

    def alignbuttons(self):
        """ Make all buttons wide as widest """
        self.__buttonBox.alignbuttons()

    def bind(self, child, balloonHelpMsg, statusHelpMsg=None):
        # Bind a help message and/or status message to a widget.
        self.__balloon.bind(child, balloonHelpMsg, statusHelpMsg)

    def updateProgress(self, newValue=0, newMax=0):
        # Used to update progress bar
        self.__progressBar.updateProgress(newValue, newMax)

    ## WIDGET UTILITY FUNCTIONS ##
    def addWidget(self, category, text, widget):
        self.widgetDict[category + '-' + text] = widget

    def getWidget(self, category, text):
        return self.widgetDict.get(category + '-' + text, None)

    def addVariable(self, category, text, variable):
        self.variableDict[category + '-' + text] = variable

    def getVariable(self, category, text):
        return self.variableDict.get(category + '-' + text, None)

    def createWidget(self, parent, category, text, widgetClass,
                     help, command, side, fill, expand, kw):
        # Update kw to reflect user inputs
        kw['text'] = text
        # Create widget
        widget = apply(widgetClass, (parent,), kw)
        # Do this after so command isn't called on widget creation
        widget['command'] = command
        # Pack widget
        widget.pack(side = side, fill = fill, expand = expand)
        # Bind help
        self.bind(widget, help)
        # Record widget
        self.addWidget(category, text, widget)
        return widget

    def newCreateLabeledEntry(self, parent, category, text, help = '',
                              command = None, value = '',
                              width = 12, relief = SUNKEN,
                              side = LEFT, fill = X, expand = 0):
        """ createLabeledEntry(parent, category, text, [options]) """
        # Create labeled entry
        frame = Frame(parent)
        variable = StringVar()
        variable.set(value)
        label = Label(frame, text = text)
        label.pack(side = LEFT, fill = X, expand = 0)
        entry = Entry(frame, width = width, relief = relief,
                      textvariable = variable)
        entry.pack(side = LEFT, fill = X, expand = 1)
        frame.pack(side = side, fill = X, expand = expand)
        if command:
            entry.bind('<Return>', command)
        # Add balloon help
        self.bind(label, help)
        self.bind(entry, help)
        # Record widgets and variable
        self.addWidget(category, text, entry)
        self.addWidget(category, text + '-Label', label)
        self.addVariable(category, text, variable)
        return entry

    def newCreateButton(self, parent, category, text,
                        help = '', command = None,
                        side = LEFT, fill = X, expand = 0, **kw):
        """ createButton(parent, category, text, [options]) """
        # Create the widget
        widget = self.createWidget(parent, category, text, Button,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateCheckbutton(self, parent, category, text,
                             help = '', command = None,
                             initialState = 0, anchor = W,
                             side = LEFT, fill = X, expand = 0, **kw):
        """ createCheckbutton(parent, category, text, [options]) """
        # Create the widget
        widget = self.createWidget(parent, category, text, Checkbutton,
                                   help, command, side, fill, expand, kw)
        # Perform extra customization
        widget['anchor'] = anchor
        variable = BooleanVar()
        variable.set(initialState)
        self.addVariable(category, text, variable)
        widget['variable'] = variable
        return widget

    def newCreateRadiobutton(self, parent, category, text, variable, value,
                             command = None, help = '', anchor = W,
                             side = LEFT, fill = X, expand = 0, **kw):
        """
        createRadiobutton(parent, category, text, variable, value, [options])
        """
        # Create the widget
        widget = self.createWidget(parent, category, text, Radiobutton,
                                   help, command, side, fill, expand, kw)
        # Perform extra customization
        widget['anchor'] = anchor
        widget['value'] = value
        widget['variable'] = variable
        return widget

    def newCreateFloater(self, parent, category, text,
                         help = '', command = None,
                         side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   Floater.Floater,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateDial(self, parent, category, text,
                      help = '', command = None,
                      side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   Dial.Dial,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateSider(self, parent, category, text,
                       help = '', command = None,
                       side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   Slider.Slider,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateEntryScale(self, parent, category, text,
                            help = '', command = None,
                            side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   EntryScale.EntryScale,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateVector2Entry(self, parent, category, text,
                              help = '', command = None,
                              side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   VectorWidgets.Vector2Entry,
                                   help, command, side, fill, expand, kw)

    def newCreateVector3Entry(self, parent, category, text,
                              help = '', command = None,
                              side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   VectorWidgets.Vector3Entry,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateColorEntry(self, parent, category, text,
                            help = '', command = None,
                            side = LEFT, fill = X, expand = 0, **kw):
        # Create the widget
        widget = self.createWidget(parent, category, text,
                                   VectorWidgets.ColorEntry,
                                   help, command, side, fill, expand, kw)
        return widget

    def newCreateOptionMenu(self, parent, category, text,
                            help = '', command = None, items = [],
                            labelpos = W, label_anchor = W,
                            label_width = 16, menu_tearoff = 1,
                            side = LEFT, fill = X, expand = 0, **kw):
        # Create variable
        variable = StringVar()
        if len(items) > 0:
            variable.set(items[0])
        # Update kw to reflect user inputs
        kw['items'] = items
        kw['label_text'] = text
        kw['labelpos'] = labelpos
        kw['label_anchor'] = label_anchor
        kw['label_width'] = label_width
        kw['menu_tearoff'] = menu_tearoff
        kw['menubutton_textvariable'] = variable
        # Create widget
        widget = apply(Pmw.OptionMenu, (parent,), kw)
        # Do this after so command isn't called on widget creation
        widget['command'] = command
        # Pack widget
        widget.pack(side = side, fill = fill, expand = expand)
        # Bind help
        self.bind(widget.component('menubutton'), help)
        # Record widget and variable
        self.addWidget(category, text, widget)
        self.addVariable(category, text, variable)
        return widget

    def newCreateComboBox(self, parent, category, text,
                          help = '', command = None,
                          items = [], state = DISABLED, history = 0,
                          labelpos = W, label_anchor = W,
                          label_width = 16, entry_width = 16,
                          side = LEFT, fill = X, expand = 0, **kw):
        # Update kw to reflect user inputs
        kw['label_text'] = text
        kw['labelpos'] = labelpos
        kw['label_anchor'] = label_anchor
        kw['label_width'] = label_width
        kw['entry_width'] = entry_width
        kw['scrolledlist_items'] = items
        kw['entryfield_entry_state'] = state
        # Create widget
        widget = apply(Pmw.ComboBox, (parent,), kw)
        # Bind selection command
        widget['selectioncommand'] = command
        # Select first item if it exists
        if len(items) > 0:
            widget.selectitem(items[0])
        # Pack widget
        widget.pack(side = side, fill = fill, expand = expand)
        # Bind help
        self.bind(widget, help)
        # Record widget
        self.addWidget(category, text, widget)
        return widget
    def transformRGB(self, rgb, max = 1.0):
        retval = '#'
        for v in [rgb[0], rgb[1], rgb[2]]:
            v = (v/max)*255
            if v > 255:
                v = 255
            if v < 0:
                v = 0
            retval = "%s%02x" % (retval, int(v))
        return retval

class TestAppShell(AppShell):
    # Override class variables here
    appname = 'Test Application Shell'
    usecommandarea = 1
    usestatusarea  = 1

    def __init__(self, parent = None, **kw):
        # Call superclass initialization function
        AppShell.__init__(self)
        self.initialiseoptions(TestAppShell)

    def createButtons(self):
        self.buttonAdd('Ok',
                       helpMessage='Exit',
                       statusMessage='Exit',
                       command=self.quit)

    def createMain(self):
        self.label = self.createcomponent('label', (), None,
                                          Label,
                                          (self.interior(),),
                                          text='Data Area')
        self.label.pack()
        self.bind(self.label, 'Space taker')

    def createInterface(self):
        self.createButtons()
        self.createMain()

if __name__ == '__main__':
    test = TestAppShell(balloon_state='none')

