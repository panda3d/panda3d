#################################################################
# AlignTool.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *


class AlignTool(AppShell):
    #################################################################
    # AlignTool(AppShell)
    #################################################################
    appversion      = '1.0'
    appname         = 'Align Tool'
    frameWidth      = 220
    frameHeight     = 330
    frameIniPosX    = 250
    frameIniPosY    = 250
    padx            = 0
    pady            = 0

    
    def __init__(self, list = [], parent = None, nodePath = None, **kw):
        # Keep nodePath Data
        self.nodePath = nodePath
        self.targetList = list
        self.targetName = None
        # Rename App
        self.appname += (' '+self.nodePath.getName())
        # Define the megawidget options.
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)
        if parent == None:
            self.parent = Toplevel()
        AppShell.__init__(self, self.parent)
        self.parent.geometry('%dx%d+%d+%d' % (self.frameWidth, self.frameHeight,self.frameIniPosX,self.frameIniPosY))
        
        self.initialiseoptions(AlignTool)
        
        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.
        
    def appInit(self):
        return
        
    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        mainFrame = Frame(interior)
        frame = Frame(mainFrame)
        self.nameBox = self.createcomponent(
            'Align Target', (), None,
            Pmw.ComboBox, (frame,),
            labelpos = W, label_text='Target Node:', entry_width = 20, entry_state = DISABLED,
            selectioncommand = self.setTargetNode,
            scrolledlist_items = self.targetList)
        self.nameBox.pack(side=LEFT)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)
        group = Pmw.Group(mainFrame, tag_text = 'Setting')
        group.pack(side=TOP, fill = 'both', expand = 1,pady=5)
        groupFrame = group.interior()
        # X and H checkbox
        frame = Frame(groupFrame)
        self.alignXVar = IntVar()
        self.alignXVar.set(False)
        self.alignXButton = Checkbutton(
            frame,
            text = ': Align X',
            variable = self.alignXVar)
        self.alignXButton.pack(side=LEFT, expand=False)
        self.alignHVar = IntVar()
        self.alignHVar.set(False)
        self.alignHButton = Checkbutton(
            frame,
            text = ': Align H',
            variable = self.alignHVar)
        self.alignHButton.pack(side=RIGHT, expand=False)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)
        
        groupFrame.pack(side=TOP, fill = 'both', expand = 1,padx=5,pady=5)
        
        frame = Frame(mainFrame)
        Button(frame, text='Align', width = 13, command=self.Align_press).pack(side=LEFT)
        Button(frame, text='OK', width = 13, command=self.ok_press).pack(side=RIGHT)
        frame.pack(side=BOTTOM, fill = X, expand = 1,pady=5)
        
        # Y and P checkbox
        frame = Frame(groupFrame)
        self.alignYVar = IntVar()
        self.alignYVar.set(False)
        self.alignYButton = Checkbutton(
            frame,
            text = ': Align Y',
            variable = self.alignYVar)
        self.alignYButton.pack(side=LEFT, expand=False)
        self.alignPVar = IntVar()
        self.alignPVar.set(False)
        self.alignPButton = Checkbutton(
            frame,
            text = ': Align P',
            variable = self.alignPVar)
        self.alignPButton.pack(side=RIGHT, expand=False)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)

        # Z and R checkbox
        frame = Frame(groupFrame)
        self.alignZVar = IntVar()
        self.alignZVar.set(False)
        self.alignZButton = Checkbutton(
            frame,
            text = ': Align Z',
            variable = self.alignZVar)
        self.alignZButton.pack(side=LEFT, expand=False)
        self.alignRVar = IntVar()
        self.alignRVar.set(False)
        self.alignRButton = Checkbutton(
            frame,
            text = ': Align R',
            variable = self.alignRVar)
        self.alignRButton.pack(side=RIGHT, expand=False)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)

        # Scale
        frame = Frame(groupFrame)
        Label(frame,text='Align Scale:').pack(side=LEFT)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)
        frame = Frame(groupFrame)
        self.alignSXVar = IntVar()
        self.alignSXVar.set(False)
        self.alignSXButton = Checkbutton(
            frame,
            text = ': X',
            variable = self.alignSXVar)
        self.alignSXButton.pack(side=LEFT, expand=False)
        self.alignSYVar = IntVar()
        self.alignSYVar.set(False)
        self.alignSYButton = Checkbutton(
            frame,
            text = ': Y',
            variable = self.alignSYVar)
        self.alignSYButton.pack(side=LEFT, expand=False)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)
        self.alignSZVar = IntVar()
        self.alignSZVar.set(False)
        self.alignSZButton = Checkbutton(
            frame,
            text = ': Z',
            variable = self.alignSZVar)
        self.alignSZButton.pack(side=LEFT, expand=False)
        frame.pack(side=TOP, fill = X, expand = 1,pady=5)
        
        
        mainFrame.pack(fill = 'both', expand = 1,padx=7,pady=7)

    
    def createMenuBar(self):
        self.menuBar.destroy()
        
    def onDestroy(self, event):
        messenger.send('ALW_close', [self.nodePath.getName()])
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

    ###############################
    
    def ok_press(self):
        #################################################################
        # ok_press(self)
        # Callback function
        # This function will be called when user click on the "OK" button on the window.
        #################################################################
        self.quit()

    def Align_press(self):
        list = [self.alignXVar.get(), self.alignYVar.get(), self.alignZVar.get(),
                self.alignHVar.get(), self.alignPVar.get(), self.alignRVar.get(),
                self.alignSXVar.get(), self.alignSYVar.get(), self.alignSZVar.get()]
        if self.targetName != None:
            messenger.send('ALW_align', [self.nodePath, self.targetName, list])
        return

    def setTargetNode(self,name=None):
        self.targetName = name
        return

