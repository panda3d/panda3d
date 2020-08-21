#################################################################
# collisionWindow.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
# Import Tkinter, Pmw, and the floater code from this directory tree.
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *
from seColorEntry import *
from direct.tkwidgets import VectorWidgets
from direct.tkwidgets import Floater
from direct.tkwidgets import Slider
import string, math, types
from panda3d.core import *


class collisionWindow(AppShell):
    #################################################################
    # This will open a talk window for user to set the collision object
    # In here, we won't finish the whole process to generate the
    # collision object, half of process will be finished by dataHolder.
    #################################################################
    # Override class variables
    appname = 'Creating Collision Object'
    frameWidth  = 600
    frameHeight = 300

    widgetsDict = {}

    # Define the types of collision we take care here
    collisionType = ['collisionPolygon',
                     'collisionSphere',
                     'collisionSegment',
                     'collisionRay']

    def __init__(self, nodePath, parent = None, **kw):

        self.nodePath = nodePath
        self.objType = 'collisionSphere' # set default type to Collision Sphere

        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',               self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass
        AppShell.__init__(self)

        # Execute option callbacks
        self.initialiseoptions(collisionWindow)

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def createInterface(self):
        # Handle to the toplevels interior
        interior = self.interior()
        menuBar = self.menuBar
        self.menuBar.destroy()

        # Create a frame to hold all stuff
        mainFrame = Frame(interior)

        frame = Frame(mainFrame)

        self.collisionTypeEntry = self.createcomponent(
            'Collision Type', (), None,
            Pmw.ComboBox, (frame,),
            labelpos = W, label_text='Collision Object Type:', entry_width = 20,
            selectioncommand = self.setObjectType,
            scrolledlist_items = self.collisionType)
        self.collisionTypeEntry.pack(side=LEFT, padx=3)

        label = Label(frame, text='Parent NodePath: '+ self.nodePath.getName(), font=('MSSansSerif', 12),
                      relief = RIDGE)
        label.pack(side=LEFT,expand=0,fill=X, padx=20)

        frame.pack(side=TOP, fill=X, expand=True, padx=3)
        self.collisionTypeEntry.selectitem('collisionSphere', setentry=True)

        self.inputZone = Pmw.Group(mainFrame, tag_pyclass = None)
        self.inputZone.pack(fill='both',expand=1)
        settingFrame = self.inputZone.interior()

        ############################################
        # Notebook pages for specific object setting
        ############################################
        self.objNotebook = Pmw.NoteBook(settingFrame, tabpos = None,
                                        borderwidth = 0)
        PolygonPage = self.objNotebook.add('Polygon')
        SpherePage = self.objNotebook.add('Sphere')
        SegmentPage = self.objNotebook.add('Segment')
        RayPage = self.objNotebook.add('Ray')
        self.objNotebook.selectpage('Sphere')
        # Put this here so it isn't called right away
        self.objNotebook['raisecommand'] = self.updateObjInfo

        # Polygon object setting

        Interior = Frame(PolygonPage)
        label = Label(Interior, text='Attention! All Coordinates Are Related To Its Parent Node!')
        label.pack(side=LEFT,expand=0,fill=X, padx=1)
        Interior.pack(side=TOP, expand=0,fill=X)

        self.createPosEntry(PolygonPage, catagory='Polygon', id='Point A')
        self.createPosEntry(PolygonPage, catagory='Polygon', id='Point B')
        self.createPosEntry(PolygonPage, catagory='Polygon', id='Point C')

        # Sphere object setting

        Interior = Frame(SpherePage)
        label = Label(Interior, text='Attention! All Coordinates Are Related To Its Parent Node!')
        label.pack(side=LEFT,expand=0,fill=X, padx=1)
        Interior.pack(side=TOP, expand=0,fill=X)

        self.createPosEntry(SpherePage, catagory='Sphere', id='Center Point')

        self.createEntryField(SpherePage,catagory='Sphere', id='Size',
                              value = 1.0,
                              command = None,
                              initialState='normal',
                              side = 'top')

        # Segment object setting

        Interior = Frame(SegmentPage)
        label = Label(Interior, text='Attention! All Coordinates Are Related To Its Parent Node!')
        label.pack(side=LEFT,expand=0,fill=X, padx=1)
        Interior.pack(side=TOP, expand=0,fill=X)

        self.createPosEntry(SegmentPage, catagory='Segment', id='Point A')
        self.createPosEntry(SegmentPage, catagory='Segment', id='Point B')

        # Ray object setting

        Interior = Frame(RayPage)
        label = Label(Interior, text='Attention! All Coordinates Are Related To Its Parent Node!')
        label.pack(side=LEFT,expand=0,fill=X, padx=1)
        Interior.pack(side=TOP, expand=0,fill=X)

        self.createPosEntry(RayPage, catagory='Ray', id='Origin')
        self.createPosEntry(RayPage, catagory='Ray', id='Direction')



        self.objNotebook.setnaturalsize()
        self.objNotebook.pack(expand = 1, fill = BOTH)

        self.okButton = Button(mainFrame, text="OK", command=self.okPress,width=10)
        self.okButton.pack(fill=BOTH,expand=0,side=RIGHT)

        mainFrame.pack(expand=1, fill = BOTH)

    def onDestroy(self, event):
        messenger.send('CW_close')
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

    def setObjectType(self, typeName = 'collisionSphere'):
        #################################################################
        # setObjectType(self, typeName = 'collisionSphere')
        # Call back function
        # This function will be called when user select target collision
        # type on the combo box on the panel.
        # Basically, this function's job is to switch the notebook page to right one.
        #################################################################
        self.objType = typeName
        if self.objType=='collisionPolygon':
            self.objNotebook.selectpage('Polygon')
        elif self.objType=='collisionSphere':
            self.objNotebook.selectpage('Sphere')
        elif self.objType=='collisionSegment':
            self.objNotebook.selectpage('Segment')
        elif self.objType=='collisionRay':
            self.objNotebook.selectpage('Ray')

        return

    def updateObjInfo(self, page=None):
        #################################################################
        # Nothing. Unlike in the lighting panel, we don't have to keep data
        # once user switch the page.
        #################################################################
        return

    def okPress(self):
        #################################################################
        # okPress(self)
        # This function will be called when user click on the Ok button.
        # Then this function will collect all parameters that we need to create
        # a collision Object from the panel and generate the colision Object.
        # In the last, it will send the object out with a message to dataHolder to
        # put the object into a CollisionNode and attach it to the target nodePath
        #################################################################
        collisionObject = None
        print(self.objType)
        if self.objType=='collisionPolygon':
            pointA =  Point3(float(self.widgetDict['PolygonPoint A'][0]._entry.get()),
                             float(self.widgetDict['PolygonPoint A'][1]._entry.get()),
                             float(self.widgetDict['PolygonPoint A'][2]._entry.get()))
            pointB =  Point3(float(self.widgetDict['PolygonPoint B'][0]._entry.get()),
                             float(self.widgetDict['PolygonPoint B'][1]._entry.get()),
                             float(self.widgetDict['PolygonPoint B'][2]._entry.get()))
            pointC =  Point3(float(self.widgetDict['PolygonPoint C'][0]._entry.get()),
                             float(self.widgetDict['PolygonPoint C'][1]._entry.get()),
                             float(self.widgetDict['PolygonPoint C'][2]._entry.get()))
            collisionObject = CollisionPolygon(pointA, pointB, pointC)

        elif self.objType=='collisionSphere':
            collisionObject = CollisionSphere(float(self.widgetDict['SphereCenter Point'][0]._entry.get()),
                                              float(self.widgetDict['SphereCenter Point'][1]._entry.get()),
                                              float(self.widgetDict['SphereCenter Point'][2]._entry.get()),
                                              float(self.widgetDict['SphereSize'].getvalue()))

        elif self.objType=='collisionSegment':
            pointA =  Point3(float(self.widgetDict['SegmentPoint A'][0]._entry.get()),
                             float(self.widgetDict['SegmentPoint A'][1]._entry.get()),
                             float(self.widgetDict['SegmentPoint A'][2]._entry.get()))
            pointB =  Point3(float(self.widgetDict['SegmentPoint B'][0]._entry.get()),
                             float(self.widgetDict['SegmentPoint B'][1]._entry.get()),
                             float(self.widgetDict['SegmentPoint B'][2]._entry.get()))

            collisionObject = CollisionSegment()
            collisionObject.setPointA(pointA)
            collisionObject.setFromLens(base.cam.node(), Point2( -1, 1 ))  ## You must set up the camera lensNode before you set point B....
            collisionObject.setPointB(pointB)

        elif self.objType=='collisionRay':
            point =  Point3(float(self.widgetDict['RayOrigin'][0]._entry.get()),
                            float(self.widgetDict['RayOrigin'][1]._entry.get()),
                            float(self.widgetDict['RayOrigin'][2]._entry.get()))

            vector =  Vec3(float(self.widgetDict['RayDirection'][0]._entry.get()),
                           float(self.widgetDict['RayDirection'][1]._entry.get()),
                           float(self.widgetDict['RayDirection'][2]._entry.get()))

            print(vector, point)

            collisionObject = CollisionRay()
            collisionObject.setOrigin(point)
            collisionObject.setDirection(vector)
            #collisionObject.setFromLens(base.cam.node(), Point2( -1, 1 ))  ## You must set up the camera lensNode before you set up others...

        if self.objType=='collisionPolygon':
            messenger.send('CW_addCollisionObj', [collisionObject, self.nodePath, pointA, pointB, pointC])
        else:
            messenger.send('CW_addCollisionObj', [collisionObject, self.nodePath])

        self.quit()

        return

    def createPosEntry(self, contentFrame, catagory, id):
        posInterior = Frame(contentFrame)
        label = Label(posInterior, text=id+':')
        label.pack(side=LEFT,expand=0,fill=X, padx=1)
        self.posX = self.createcomponent('posX'+catagory+id, (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'X', relief = FLAT,
                                         value = 0.0,
                                         entry_width = 6)

        self.posX.pack(side=LEFT,expand=0,fill=X, padx=1)

        self.posY = self.createcomponent('posY'+catagory+id, (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Y', relief = FLAT,
                                         value = 0.0,
                                         entry_width = 6)
        self.posY.pack(side=LEFT, expand=0,fill=X, padx=1)

        self.posZ = self.createcomponent('posZ'+catagory+id, (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Z', relief = FLAT,
                                         value = 0.0,
                                         entry_width = 6)
        self.posZ.pack(side=LEFT, expand=0,fill=X, padx=1)
        self.widgetDict[catagory+id]=[self.posX, self.posY, self.posZ]
        posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)
        return

    def createEntryField(self, parent, catagory, id, value,
                         command, initialState, labelWidth = 6,
                         side = 'left', fill = X, expand = 0,
                         validate = None,
                         defaultButton = False, buttonText = 'Default',defaultFunction = None ):
        frame = Frame(parent)
        widget = Pmw.EntryField(frame, labelpos='w', label_text = id+':',
                                value = value, entry_font=('MSSansSerif', 10),label_font=('MSSansSerif', 10),
                                modifiedcommand=command, validate = validate,
                                label_width = labelWidth)
        widget.configure(entry_state = initialState)
        widget.pack(side=LEFT)
        self.widgetDict[catagory+id] = widget
        if defaultButton and (defaultFunction!=None):
            widget = Button(frame, text=buttonText, font=('MSSansSerif', 10), command = defaultFunction)
            widget.pack(side=LEFT, padx=3)
            self.widgetDict[catagory+id+'-'+'DefaultButton']=widget

        frame.pack(side = side, fill = fill, expand = expand,pady=3)
