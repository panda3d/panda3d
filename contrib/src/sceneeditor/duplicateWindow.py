#################################################################
# duplicateWindow.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *

import seSceneGraphExplorer


class duplicateWindow(AppShell):
    #################################################################
    # duplicateWindow(AppShell)
    # This class is used to create a dialog window
    # for handling the "dupicate" command from the sceneEditor
    #
    # Notice!
    # The actual duplicating process is not happending here!
    # The one handle the process is dataHolder...
    #################################################################
    appversion      = '1.0'
    appname         = 'Duplication'
    frameWidth      = 450
    frameHeight     = 320
    frameIniPosX    = 250
    frameIniPosY    = 250
    padx            = 0
    pady            = 0


    def __init__(self, parent = None, nodePath = None, **kw):
        # Define the megawidget options.
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        if parent == None:
            self.parent = Toplevel()
        AppShell.__init__(self, self.parent)
        self.parent.geometry('%dx%d+%d+%d' % (self.frameWidth, self.frameHeight,self.frameIniPosX,self.frameIniPosY))

        self.nodePath = nodePath

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def appInit(self):
        print('----SideWindow is Initialized!!')

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        mainFrame = Frame(interior)
        self.inputZone = Pmw.Group(mainFrame, tag_text='Offset setting')
        self.inputZone.pack(fill='both',expand=1)
        settingFrame = self.inputZone.interior()
        Label(settingFrame,text='  X  ').place(anchor=NW,x=110,y=15)
        Label(settingFrame,text='  Y  ').place(anchor=NW,x=205,y=15)
        Label(settingFrame,text='  Z  ').place(anchor=NW,x=295,y=15)
        self.move_x = Pmw.EntryField(settingFrame,label_text='Move  :',labelpos='w',value='0.0', validate=Pmw.realvalidator)
        self.move_x.component('entry').config(width=10)
        self.move_y = Pmw.EntryField(settingFrame,value='0.0', validate=Pmw.realvalidator)
        self.move_y.component('entry').config(width=10)
        self.move_z = Pmw.EntryField(settingFrame, value='0.0', validate=Pmw.realvalidator)
        self.move_z.component('entry').config(width=10)
        self.move_x.place(anchor=NW,x=50,y=40)
        self.move_y.place(anchor=NW,x=185,y=40)
        self.move_z.place(anchor=NW,x=275,y=40)

        self.rotate_x = Pmw.EntryField(settingFrame,label_text='Rotate:',labelpos='w',value='0.0', validate=Pmw.realvalidator)
        self.rotate_x.component('entry').config(width=10)
        self.rotate_y = Pmw.EntryField(settingFrame,value='0.0', validate=Pmw.realvalidator)
        self.rotate_y.component('entry').config(width=10)
        self.rotate_z = Pmw.EntryField(settingFrame, value='0.0', validate=Pmw.realvalidator)
        self.rotate_z.component('entry').config(width=10)
        self.rotate_x.place(anchor=NW,x=50,y=70)
        self.rotate_y.place(anchor=NW,x=185,y=70)
        self.rotate_z.place(anchor=NW,x=275,y=70)

        self.scale_x = Pmw.EntryField(settingFrame,label_text='Scale :',labelpos='w',value='1.0', validate=Pmw.realvalidator)
        self.scale_x.component('entry').config(width=10)
        self.scale_y = Pmw.EntryField(settingFrame,value='1.0', validate=Pmw.realvalidator)
        self.scale_y.component('entry').config(width=10)
        self.scale_z = Pmw.EntryField(settingFrame, value='1.0', validate=Pmw.realvalidator)
        self.scale_z.component('entry').config(width=10)
        self.scale_x.place(anchor=NW,x=52,y=100)
        self.scale_y.place(anchor=NW,x=185,y=100)
        self.scale_z.place(anchor=NW,x=275,y=100)

        self.numberOfCopy = Pmw.EntryField(settingFrame,label_text='Number of Copy :',labelpos='w',value='1', validate=Pmw.integervalidator)
        self.numberOfCopy.component('entry').config(width=15)
        self.numberOfCopy.place(anchor=NW,x=52,y=150)

        settingFrame.pack(fill=BOTH,expand=1,padx=7,pady=7)

        self.button_ok = Button(mainFrame, text="OK", command=self.ok_press,width=10)
        self.button_ok.pack(fill=BOTH,expand=0,side=RIGHT)

        mainFrame.pack(fill = 'both', expand = 1,padx=7,pady=7)


    def createMenuBar(self):
        self.menuBar.destroy()

    def onDestroy(self, event):
        messenger.send('DW_close')
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
        # After collect all data we need for the duplication process,
        # this function will send out a message with all data.
        # 'DW_duplicating'
        # This message will be caught by sceneEditor.
        #################################################################
        if not self.allEntryValid():
            print('---- Duplication Window: Invalid value!!')
            return
        x = self.move_x.getvalue()
        y = self.move_y.getvalue()
        z = self.move_z.getvalue()
        pos=Vec3(FloatType(x),FloatType(y),FloatType(z))
        x = self.rotate_x.getvalue()
        y = self.rotate_y.getvalue()
        z = self.rotate_z.getvalue()
        hpr=Vec3(FloatType(x),FloatType(y),FloatType(z))
        x = self.scale_x.getvalue()
        y = self.scale_y.getvalue()
        z = self.scale_z.getvalue()
        scale=Vec3(FloatType(x),FloatType(y),FloatType(z))
        num = int(self.numberOfCopy.getvalue())
        messenger.send('DW_duplicating',[self.nodePath,pos,hpr,scale,num])
        self.quit()

    def allEntryValid(self):
        #################################################################
        # allEntryValid(self)
        # This function is used to check all data in the input entries are valid.
        #
        # For example, none of entries contains blank data.
        #
        #################################################################
        if not self.move_x.valid():
            return False
        elif not self.move_y.valid():
            return False
        elif not self.move_z.valid():
            return False
        elif not self.rotate_x.valid():
            return False
        elif not self.rotate_y.valid():
            return False
        elif not self.rotate_z.valid():
            return False
        elif not self.scale_x.valid():
            return False
        elif not self.scale_y.valid():
            return False
        elif not self.scale_z.valid():
            return False
        elif not self.numberOfCopy.valid():
            return False

        return True
