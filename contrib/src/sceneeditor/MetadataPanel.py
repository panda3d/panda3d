from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *
import Pmw

class MetadataPanel(AppShell,Pmw.MegaWidget):
    appversion      = '1.0'
    appname         = 'Metadata Panel'
    frameWidth      = 400
    frameHeight     = 400
    padx            = 0
    pady            = 0
    usecommandarea  = 0
    usestatusarea   = 0
    Metatag=""
    Metanode=None
    tag_text=None
    def __init__(self,nodePath,parent=None,**kw):

        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Define the megawidget options.
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)
        self.Metanode=nodePath
        if(nodePath.hasTag("Metadata")):
            self.Metatag=self.Metanode.getTag("Metadata")

        if parent == None:
            self.parent = Toplevel()
        AppShell.__init__(self, self.parent)

        self.parent.resizable(False,False)


    def appInit(self):
        print("Metadata Panel")

    def createInterface(self):
        interior = self.interior()
        mainFrame = Frame(interior)
        tag_label=Label (mainFrame,text="Enter Metadata",font=('MSSansSerif', 15),
                           relief = RIDGE, borderwidth=5)
        tag_label.pack()
        source=StringVar()
        source.set(self.Metatag)
        self.tag_text=Entry(mainFrame, width=10,textvariable=source)
        self.tag_text.pack()
        set_button=Button(mainFrame, text='Set Metadata',font=('MSSansSerif', 15),
                           relief = RIDGE, borderwidth=5, command= lambda:self.SetIt())
        set_button.pack()
        mainFrame.pack(fill = 'both', expand = 1)

    def SetIt(self):
        self.Metanode.setTag("Metadata",self.tag_text.get())


