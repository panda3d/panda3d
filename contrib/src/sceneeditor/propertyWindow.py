#################################################################
# propertyWindow.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *

from seColorEntry import *

from direct.tkwidgets import Floater
from direct.tkwidgets import Dial
from direct.tkwidgets import Slider
from direct.tkwidgets import VectorWidgets
from panda3d.core import *
import Pmw

class propertyWindow(AppShell,Pmw.MegaWidget):
    #################################################################
    # propertyWindow(AppShell,Pmw.MegaWidget)
    # This class will create a widow to show the object property and
    # let user can change shoe of them.
    #################################################################
    appversion      = '1.0'
    appname         = 'Property Window'
    frameWidth      = 400
    frameHeight     = 400
    padx            = 0
    pady            = 0
    usecommandarea  = 0
    usestatusarea   = 0
    widgetsDict = {}


    def __init__(self, target, type, info, parent = None, nodePath = render, **kw):
        self.nodePath = target
        self.name = target.getName()
        self.type = type
        self.info = info


        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Define the megawidget options.
        optiondefs = (
            ('title',       self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        if parent == None:
            self.parent = Toplevel()
        AppShell.__init__(self, self.parent)

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def appInit(self):
        return

    def createInterface(self):
        # The interior of the toplevel panel
        interior = self.interior()
        mainFrame = Frame(interior)
        name_label = Label(mainFrame, text= self.name,font=('MSSansSerif', 15),
                           relief = RIDGE, borderwidth=5)
        name_label.pack()
        outFrame = Frame(mainFrame, relief = RIDGE, borderwidth=3)
        self.contentWidge = self.createcomponent(
            'scrolledFrame',
            (), None,
            Pmw.ScrolledFrame, (outFrame,),
            hull_width = 200, hull_height = 300,
            usehullsize = 1)
        self.contentFrame = self.contentWidge.component('frame')
        self.contentWidge.pack(fill = 'both', expand = 1,padx = 3, pady = 5)
        outFrame.pack(fill = 'both', expand = 1)

        # Creating different interface depands on object's type
        if self.type == 'camera':
            self.cameraInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneCamera)
        elif self.type == 'Model':
            self.modelInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneModel)
        elif self.type == 'Actor':
            self.modelInterface(self.contentFrame)
            self.actorInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneActor)
            pass
        elif self.type == 'Light':
            self.lightInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneLight)
            pass
        elif self.type == 'dummy':
            self.dummyInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneDummy)
            pass
        elif self.type == 'collisionNode':
            self.collisionInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, self.trackDataFromSceneCollision)
            pass
        elif self.type == 'Special':
            # If user try to open the property window for node "SEditor"
            # It will show the grid property.
            self.gridInterface(self.contentFrame)
            self.accept('forPorpertyWindow'+self.name, None)
            pass

        self.curveFrame = None
        #### If nodePath has been binded with any curves
        if 'curveList' in self.info:
            self.createCurveFrame(self.contentFrame)

        ## Set all stuff done
        mainFrame.pack(fill = 'both', expand = 1)


    def createMenuBar(self):
        # we don't need menu bar here.
        self.menuBar.destroy()

    def onDestroy(self, event):
        self.ignore('forPorpertyWindow'+self.name)
        messenger.send('PW_close', [self.name])
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

    def createEntryField(self, parent,text, value,
                         command, initialState, labelWidth = 12,
                         side = 'left', fill = X, expand = 0,
                         validate = None,
                         defaultButton = False, buttonText = 'Default',defaultFunction = None ):
        #################################################################
        # createEntryField(self, parent,text, value,
        #                  command, initialState, labelWidth = 12,
        #                  side = 'left', fill = X, expand = 0,
        #                  validate = None,
        #                  defaultButton = False, buttonText = 'Default',defaultFunction = None ):
        # This function will create a Entry on the frame "parent"
        # Also, if user has enabled the "defaultButton," it will create a button right after the entry.
        #################################################################
        frame = Frame(parent)
        widget = Pmw.EntryField(frame, labelpos='w', label_text = text,
                                value = value, entry_font=('MSSansSerif', 10),label_font=('MSSansSerif', 10),
                                modifiedcommand=command, validate = validate,
                                label_width = labelWidth)
        widget.configure(entry_state = initialState)
        widget.pack(side=LEFT)
        self.widgetsDict[text] = widget
        if defaultButton and (defaultFunction!=None):
            # create a button if they need.
            widget = Button(frame, text=buttonText, font=('MSSansSerif', 10), command = defaultFunction)
            widget.pack(side=LEFT, padx=3)
            self.widgetsDict[text+'-'+'DefaultButton']=widget

        frame.pack(side = side, fill = fill, expand = expand,pady=3)


    def createPosEntry(self, contentFrame):
        #################################################################
        # createPosEntry(self, contentFrame)
        # This function will create three entries for setting position for the objects.
        # the entry type is Floater.
        # And, it will set the call back function to setNodePathPosHprScale()
        #################################################################
        posInterior = Frame(contentFrame)
        self.posX = self.createcomponent('posX', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'X', relief = FLAT,
                                         value = self.nodePath.getX(),
                                         label_foreground = 'Red',
                                         entry_width = 9)
        self.posX['commandData'] = ['x']
        self.posX['command'] = self.setNodePathPosHprScale
        self.posX.pack(side=LEFT,expand=0,fill=X, padx=1)

        self.posY = self.createcomponent('posY', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Y', relief = FLAT,
                                         value = self.nodePath.getY(),
                                         label_foreground = '#00A000',
                                         entry_width = 9)
        self.posY['commandData'] = ['y']
        self.posY['command'] = self.setNodePathPosHprScale
        self.posY.pack(side=LEFT, expand=0,fill=X, padx=1)

        self.posZ = self.createcomponent('posZ', (), None,
                                         Floater.Floater, (posInterior,),
                                         text = 'Z', relief = FLAT,
                                         value = self.nodePath.getZ(),
                                         label_foreground = 'Blue',
                                         entry_width = 9)
        self.posZ['commandData'] = ['z']
        self.posZ['command'] = self.setNodePathPosHprScale
        self.posZ.pack(side=LEFT, expand=0,fill=X, padx=1)
        posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)

    def createHprEntry(self, contentFrame):
        #################################################################
        # createHprEntry(self, contentFrame)
        # This function will create three entries for setting orientation for the objects.
        # the entry type is Floater.
        # And, it will set the call back function to setNodePathPosHprScale()
        #################################################################
        hprInterior = Frame(contentFrame)
        self.hprH = self.createcomponent('hprH', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'H', value = self.nodePath.getH(),
                                         relief = FLAT,
                                         label_foreground = 'blue',
                                         entry_width = 9)
        self.hprH['commandData'] = ['h']
        self.hprH['command'] = self.setNodePathPosHprScale
        self.hprH.pack(side = LEFT, expand=0,fill=X)

        self.hprP = self.createcomponent('hprP', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'P', value = self.nodePath.getP(),
                                         relief = FLAT,
                                         label_foreground = 'red',
                                         entry_width = 9)
        self.hprP['commandData'] = ['p']
        self.hprP['command'] = self.setNodePathPosHprScale
        self.hprP.pack(side = LEFT, expand=0,fill=X)

        self.hprR = self.createcomponent('hprR', (), None,
                                         Dial.AngleDial, (hprInterior,),
                                         style = 'mini',
                                         text = 'R', value = self.nodePath.getR(),
                                         relief = FLAT,
                                         label_foreground = '#00A000',
                                         entry_width = 9)
        self.hprR['commandData'] = ['r']
        self.hprR['command'] = self.setNodePathPosHprScale
        self.hprR.pack(side = LEFT, expand=0,fill=X)

        hprInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)


    def createScaleEntry(self, contentFrame):
        #################################################################
        # createScaleEntry(self, contentFrame)
        # This function will create three entries for setting scale for the objects.
        # the entry type is Floater.
        # And, it will set the call back function to setNodePathPosHprScale()
        #################################################################
        scaleInterior = Frame(contentFrame)

        self.scale = self.createcomponent('scale', (), None,
                                           Floater.Floater, (scaleInterior,),
                                           text = 'Scale',
                                           relief = FLAT,
                                           min = 0.0001, value = self.nodePath.getScale().getX(),
                                           resetValue = 1.0,
                                           label_foreground = 'Blue')
        self.scale['commandData'] = ['s']
        self.scale['command'] = self.setNodePathPosHprScale
        self.scale.pack(side=LEFT,expand=0,fill=X)

        scaleInterior.pack(side=TOP,expand=0,fill=X, padx=3, pady=3)

    def createColorEntry(self, contentFrame):
        #################################################################
        # createColorEntry(self, contentFrame)
        # This function will create three entries for setting color for the objects.
        # the entry type is Floater.
        # And, it will set the call back function to setNodeColorVec()
        #################################################################
        color = self.nodePath.getColor()
        print(color)
        self.nodeColor = VectorWidgets.ColorEntry(
            contentFrame, text = 'Node Color', value=[color.getX()*255,
                                                      color.getY()*255,
                                                      color.getZ()*255,
                                                      color.getW()*255])
        self.nodeColor['command'] = self.setNodeColorVec
        self.nodeColor['resetValue'] = [255,255,255,255]
        self.nodeColor.place(anchor=NW,y=235)
        self.bind(self.nodeColor, 'Set nodePath color')
        self.nodeColor.pack(side=TOP,expand=0,fill=X, padx=3, pady=3)
        return

    def setNodeColorVec(self, color):
        #################################################################
        # setNodeColorVec(self, color)
        # This function will set the color of the object
        #################################################################
        self.nodePath.setColor(color[0]/255.0,
                               color[1]/255.0,
                               color[2]/255.0,
                               color[3]/255.0)
        return


    def setNodePathPosHprScale(self, data, axis):
        #################################################################
        # setNodePathPosHprScale(self, data, axis)
        # This function will set the postion, orientation or scale of the object
        # use the "axis" parameter to decide which property should be set.
        #################################################################
        if axis == 'x':
            self.nodePath.setX(data)
        elif axis == 'y':
            self.nodePath.setY(data)
        elif axis == 'z':
            self.nodePath.setZ(data)
        elif axis == 'h':
            self.nodePath.setH(data)
        elif axis == 'p':
            self.nodePath.setP(data)
        elif axis == 'r':
            self.nodePath.setR(data)
        elif axis == 's':
            self.nodePath.setScale(data)


    #### Curve property
    def createCurveFrame(self, contentFrame):
        #################################################################
        # createCurveFrame(self, contentFrame)
        # Draw the curve property frame
        # This function will draw the property frame and content of curves
        # pass the target frame as a variable
        #################################################################
        if self.curveFrame==None:
            self.curveFrame = Frame(contentFrame)
            group = Pmw.Group(self.curveFrame,
                              tag_text='Motion Path List for this Node',
                              tag_font=('MSSansSerif', 10))
            innerFrame = group.interior()
            n = 0
            for curve in self.info['curveList']:
                n += 1
                self.createEntryField(innerFrame,'Curve %d:' %n,
                                      value = curve.getCurve(0).getName(),
                                      command = None,
                                      initialState='disabled',
                                      side = 'top',
                                      defaultButton = True,
                                      buttonText = 'delete',
                                      defaultFunction = lambda a = n, b = self : b.deleteCurve(a))
            group.pack(side = TOP, fill = X, expand = 0,pady=3, padx=3)
            self.curveFrame.pack(side = TOP, fill = X, expand = 0,pady=3, padx=3)

        return

    def deleteCurve(self, number = 0):
        #################################################################
        # deleteCurve(self, number = 0)
        # Call back function, will be called when user click on the "delete" button beside the curve name.
        # This function will send the message to sceneEditor to remove the target curve
        # and will set a callback function waitting the result.
        #################################################################
        widget = self.widgetsDict['Curve %d:' %number]
        curveName = widget.getvalue()
        self.accept('curveRemovedFromNode',self.redrawCurveProperty)
        messenger.send('PW_removeCurveFromNode',[self.nodePath, curveName])
        return

    def redrawCurveProperty(self, nodePath, curveList):
        #################################################################
        # redrawCurveProperty(self, nodePath, curveList)
        # Callback function, will be called once get the result from dataHolder.
        # It will check the target nodePath first, then check the curve list is empty or not.
        # If yes, then delete whole curve frame. If not, then renew the data and redraw the curve frame again.
        #################################################################
        self.name = self.nodePath.getName()
        if self.name != nodePath.getName():
            messenger.send('curveRemovedFromNode',[nodePath, curveList])
            return
        else:
            self.ignore('curveRemovedFromNode')

        if curveList!= None:
            del self.info['curveList']
            self.info['curveList'] = curveList
            self.curveFrame.destroy()
            del self.curveFrame
            self.curveFrame = None
            self.createCurveFrame(self.contentFrame)
        else:
            del self.info['curveList']
            self.curveFrame.destroy()
            del self.curveFrame
            self.curveFrame = None
        return

    ####
    ####  Anything about Camera will be here!
    ####
    def cameraInterface(self, contentFrame):
        #################################################################
        # cameraInterface(self, interior, mainFrame)
        # Create the interface for camera node.
        #################################################################

        ## Type entry : unchageable
        widget = self.createEntryField(contentFrame,'Type:',
                                       value = self.type,
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')

        ## lens Type entry
        widget = self.createEntryField(contentFrame, 'Lens Type:',
                                       value = self.info['lensType'],
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')

        ## Pos
        group = Pmw.Group(contentFrame,tag_text='Position',
                          tag_font=('MSSansSerif', 10))
        self.createPosEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        ## Orientation
        group = Pmw.Group(contentFrame,tag_text='Orientation',
                          tag_font=('MSSansSerif', 10))
        self.createHprEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        ## near entry
        group = Pmw.Group(contentFrame,tag_text='Lens Property',
                          tag_font=('MSSansSerif', 10))
        lensFrame = group.interior()
        widget = self.createEntryField(lensFrame, 'Near:',value = self.info['near'],
                                       command = self.setCameraNear,
                                       initialState='normal',
                                       validate = Pmw.realvalidator,
                                       side = 'top',
                                       defaultButton = True,
                                       defaultFunction = self.defaultCameraNear)

        ## far entry
        widget = self.createEntryField(lensFrame, 'Far:',
                                       value = self.info['far'],
                                       command = self.setCameraFar,
                                       initialState='normal',
                                       side = 'top',
                                       validate = Pmw.realvalidator,
                                       defaultButton = True,
                                       defaultFunction = self.defaultCameraFar)

        ## Hfov entry
        widget = self.createEntryField(lensFrame, 'H.F.O.V.:',
                                           value = self.info['hFov'],
                                           command = self.setCameraFov,
                                           validate = Pmw.realvalidator,
                                           initialState='normal',
                                           side = 'top',
                                           defaultButton = True,
                                           defaultFunction = self.defaultCameraHfov)

        ## Vfov entry
        widget = self.createEntryField(lensFrame, 'V.F.O.V.:',
                                       value = self.info['vFov'],
                                       command = self.setCameraFov,
                                       validate = Pmw.realvalidator,
                                       initialState='normal',
                                       side = 'top',
                                       defaultButton = True,
                                       defaultFunction = self.defaultCameraVfov)

        ## Film Size entry
        frame = Frame(lensFrame)
        widget = Label(frame, text = "Film Size:", font=('MSSansSerif', 10),width=12)
        widget.pack(side=LEFT)
        frame.pack(side = TOP, fill = X, expand = 0, pady=3)

        frame = Frame(lensFrame)
        widget = Pmw.EntryField(frame, labelpos='w', label_text = '                        ',
                                value = self.info['FilmSize'].getX(),
                                entry_font=('MSSansSerif', 10),
                                label_font=('MSSansSerif', 10),
                                modifiedcommand=self.setCameraFilmSize, validate = Pmw.realvalidator,
                                entry_width = 8)
        self.widgetsDict['FilmSizeX']=widget
        widget.pack(side=LEFT, padx=3)
        widget = Pmw.EntryField(frame, labelpos='w', label_text = ': ', value = self.info['FilmSize'].getY() ,
                                label_font=('MSSansSerif', 10),
                                entry_font=('MSSansSerif', 10),
                                modifiedcommand=self.setCameraFilmSize, validate = Pmw.realvalidator,
                                entry_width = 8)
        self.widgetsDict['FilmSizeY']=widget
        widget.pack(side=LEFT, padx=3)
        widget = Button(frame, text='Default', font=('MSSansSerif', 10), command = self.defaultCameraFilmSize)
        widget.pack(side=LEFT, padx=3)
        self.widgetsDict['FilmSize'+'-'+'DefaultButton']=widget
        frame.pack(side = TOP, fill = X, expand = 0,pady=0)

        ## Focal Length entry
        widget = self.createEntryField(lensFrame, 'Focal Length:',
                                       value = self.info['focalLength'],
                                       command = self.setCameraFocalLength,
                                       validate = Pmw.realvalidator,
                                       initialState='normal',
                                       side = 'top',
                                       defaultButton = True,
                                       defaultFunction = self.defaultCameraFocalLength)
        group.pack(side = TOP, fill = X, expand = 0,pady=2)


    def defaultCameraFar(self):
        #################################################################
        # defaultCameraFar(self)
        # set the camera "Far" value back to default.
        #################################################################
        widget = self.widgetsDict['Far:']
        widget.setvalue(base.cam.node().getLens().getDefaultFar())
        return

    def setCameraFar(self):
        #################################################################
        # setCameraFar(self)
        # set the camera "Far" value to what now user has typed in the entry
        #################################################################
        if self.widgetsDict['Far:'].getvalue() != '':
            value = float(self.widgetsDict['Far:'].getvalue())
        else:
            value = 0
        camera.getChild(0).node().getLens().setFar(value)
        return

    def defaultCameraNear(self):
        #################################################################
        # defaultCameraNear(self)
        # set the camera "Near" value back to default.
        #################################################################
        widget = self.widgetsDict['Near:']
        widget.setvalue(base.cam.node().getLens().getDefaultNear())
        return

    def setCameraNear(self):
        #################################################################
        # setCameraNear(self)
        # set the camera "Near" value to what now user has typed in the entry
        #################################################################
        if self.widgetsDict['Near:'].getvalue() != '':
            value = float(self.widgetsDict['Near:'].getvalue())
        else:
            value = 0
        camera.getChild(0).node().getLens().setNear(value)
        return

    def defaultCameraHfov(self):
        #################################################################
        # defaultCameraHfov(self)
        # set the camera "Hfov" value back to default.
        #################################################################
        widget = self.widgetsDict['H.F.O.V.:']
        widget.setvalue(45.0)
        return

    def setCameraFov(self):
        #################################################################
        # setCameraFov(self)
        # set the camera "Fov" value to what now user has typed in the entry
        #################################################################
        if self.widgetsDict['H.F.O.V.:'].getvalue() != '':
            value1 = float(self.widgetsDict['H.F.O.V.:'].getvalue())
        else:
            value1 = 0
        if self.widgetsDict['V.F.O.V.:'].getvalue() != '':
            value2 = float(self.widgetsDict['V.F.O.V.:'].getvalue())
        else:
            value2 = 0
        camera.getChild(0).node().getLens().setFov(VBase2(value1,value2))
        return

    def defaultCameraVfov(self):
        #################################################################
        # defaultCameraVfov(self)
        # set the camera "Vfov" value back to default.
        #################################################################
        widget = self.widgetsDict['V.F.O.V.:']
        widget.setvalue(34.51587677)
        return

    def defaultCameraFocalLength(self):
        #################################################################
        # defaultCameraFocalLength(self)
        # set the camera "Focal Length" value back to default.
        #################################################################
        widget = self.widgetsDict['Focal Length:']
        widget.setvalue(1.20710682869)
        return

    def setCameraFocalLength(self):
        #################################################################
        # setCameraFocalLength(self)
        # set the camera "Focal Length" value to what now user has typed in the entry
        #################################################################
        if self.widgetsDict['Focal Length:'].getvalue() != '':
            value = float(self.widgetsDict['Focal Length:'].getvalue())
        else:
            value = 0
        camera.getChild(0).node().getLens().setFocalLength(value)
        camera.getChild(0).node().getLens().setFilmSize(VBase2(float(self.widgetsDict['FilmSizeX'].getvalue()),float(self.widgetsDict['FilmSizeY'].getvalue())))
        return

    def defaultCameraFilmSize(self):
        #################################################################
        # defaultCameraFilmSize(self)
        # set the camera "Film Size" value back to default.
        #################################################################
        widget = self.widgetsDict['FilmSizeX']
        widget.setvalue(1)
        widget = self.widgetsDict['FilmSizeY']
        widget.setvalue(0.75)
        return

    def setCameraFilmSize(self):
        #################################################################
        # setCameraFilmSize(self)
        # set the camera "Film Size" value to what now user has typed in the entry
        #################################################################
        if self.widgetsDict['FilmSizeX'].getvalue() != '':
            value1 = float(self.widgetsDict['FilmSizeX'].getvalue())
        else:
            value1 = 0
        if self.widgetsDict['FilmSizeY'].getvalue() != '':
            value2 = float(self.widgetsDict['FilmSizeY'].getvalue())
        else:
            value2 = 0
        camera.getChild(0).node().getLens().setFilmSize(VBase2(value1,value2))
        return

    ####
    ####  Anything about Model & Actor will be here!
    ####
    def modelInterface(self, contentFrame):
        #################################################################
        # modelInterface(self, contentFrame)
        # Create the basic interface for ModelRoot Type Node
        #################################################################
        widget = self.createEntryField(contentFrame,'Type:',
                                       value = self.type,
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')
        widget = self.createEntryField(contentFrame,'Model File:',
                                       value = self.info['filePath'].getFullpath(),
                                       command = None,
                                       initialState='disabled',
                                       side = 'top',
                                       defaultButton = False,
                                       buttonText = 'Change',
                                       defaultFunction = None)
        group = Pmw.Group(contentFrame,tag_text='Position',
                          tag_font=('MSSansSerif', 10))
        self.createPosEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        group = Pmw.Group(contentFrame,tag_text='Orientation',
                          tag_font=('MSSansSerif', 10))
        self.createHprEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        self.createScaleEntry(contentFrame)

        group = Pmw.Group(contentFrame,tag_text='Color',
                          tag_font=('MSSansSerif', 10))
        frame = group.interior()
        self.createColorEntry(frame)
        self.varAlpha = IntVar()
        self.varAlpha.set(self.nodePath.hasTransparency())
        checkButton = Checkbutton(frame, text='Enable Alpha',
                                  variable=self.varAlpha, command=self.toggleAlpha)
        checkButton.pack(side=RIGHT,pady=3)
        group.pack(side=TOP,fill = X, expand = 0, pady=3)
        return

    def toggleAlpha(self):
        #################################################################
        # toggleAlpha(self)
        # This funtion will toggle the objects alpha value
        # And, it will also reset the "Bin" to
        # "fixed" if user enable the alpha for this object.
        #################################################################
        if self.nodePath.hasTransparency():
            self.nodePath.clearTransparency()
            self.nodePath.setBin("default", 0)
        else:
            self.nodePath.setTransparency(True)
            self.nodePath.setBin("fixed", 1)
        return

    def actorInterface(self, contentFrame):
        #################################################################
        # actorInterface(self, contentFrame)
        # Create the basic interface for Actor Type Node
        #################################################################
        self.animFrame = None
        animeDict = self.info['animDict']
        if len(animeDict)==0:
            return

        self.animFrame = Frame(contentFrame)
        group = Pmw.Group(self.animFrame,tag_text='Animations',
                                   tag_font=('MSSansSerif', 10))
        innerFrame = group.interior()
        for name in animeDict:
            self.createEntryField(innerFrame, name,
                                  value = animeDict[name],
                                  command = None,
                                  initialState='disabled',
                                  side = 'top',
                                  defaultButton = True,
                                  buttonText = 'Remove',
                                  defaultFunction = lambda a = name, b = self : b.deleteAnimation(a))
        group.pack(side=TOP,fill = X, expand = 0, pady=3)
        self.animFrame.pack(side=TOP,fill = X, expand = 0, pady=3)
        return


    def deleteAnimation(self, anim):
        #################################################################
        # deleteAnimation(self, anim)
        # This function will delete the animation named "anim" in this actor
        # But, not directly removed be this function.
        # This function will send out a message to notice dataHolder to remove this animation
        #################################################################
        print(anim)
        widget = self.widgetsDict[anim]
        self.accept('animRemovedFromNode',self.redrawAnimProperty)
        messenger.send('PW_removeAnimFromNode',[self.name, anim])
        return

    def redrawAnimProperty(self, nodePath, animDict):
        #################################################################
        # redrawCurveProperty(self, nodePath, curveList)
        # Callback function, will be called once get the result from dataHolder.
        # It will check the target nodePath first, then check the curve list is empty or not.
        # If yes, then delete whole curve frame. If not, then renew the data and redraw the curve frame again.
        #################################################################
        self.name = self.nodePath.getName()
        if self.name != nodePath.getName():
            messenger.send('animRemovedFromNode',[nodePath, animDict])
            return
        else:
            self.ignore('animRemovedFromNode')

        if len(animDict)!= 0:
            del self.info['animDict']
            self.info['animDict'] = animDict
            self.animFrame.destroy()
            del self.animFrame
            self.animFrame = None
            self.actorInterface(self.contentFrame)
        else:
            del self.info['animDict']
            self.animFrame.destroy()
            del self.animFrame
            self.animFrame = None
        return

    ####
    ####  Anything about Light will be here!
    ####
    def lightInterface(self, contentFrame):
        #################################################################
        # lightInterface(self, contentFrame)
        # Create the basic interface for light Type Node
        #################################################################
        widget = self.createEntryField(contentFrame,'Type:',
                                       value = self.nodePath.node().getType().getName(),
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')

        self.lightNode = self.info['lightNode']

        lightingGroup = Pmw.Group(contentFrame,tag_pyclass=None)
        frame = lightingGroup.interior()
        self.lightColor = seColorEntry(
            frame, text = 'Light Color', label_font=('MSSansSerif', 10),
            value=[self.lightNode.lightcolor.getX()*255, self.lightNode.lightcolor.getY()*255,self.lightNode.lightcolor.getZ()*255,0])
        self.lightColor['command'] = self.setLightingColorVec
        self.lightColor['resetValue'] = [0.3*255,0.3*255,0.3*255,0]
        self.lightColor.pack(side=TOP, fill=X,expand=1, padx = 2, pady =2)
        self.bind(self.lightColor, 'Set light color')

        self.varActive = IntVar()
        self.varActive.set(self.lightNode.active)
        checkButton = Checkbutton(frame, text='Enable This Light',
                                  variable=self.varActive, command=self.toggleLight)
        checkButton.pack(side=RIGHT,pady=3)
        lightingGroup.pack(side=TOP, fill = X, expand =1)

        # Directional light controls
        if self.lightNode.type == 'directional':
            lightingGroup = Pmw.Group(contentFrame,tag_pyclass=None)
            directionalPage = lightingGroup.interior()
            self.dSpecularColor = seColorEntry(
                directionalPage, text = 'Specular Color',  label_font=('MSSansSerif', 10),value = [self.lightNode.specularColor.getX()*255,self.lightNode.specularColor.getY()*255,self.lightNode.specularColor.getZ()*255,0])
            self.dSpecularColor['command'] = self.setSpecularColor
            self.dSpecularColor.pack(fill = X, expand = 1)
            self.bind(self.dSpecularColor,
                      'Set directional light specular color')
            self.dPosition = VectorWidgets.Vector3Entry(
                directionalPage, text = 'Position',  label_font=('MSSansSerif', 10),value = [self.lightNode.getPosition().getX(),self.lightNode.getPosition().getY(),self.lightNode.getPosition().getZ()])
            self.dPosition['command'] = self.setPosition
            self.dPosition['resetValue'] = [0,0,0,0]
            self.dPosition.pack(fill = X, expand = 1)
            self.bind(self.dPosition, 'Set directional light position')
            self.dOrientation = VectorWidgets.Vector3Entry(
                directionalPage, text = 'Orientation', label_font=('MSSansSerif', 10),
                value = [self.lightNode.getOrientation().getX(),self.lightNode.getOrientation().getY(),self.lightNode.getOrientation().getZ(),0])
            self.dOrientation['command'] = self.setOrientation
            self.dOrientation['resetValue'] = [0,0,0,0]
            self.dOrientation.pack(fill = X, expand = 1)
            self.bind(self.dOrientation, 'Set directional light orientation')

            lightingGroup.pack(side=TOP, fill = X, expand =1)


        elif self.lightNode.type == 'point':
            # Point light controls
            lightingGroup = Pmw.Group(contentFrame,tag_pyclass=None)
            pointPage = lightingGroup.interior()
            self.pSpecularColor = seColorEntry(
                pointPage, text = 'Specular Color', label_font=('MSSansSerif', 10),
                value = [self.lightNode.specularColor.getX(),self.lightNode.specularColor.getY(),self.lightNode.specularColor.getZ(),0])
            self.pSpecularColor['command'] = self.setSpecularColor
            self.pSpecularColor.pack(fill = X, expand = 1)
            self.bind(self.pSpecularColor,
                      'Set point light specular color')

            self.pPosition = VectorWidgets.Vector3Entry(
                pointPage, text = 'Position',  label_font=('MSSansSerif', 10),
                value = [self.lightNode.getPosition().getX(),self.lightNode.getPosition().getY(),self.lightNode.getPosition().getZ(),0])
            self.pPosition['command'] = self.setPosition
            self.pPosition['resetValue'] = [0,0,0,0]
            self.pPosition.pack(fill = X, expand = 1)
            self.bind(self.pPosition, 'Set point light position')

            self.pConstantAttenuation = Slider.Slider(
                pointPage,
                text = 'Constant Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.constant)
            self.pConstantAttenuation['command'] = self.setConstantAttenuation
            self.pConstantAttenuation.pack(fill = X, expand = 1)
            self.bind(self.pConstantAttenuation,
                      'Set point light constant attenuation')

            self.pLinearAttenuation = Slider.Slider(
                pointPage,
                text = 'Linear Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.linear)
            self.pLinearAttenuation['command'] = self.setLinearAttenuation
            self.pLinearAttenuation.pack(fill = X, expand = 1)
            self.bind(self.pLinearAttenuation,
                      'Set point light linear attenuation')

            self.pQuadraticAttenuation = Slider.Slider(
                pointPage,
                text = 'Quadratic Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.quadratic)
            self.pQuadraticAttenuation['command'] = self.setQuadraticAttenuation
            self.pQuadraticAttenuation.pack(fill = X, expand = 1)
            self.bind(self.pQuadraticAttenuation,
                      'Set point light quadratic attenuation')

            lightingGroup.pack(side=TOP, fill = X, expand =1)


        elif self.lightNode.type == 'spot':
            # Spot light controls
            lightingGroup = Pmw.Group(contentFrame,tag_pyclass=None)
            spotPage = lightingGroup.interior()
            self.sSpecularColor = seColorEntry(
                spotPage, text = 'Specular Color', label_font=('MSSansSerif', 10),
                value = [self.lightNode.specularColor.getX()*255,self.lightNode.specularColor.getY()*255,self.lightNode.specularColor.getZ()*255,0])
            self.sSpecularColor['command'] = self.setSpecularColor
            self.sSpecularColor.pack(fill = X, expand = 1)
            self.bind(self.sSpecularColor,
                      'Set spot light specular color')

            self.sConstantAttenuation = Slider.Slider(
                spotPage,
                text = 'Constant Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.constant)
            self.sConstantAttenuation['command'] = self.setConstantAttenuation
            self.sConstantAttenuation.pack(fill = X, expand = 1)
            self.bind(self.sConstantAttenuation,
                      'Set spot light constant attenuation')

            self.sLinearAttenuation = Slider.Slider(
                spotPage,
                text = 'Linear Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.linear)
            self.sLinearAttenuation['command'] = self.setLinearAttenuation
            self.sLinearAttenuation.pack(fill = X, expand = 1)
            self.bind(self.sLinearAttenuation,
                      'Set spot light linear attenuation')

            self.sQuadraticAttenuation = Slider.Slider(
                spotPage,
                text = 'Quadratic Attenuation', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.quadratic)
            self.sQuadraticAttenuation['command'] = self.setQuadraticAttenuation
            self.sQuadraticAttenuation.pack(fill = X, expand = 1)
            self.bind(self.sQuadraticAttenuation,
                      'Set spot light quadratic attenuation')

            self.sExponent = Slider.Slider(
                spotPage,
                text = 'Exponent', label_font=('MSSansSerif', 10),
                max = 1.0,
                value = self.lightNode.exponent)
            self.sExponent['command'] = self.setExponent
            self.sExponent.pack(fill = X, expand = 1)
            self.bind(self.sExponent,
                      'Set spot light exponent')
            lightingGroup.pack(side=TOP, fill = X, expand =1)

        return

    def setLightingColorVec(self,color):
        if self.lightNode==None:
            return
        self.lightNode.setColor(VBase4((color[0]/255),(color[1]/255),(color[2]/255),1))
        return

    def setSpecularColor(self,color):
        if self.lightNode==None:
            return
        self.lightNode.setSpecColor(VBase4((color[0]/255),(color[1]/255),(color[2]/255),1))
        return

    def setPosition(self,position):
        if self.lightNode==None:
            return
        self.lightNode.setPosition(Point3(position[0],position[1],position[2]))
        return

    def setOrientation(self, orient):
        if self.lightNode==None:
            return
        self.lightNode.setOrientation(Vec3(orient[0],orient[1],orient[2]))
        return

    def setConstantAttenuation(self, value):
        self.lightNode.setConstantAttenuation(value)
        return

    def setLinearAttenuation(self, value):
        self.lightNode.setLinearAttenuation(value)
        return

    def setQuadraticAttenuation(self, value):
        self.lightNode.setQuadraticAttenuation(value)
        return

    def setExponent(self, value):
        self.lightNode.setExponent(value)
        return

    def toggleLight(self):
        messenger.send('PW_toggleLight',[self.lightNode])
        return


    ####
    ####  Anything about Dummy will be here!
    ####
    def dummyInterface(self, contentFrame):
        #################################################################
        # dummyInterface(self, contentFrame)
        # Create the basic interface for dummy Type Node
        #################################################################
        '''dummyInterface(self, contentFrame)
        Create the basic interface for dummy Node
        '''
        widget = self.createEntryField(contentFrame,'Type:',
                                       value = 'Dummy Nodepath',
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')

        group = Pmw.Group(contentFrame,tag_text='Position',
                          tag_font=('MSSansSerif', 10))
        self.createPosEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        group = Pmw.Group(contentFrame,tag_text='Orientation',
                          tag_font=('MSSansSerif', 10))
        self.createHprEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        self.createScaleEntry(contentFrame)

        group = Pmw.Group(contentFrame,tag_text='Color',
                          tag_font=('MSSansSerif', 10))
        frame = group.interior()
        self.createColorEntry(frame)
        self.varAlpha = IntVar()
        self.varAlpha.set(self.nodePath.hasTransparency())
        checkButton = Checkbutton(frame, text='Enable Alpha',
                                  variable=self.varAlpha, command=self.toggleAlpha)
        checkButton.pack(side=RIGHT,pady=3)
        group.pack(side=TOP,fill = X, expand = 0, pady=3)
        return


    #########
    #######  This will be called when user try to open property window for SEditor Node
    #########
    def gridInterface(self, contentFrame):
        #################################################################
        # gridInterface(self, contentFrame)
        # Create the basic interface for grid (Which is stolen from directGrid)
        #################################################################
        group = Pmw.Group(contentFrame,tag_text='Grid Property',
                          tag_font=('MSSansSerif', 10))
        group.pack(side=TOP,fill = X, expand = 0, padx = 3, pady=3)

        gridPage = group.interior()

        self.xyzSnap = BooleanVar()
        self.xyzSnapButton = Checkbutton(
            gridPage,
            text = 'XYZ Snap',
            anchor = 'w', justify = LEFT,
            variable = self.xyzSnap,
            command = self.toggleXyzSnap)
        self.xyzSnapButton.pack(fill = X, expand = 0, pady=3)

        self.hprSnap = BooleanVar()
        self.hprSnapButton = Checkbutton(
            gridPage,
            text = 'HPR Snap',
            anchor = 'w', justify = LEFT,
            variable = self.hprSnap,
            command = self.toggleHprSnap)
        self.hprSnapButton.pack(fill = X, expand = 0, pady=3)

        self.xyzSnap.set(SEditor.grid.getXyzSnap())
        self.hprSnap.set(SEditor.grid.getHprSnap())

        self.gridSpacing = Floater.Floater(
            gridPage,
            text = 'Grid Spacing',
            min = 0.1,
            value = SEditor.grid.getGridSpacing())
        self.gridSpacing['command'] = SEditor.grid.setGridSpacing
        self.gridSpacing.pack(fill = X, expand = 0, pady=3)

        self.gridSize = Floater.Floater(
            gridPage,
            text = 'Grid Size',
            min = 1.0,
            value = SEditor.grid.getGridSize())
        self.gridSize['command'] = SEditor.grid.setGridSize
        self.gridSize.pack(fill = X, expand = 0, pady=3)

        self.gridSnapAngle = Dial.AngleDial(
            gridPage,
            text = 'Snap Angle',
            style = 'mini',
            value = SEditor.grid.getSnapAngle())
        self.gridSnapAngle['command'] = SEditor.grid.setSnapAngle
        self.gridSnapAngle.pack(fill = X, expand = 0, pady=3)

        return

    def toggleXyzSnap(self):
        SEditor.grid.setXyzSnap(self.xyzSnap.get())
        return

    def toggleHprSnap(self):
        SEditor.grid.setHprSnap(self.hprSnap.get())
        return



    ###### Collision Section!!!!
    def collisionInterface(self, contentFrame):
        #################################################################
        # collisionInterface(self, contentFrame)
        # Create the basic interface for CollisionNode Type Node
        #################################################################
        collisionNode = self.info['collisionNode']
        self.collisionObj = collisionNode.node().getSolid(0)
        widget = self.createEntryField(contentFrame,'Node Type:',
                                       value = self.type,
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')
        cType = self.collisionObj.getType().getName()
        widget = self.createEntryField(contentFrame,'Object Type:',
                                       value = cType,
                                       command = None,
                                       initialState='disabled',
                                       side = 'top')
        group = Pmw.Group(contentFrame,tag_text='Position',
                          tag_font=('MSSansSerif', 10))
        self.createPosEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        group = Pmw.Group(contentFrame,tag_text='Orientation',
                          tag_font=('MSSansSerif', 10))
        self.createHprEntry(group.interior())
        group.pack(side=TOP,fill = X, expand = 0, pady=3)

        self.createScaleEntry(contentFrame)

        collisionGroup = Pmw.Group(contentFrame,tag_text='Collision Object Properties',
                          tag_font=('MSSansSerif', 10))
        cObjFrame = collisionGroup.interior()

        ### Generate different Interface for each different kinds of Collision Objects
        ### Yeah, yeah. I know this part of code looks so ugly...
        if cType == 'CollisionSphere':
            centerPos = self.collisionObj.getCenter()
            radius = self.collisionObj.getRadius()
            group = Pmw.Group(cObjFrame,tag_text='Origin',
                              tag_font=('MSSansSerif', 10))


            posInterior = Frame(group.interior())
            self.cPosX = self.createcomponent('originX', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'X', relief = FLAT,
                                             value = centerPos.getX(),
                                             label_foreground = 'Red',
                                             entry_width = 9)
            self.cPosX['commandData'] = ['sphere-o']
            self.cPosX['command'] = self.setCollisionPosHprScale
            self.cPosX.pack(side=LEFT,expand=0,fill=X, padx=1)

            self.cPosY = self.createcomponent('originY', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Y', relief = FLAT,
                                             value = centerPos.getY(),
                                             label_foreground = '#00A000',
                                             entry_width = 9)
            self.cPosY['commandData'] = ['sphere-o']
            self.cPosY['command'] = self.setCollisionPosHprScale
            self.cPosY.pack(side=LEFT, expand=0,fill=X, padx=1)

            self.cPosZ = self.createcomponent('originZ', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Z', relief = FLAT,
                                             value = centerPos.getZ(),
                                             label_foreground = 'Blue',
                                             entry_width = 9)
            self.cPosZ['commandData'] = ['sphere-o']
            self.cPosZ['command'] = self.setCollisionPosHprScale
            self.cPosZ.pack(side=LEFT, expand=0,fill=X, padx=1)
            posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)

            group.pack(side=TOP,fill = X, expand = 0, pady=3)

            scaleInterior = Frame(cObjFrame)

            self.scaleS = self.createcomponent('radius', (), None,
                                               Floater.Floater, (scaleInterior,),
                                               text = 'Radius',
                                               relief = FLAT,
                                               min = 0.0001, value = radius,
                                               resetValue = 1.0,
                                               label_foreground = 'Blue')
            self.scaleS['commandData'] = ['sphere-radius']
            self.scaleS['command'] = self.setCollisionPosHprScale
            self.scaleS.pack(side=LEFT,expand=0,fill=X)

            scaleInterior.pack(side=TOP,expand=0,fill=X, padx=3, pady=3)
            pass

        elif cType == 'CollisionPolygon':
            frame = Frame(cObjFrame)
            label = Label(frame, text= "Sorry!",font=('MSSansSerif', 10),
                          borderwidth=5)
            label.pack(side=LEFT)
            frame.pack(side=TOP, fill=X, expand=True)
            frame = Frame(cObjFrame)
            label = Label(frame, text= "There is no way to change",font=('MSSansSerif', 10),
                          borderwidth=5)
            label.pack(side=LEFT)
            frame.pack(side=TOP, fill=X, expand=True)
            frame = Frame(cObjFrame)
            label = Label(frame, text= "the basic properties of Collision Polygon!",font=('MSSansSerif', 10),
                          borderwidth=5)
            label.pack(side=LEFT)
            frame.pack(side=TOP, fill=X, expand=True)
            frame = Frame(cObjFrame)
            label = Label(frame, text= "If you really need to change, recreate one...",font=('MSSansSerif', 10),
                          borderwidth=5)
            label.pack(side=LEFT)
            frame.pack(side=TOP, fill=X, expand=True)
            pass

        elif cType == 'CollisionSegment':
            pointA = self.collisionObj.getPointA()
            pointB = self.collisionObj.getPointB()
            group = Pmw.Group(cObjFrame,tag_text='Point A',
                              tag_font=('MSSansSerif', 10))
            posInterior = Frame(group.interior())
            self.cPosX = self.createcomponent('pointA-X', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'X', relief = FLAT,
                                             value = pointA.getX(),
                                             label_foreground = 'Red',
                                             entry_width = 9)
            self.cPosX['commandData'] = ['segment-A']
            self.cPosX['command'] = self.setCollisionPosHprScale
            self.cPosX.pack(side=LEFT,expand=0,fill=X, padx=1)

            self.cPosY = self.createcomponent('pointA-Y', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Y', relief = FLAT,
                                             value = pointA.getY(),
                                             label_foreground = '#00A000',
                                             entry_width = 9)
            self.cPosY['commandData'] = ['segment-A']
            self.cPosY['command'] = self.setCollisionPosHprScale
            self.cPosY.pack(side=LEFT, expand=0,fill=X, padx=1)

            self.cPosZ = self.createcomponent('pointA-Z', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Z', relief = FLAT,
                                             value = pointA.getZ(),
                                             label_foreground = 'Blue',
                                             entry_width = 9)
            self.cPosZ['commandData'] = ['segment-A']
            self.cPosZ['command'] = self.setCollisionPosHprScale
            self.cPosZ.pack(side=LEFT, expand=0,fill=X, padx=1)
            posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)
            group.pack(side=TOP,fill = X, expand = 0, pady=3)
            group = Pmw.Group(cObjFrame,tag_text='Point B',
                              tag_font=('MSSansSerif', 10))
            posInterior = Frame(group.interior())
            self.cPosXB = self.createcomponent('pointB-X', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'X', relief = FLAT,
                                             value = pointB.getX(),
                                             label_foreground = 'Red',
                                             entry_width = 9)
            self.cPosXB['commandData'] = ['segment-B']
            self.cPosXB['command'] = self.setCollisionPosHprScale
            self.cPosXB.pack(side=LEFT,expand=0,fill=X, padx=1)

            self.cPosYB = self.createcomponent('pointB-Y', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Y', relief = FLAT,
                                             value = pointB.getY(),
                                             label_foreground = '#00A000',
                                             entry_width = 9)
            self.cPosYB['commandData'] = ['segment-B']
            self.cPosYB['command'] = self.setCollisionPosHprScale
            self.cPosYB.pack(side=LEFT, expand=0,fill=X, padx=1)

            self.cPosZB = self.createcomponent('pointB-Z', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Z', relief = FLAT,
                                             value = pointB.getZ(),
                                             label_foreground = 'Blue',
                                             entry_width = 9)
            self.cPosZB['commandData'] = ['segment-B']
            self.cPosZB['command'] = self.setCollisionPosHprScale
            self.cPosZB.pack(side=LEFT, expand=0,fill=X, padx=1)
            posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)
            group.pack(side=TOP,fill = X, expand = 0, pady=3)
            pass
        elif cType == 'CollisionRay':
            origin = self.collisionObj.getOrigin()
            direction = self.collisionObj.getDirection()
            group = Pmw.Group(cObjFrame,tag_text='Origin Point',
                              tag_font=('MSSansSerif', 10))
            posInterior = Frame(group.interior())
            self.cPosX = self.createcomponent('origin-X', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'X', relief = FLAT,
                                             value = origin.getX(),
                                             label_foreground = 'Red',
                                             entry_width = 9)
            self.cPosX['commandData'] = ['ray-A']
            self.cPosX['command'] = self.setCollisionPosHprScale
            self.cPosX.pack(side=LEFT,expand=0,fill=X, padx=1)

            self.cPosY = self.createcomponent('origin-Y', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Y', relief = FLAT,
                                             value = origin.getY(),
                                             label_foreground = '#00A000',
                                             entry_width = 9)
            self.cPosY['commandData'] = ['ray-A']
            self.cPosY['command'] = self.setCollisionPosHprScale
            self.cPosY.pack(side=LEFT, expand=0,fill=X, padx=1)

            self.cPosZ = self.createcomponent('origin-Z', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Z', relief = FLAT,
                                             value = origin.getZ(),
                                             label_foreground = 'Blue',
                                             entry_width = 9)
            self.cPosZ['commandData'] = ['ray-A']
            self.cPosZ['command'] = self.setCollisionPosHprScale
            self.cPosZ.pack(side=LEFT, expand=0,fill=X, padx=1)
            posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)
            group.pack(side=TOP,fill = X, expand = 0, pady=3)
            group = Pmw.Group(cObjFrame,tag_text='Direction',
                              tag_font=('MSSansSerif', 10))
            posInterior = Frame(group.interior())
            self.cPosXB = self.createcomponent('direction-X', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'X', relief = FLAT,
                                             value = direction.getX(),
                                             label_foreground = 'Red',
                                             entry_width = 9)
            self.cPosXB['commandData'] = ['ray-B']
            self.cPosXB['command'] = self.setCollisionPosHprScale
            self.cPosXB.pack(side=LEFT,expand=0,fill=X, padx=1)

            self.cPosYB = self.createcomponent('direction-Y', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Y', relief = FLAT,
                                             value = direction.getY(),
                                             label_foreground = '#00A000',
                                             entry_width = 9)
            self.cPosYB['commandData'] = ['ray-B']
            self.cPosYB['command'] = self.setCollisionPosHprScale
            self.cPosYB.pack(side=LEFT, expand=0,fill=X, padx=1)

            self.cPosZB = self.createcomponent('direction-Z', (), None,
                                             Floater.Floater, (posInterior,),
                                             text = 'Z', relief = FLAT,
                                             value = direction.getZ(),
                                             label_foreground = 'Blue',
                                             entry_width = 9)
            self.cPosZB['commandData'] = ['ray-B']
            self.cPosZB['command'] = self.setCollisionPosHprScale
            self.cPosZB.pack(side=LEFT, expand=0,fill=X, padx=1)
            posInterior.pack(side=TOP, expand=0,fill=X, padx=3, pady=3)
            group.pack(side=TOP,fill = X, expand = 0, pady=3)
            pass

        collisionGroup.pack(side=TOP,fill = X, expand = 0, pady=3)

        return

    def setCollisionPosHprScale(self, data, dataType):
        #################################################################
        # setCollisionPosHprScale(self, data, dataType)
        # Well, the reason that we didn't use the same one with other nodePath
        # is that each tyoe of collsion objects has its unique properties and way to set value.
        # So, they have to be separated from other nodePath
        #################################################################
        if dataType == 'sphere-o':
            origin = Point3(float(self.cPosX._entry.get()),
                            float(self.cPosY._entry.get()),
                            float(self.cPosZ._entry.get()))
            self.collisionObj.setCenter(origin)
        elif dataType == 'sphere-radius':
            self.collisionObj.setRadius(data)
        elif dataType == 'segment-A':
            pointA = Point3(float(self.cPosX._entry.get()),
                            float(self.cPosY._entry.get()),
                            float(self.cPosZ._entry.get()))
            self.collisionObj.setPointA(pointA)
        elif dataType == 'segment-B':
            pointB = Point3(float(self.cPosXB._entry.get()),
                            float(self.cPosYB._entry.get()),
                            float(self.cPosZB._entry.get()))
            self.collisionObj.setPointB(pointB)
        elif dataType == 'ray-A':
            pointA = Point3(float(self.cPosX._entry.get()),
                            float(self.cPosY._entry.get()),
                            float(self.cPosZ._entry.get()))
            self.collisionObj.setOrigin(pointA)
        elif dataType == 'ray-B':
            pointB = Vec3(float(self.cPosXB._entry.get()),
                          float(self.cPosYB._entry.get()),
                          float(self.cPosZB._entry.get()))
            self.collisionObj.setDirection(pointB)
        return

    #################################################################
    #################################################################
    # Functions below are all call back function
    # They will be called when user has manipulated its node on the screen
    # The message itself is sent by a task called monitorSelectedNode in the sceneEditor.
    #################################################################
    #################################################################
    def trackDataFromSceneCamera(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        self.posX.set(pos.getX())
        self.posY.set(pos.getY())
        self.posZ.set(pos.getZ())
        self.hprH.set(hpr.getX())
        self.hprP.set(hpr.getY())
        self.hprR.set(hpr.getZ())
        return

    def trackDataFromSceneModel(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        self.posX.set(pos.getX())
        self.posY.set(pos.getY())
        self.posZ.set(pos.getZ())
        self.hprH.set(hpr.getX())
        self.hprP.set(hpr.getY())
        self.hprR.set(hpr.getZ())
        self.scale.set(scale.getX())
        return

    def trackDataFromSceneActor(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        self.posX.set(pos.getX())
        self.posY.set(pos.getY())
        self.posZ.set(pos.getZ())
        self.hprH.set(hpr.getX())
        self.hprP.set(hpr.getY())
        self.hprR.set(hpr.getZ())
        self.scale.set(scale.getX())
        return

    def trackDataFromSceneLight(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        if self.lightNode.type == 'directional':
            self.dPosition.set([pos.getX(),pos.getY(),pos.getZ()])
            self.dOrientation.set([hpr.getX(),hpr.getY(),hpr.getZ()])
            pass

        elif self.lightNode.type == 'point':
            self.pPosition.set([pos.getX(),pos.getY(),pos.getZ()])
            pass
        return

    def trackDataFromSceneDummy(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        self.posX.set(pos.getX())
        self.posY.set(pos.getY())
        self.posZ.set(pos.getZ())
        self.hprH.set(hpr.getX())
        self.hprP.set(hpr.getY())
        self.hprR.set(hpr.getZ())
        self.scale.set(scale.getX())
        return

    def trackDataFromSceneCollision(self, pos=Point3(0,0,0), hpr=Vec3(0,0,0), scale=Point3(0,0,0)):
        self.posX.set(pos.getX())
        self.posY.set(pos.getY())
        self.posZ.set(pos.getZ())
        self.hprH.set(hpr.getX())
        self.hprP.set(hpr.getY())
        self.hprR.set(hpr.getZ())
        self.scale.set(scale.getX())
        return

