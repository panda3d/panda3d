"""
AppShell provides a GUI application framework.
This is an adaption of AppShell.py found in Python and Tkinter Programming
by John E. Grayson which is a streamlined adaptation of GuiAppD.py, originally
created by Doug Hellmann (doughellmann@mindspring.com).
"""

from PandaObject import *
from Tkinter import *
import Pmw
import sys, string
import ProgressBar

# Inherit from MegaWidget instead of Toplevel so you can pass in a toplevel
# to use as a container if you wish.  If no toplevel passed in, create one
class AppShell(Pmw.MegaWidget, PandaObject):
    appversion      = '1.0'
    appname         = 'Generic Application Frame'
    copyright       = ('Copyright 2001 Walt Disney Imagineering.' +
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
                                     Frame,(self._hull,), relief=SUNKEN)
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
        return None
       
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

