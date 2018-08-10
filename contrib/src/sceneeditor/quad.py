#########################################################################################################################################
# This file implements a Quad View for the level editor
# This feature is not yet enabled in the level editor because picking objects in quad view doesnt work
# I have tried to send the picking function in seSelection.py the correct camera and mouse coordinates but something seems to be wron
# There are two classes in this file..the QuadView and the viewPort...there are four instances of viewport, one for each view in QuadView
#########################################################################################################################################


from direct.showbase.ShowBaseGlobal import *
from direct.interval.IntervalGlobal import *
from direct.showbase.DirectObject import DirectObject
from panda3d.core import *
import math


class ViewPort:
#########################################################################################################################################
# The ViewPort class has the camera and associated display region set up for actually rendering the four sub-views
# The constructor needs the bounds, window layer, camera, color, projection type, name and scene for the view
#########################################################################################################################################

    def __init__(self,X1,X2,Y1,Y2,layer,cam,background=Vec4(0.3,0.3,0.3,1),projection="perspective",type="top",scene=render):
        self.VPType=type
        self.VP_X1=X1
        self.VP_Y1=Y1
        self.VP_X2=X2
        self.VP_Y2=Y2
        self.VP_width=self.VP_X2 - self.VP_X1
        self.VP_height=self.VP_Y2 - self.VP_Y1

        self.the_viewport=layer.makeDisplayRegion(self.VP_X1, self.VP_X2,self.VP_Y1, self.VP_Y2)
        self.the_viewport.setCamera(cam)
        self.the_viewport.setClearDepthActive(1)
        self.the_viewport.setClearColorActive(1)
        self.the_viewport.setClearColor(background)
        self.cam=cam


        # Set up the cameras to look in the right place.
        if(type=="top"):
            self.cam.setP(-90)
            self.cam.setZ(-40)
        elif(type=="left"):
            self.cam.setH(-90)
            self.cam.setX(10)
        elif(type=="front"):
            self.cam.setY(-10)
        elif(type=="perspective"):
            cam.setY(-100)
            #cam.setX(10)
            #cam.setZ(-10)
            #cam.setH(45)
            #cam.setP(-45)
            #print "aa"


        if(projection=="ortho"):
            self.lens=OrthographicLens()
            self.lens.setAspectRatio((self.VP_X2-self.VP_X1)/(self.VP_Y2-self.VP_Y1))
            self.lens.setFilmSize(self.VP_width*200,self.VP_height*200)
            #lens.setFilmOffset((self.VP_X2 + self.VP_X1) * 0.5, (self.VP_Y2 + self.VP_Y1) * 0.5)
            self.lens.setNearFar(-1000, 1000)
            self.cam.node().setLens(self.lens)
            self.cam.node().setScene(scene)
        elif(projection=="perspective"):
            self.lens=base.cam.node().getLens()
            self.lens.setAspectRatio((self.VP_X2-self.VP_X1)/(self.VP_Y2-self.VP_Y1))
            self.cam.node().setLens(self.lens)
            self.cam.node().setScene(scene)

        self.the_viewport.setCamera(self.cam)

    def resizeX(self,width_increment):
        if(self.VPType=="top" or self.VPType=="left"):
            self.the_viewport.setDimensions(self.VP_X1,self.VP_X2+width_increment,self.VP_Y1,self.VP_Y2)
        elif(self.VPType=="perspective" or self.VPType=="front"):
            self.the_viewport.setDimensions(self.VP_X1+width_increment,self.VP_X2,self.VP_Y1,self.VP_Y2)

    def resizeY(self,height_increment,direction):
        if(self.VPType=="left" or self.type=="perspective"):
            self.the_viewport.setDimensions(self.VP_X1,self.VP_X2,self.VP_Y1,self.VP_Y2+height_increment)
        else:
            self.the_viewport.setDimensions(self.VP_X1,self.VP_X2,self.VP_Y1+height_increment,self.VP_Y2)

    def AdjustAspect(self,x,y):
        if (y==0):
            y=1
        self.lens.setAspectRatio(x/y)
        self.cam.node().setLens(self.lens)

    def resize(self,x,y):

        if(self.VPType=="left"):
            self.the_viewport.setDimensions(0,x,0,y)
            w=abs(x-self.VP_X1)
            h=abs(y-self.VP_Y1)
            if(h==0):
                h=1
            self.lens.setAspectRatio(w/h)
            self.cam.node().setLens(self.lens)
        if(self.VPType=="top"):
            self.the_viewport.setDimensions(0,x,y,1)
            w=abs(x-self.VP_X1)
            h=abs(self.VP_Y2-y)
            if(h==0):
                h=1
            self.lens.setAspectRatio(w/h)
            self.cam.node().setLens(self.lens)
        if(self.VPType=="front"):
            self.the_viewport.setDimensions(x,1,y,1)
            w=abs(self.VP_X2-x)
            h=abs(self.VP_Y2-y)
            if(h==0):
                h=1
            self.lens.setAspectRatio(w/h)
            self.cam.node().setLens(self.lens)
        if(self.VPType=="perspective"):
            self.the_viewport.setDimensions(x,1,0,y)
            w=abs(self.VP_X2-x)
            h=abs(y-self.VP_Y1)
            if(h==0):
                h=1
            self.lens.setAspectRatio(w/h)
            self.cam.node().setLens(self.lens)

    def setScene(self,scene):
        self.cam.node().setScene(scene)

    def setDR(self,mouseWatcher):
        #mouseWatcher.setDisplayRegion(self.the_viewport)
        pass

    def setCam(self):
        #base.cam=self.cam
        #base.cam.node().setLens(self.cam.node().getLens())
        base.camNode=self.cam.node()
        #base.camNode.setLens(self.cam.node().getLens())
        #base.camLens=self.cam.node().getLens()

    def getCam(self):
        return self.cam


class QuadView(DirectObject):
#########################################################################################################################################
# This class sets up four cameras for the scene (ideally we want four instances of render too)
# and then instatiates a ViewPort class for each of them
#
#########################################################################################################################################

    def __init__(self):

        self.PTracker=1
        self.ControlPressed=0
        self.AltPressed=0
        self.PanConstantX=50
        self.PanConstantY=50
        self.ZoomConstant=1
        self.FrontWidth=100
        self.FrontHeight=100
        self.TopWidth=100
        self.TopHeight=100
        self.LeftWidth=100
        self.LeftHeight=100

        self.MouseButton=0
        self.CurrentQuad=4
        self.HorizontalAxis=0.0
        self.VerticalAxis=0.0
        #base.disableMouse()
        self.MouseDragging=0
        self.currX= 0
        self.oldX=self.currX
        self.currY= 0
        self.oldY=self.currY

        self.FrontTexture=1
        self.LeftTexture=1
        self.PerspectiveTexture=1
        self.TopTexture=1

        self.FrontWire=0
        self.LeftWire=0
        self.PerspectiveWire=0
        self.TopWire=0

        # Keep track of the currently selected window... values are 1-4 for four quadrants of a standard
        # Cartesian coordinate system

        # These are the orthographic cameras
        # They will be restricted to panning and zooming i.e. no rotation
        # Top could be flipped to back, left to right and front to back
        self.topCam= render.attachNewNode(Camera('topCam'))
        self.frontCam = render.attachNewNode(Camera('frontCam'))
        self.leftCam= render.attachNewNode(Camera('leftCam'))

        # This camera will have a trackball control since its perspective
        self.perspectiveCam = render.attachNewNode(Camera('perspectiveCam'))

        #self.toplens=OrthographicLens()
        #self.leftLens=OrthographicLens()
        #self.frontLens=OrthographicLens()
        #self.perspectiveLens=base.cam.node().getLens()

        # For now all lenses are same as that of base.cam
        #self.topCamLens=OrthographicLens()
        #self.frontCamLens= base.cam.node().getLens()
        #self.leftCamLens= base.cam.node().getLens()
        #self.perspectiveCamLens= base.cam.node().getLens()

        # Manipulate lenses here if need be
        #self.topCamLens.setFilmSize(250)

        # Set the Lenses
        #self.topCam.node().setLens(self.topCamLens)
        #self.frontCam.node().setLens(self.frontCamLens)
        #self.leftCam.node().setLens(self.leftCamLens)
        #self.perspectiveCam.node().setLens(self.perspectiveCamLens)

        #self.badwiz = loader.loadModel('badwizard1')
        #self.badwiz.reparentTo(render)

        # Create four separate display regions for the quad view.
        # These will overlap the main display region
        # To stack these overlapping DisplayRegions, we need a new layer.  If
        # they didn't overlap, we could put them in the same layer.

        self.newLayer = base.win.getChannel(0).makeLayer()

        self.PerspectiveScene=NodePath('PerspectiveScene')
        self.FrontScene=NodePath('FrontScene')
        self.TopScene=NodePath('TopScene')
        self.LeftScene=NodePath('LeftScene')
        self.SceneParent=NodePath('SceneParent')

        #self.PerspectiveScene=render.copyTo(self.SceneParent)
        #self.FrontScene=render.copyTo(self.SceneParent)
        #self.TopScene=render.copyTo(self.SceneParent)
        #self.LeftScene=render.copyTo(self.SceneParent)

        self.PerspectiveScene=render
        self.FrontScene=render
        self.TopScene=render
        self.LeftScene=render

        #self.PerspectiveScene.reparentTo(self.SceneParent)
        #self.FrontScene.reparentTo(self.SceneParent)
        #self.TopScene.reparentTo(self.SceneParent)
        #self.LeftScene.reparentTo(self.SceneParent)

        self.Perspective=ViewPort(0.5,1.0,0.0,0.5,self.newLayer,self.perspectiveCam,Vec4(0.75,0.75,0.75,1),"perspective","perspective",self.PerspectiveScene)
        self.Top=ViewPort(0.0,0.5,0.5,1.0,self.newLayer,self.topCam,Vec4(0.80,0.80,0.80,1),"ortho","top",self.TopScene)
        self.Left=ViewPort(0.0,0.5,0.0,0.5,self.newLayer,self.leftCam,Vec4(0.85,0.85,0.85,1),"ortho","left",self.LeftScene)
        self.Front=ViewPort(0.5,1.0,0.5,1.0,self.newLayer,self.frontCam,Vec4(0.85,0.85,0.85,1),"ortho","front",self.FrontScene)
        #self.Perspective=None
        #self.Top=None
        #self.Front=None
        #self.Left=None

        #self.raycaster = RayCaster( camera )
        #self.lastPickPoint = None

        #base.useTrackball()

        #self.dataRoot = NodePath('dataRoot')
        # Cache the node so we do not ask for it every frame
        #self.dataRootNode = self.dataRoot.node()
        #self.dataUnused = NodePath('dataUnused')
        #self.mak=None
        #self.mak = self.dataRoot.attachNewNode(MouseAndKeyboard(base.win, 0, 'mak'))
        #self.mak.node().setSource(base.win, 0)
        self.mouseWatcherNode = MouseWatcher('mouseWatcher')

        self.mouseWatcher = base.mak.attachNewNode(self.mouseWatcherNode)
        #self.Perspective.setDR(self.mouseWatcherNode)

        self.buttonThrower = self.mouseWatcher.attachNewNode(ButtonThrower('buttons'))


        #ddr=DisplayRegionContext(self.Perspective.getCam())
        #base.setMouseOnNode(self.smiley.node()) # Let Mouse Control Perspective View for now
        #base.enableSoftwareMousePointer()

        # Message Handlers
        self.accept("a",self.setLeft)
        self.accept("q",self.setTop)
        self.accept("w",self.setFront)
        self.accept("s",self.setPerspective)
        self.accept("mouse1",self.MouseTell,[1])
        self.accept("mouse2",self.MouseTell,[2])
        self.accept("mouse3",self.MouseTell,[3])
        self.accept("mouse1-up",self.MouseTellUp,[4])
        self.accept("mouse2-up",self.MouseTellUp,[5])
        self.accept("mouse3-up",self.MouseTellUp,[6])
        self.accept("mouse2-scroll",self.resizedr)
        self.accept("r",self.resizedr)
        self.accept("alt",self.AltHandler)
        self.accept("alt-up",self.AltUpHandler)
        self.accept("alt-mouse1",self.AltDown)
        self.accept("alt-mouse1-up",self.AltUp)
        self.accept("control-mouse1",self.CtlDown)
        self.accept("control-mouse1-up",self.CtlUp)

    # Methods

    #def setLastPickPoint( self ):
    #    mouseX, mouseY = self.mouseWatcherNode.getMouseX(), self.mouseWatcherNode.getMouseY()
    #    self.lastPickPoint = self.raycaster.pick( mouseX, mouseY )
    #    print self.lastPickPoint

    def AltDown(self):
        self.AltPressed=1


    def AltUp(self):
        self.AltPressed=0


    def CtlDown(self):
        self.ControlPressed=1


    def CtlUp(self):
        self.ControlPressed=0


    def ToggleWire(self):
        if (self.CurrentQuad==1): # Front View
            if(self.FrontWire): # Wireframe is On so turn it off
                self.FrontScene.setRenderModeWireframe(100)
                self.FrontScene.setTwoSided(1)
                self.FrontScene.setTextureOff(100)
                self.FrontWire=0
            else:
                self.FrontScene.clearRenderMode()
                #self.FrontScene.setTwoSided(not self.backfaceCullingEnabled)
                if(self.FrontTexture):
                    self.FrontScene.clearTexture()
                self.FrontWire=1
        elif (self.CurrentQuad==2): # Front View
            if(self.TopWire): # Wireframe is On so turn it off
                self.TopScene.setRenderModeWireframe(100)
                self.TopScene.setTwoSided(1)
                self.TopScene.setTextureOff(100)
                self.TopWire=0
            else:
                self.TopScene.clearRenderMode()
                #self.TopScene.setTwoSided(not self.backfaceCullingEnabled)
                if(self.TopTexture):
                    self.TopScene.clearTexture()
                self.TopWire=1
        elif (self.CurrentQuad==3): # Front View
            if(self.LeftWire): # Wireframe is On so turn it off
                self.LeftScene.setRenderModeWireframe(100)
                self.LeftScene.setTwoSided(1)
                self.LeftScene.setTextureOff(100)
                self.LeftWire=0
            else:
                self.LeftScene.clearRenderMode()
                #self.LeftScene.setTwoSided(not self.backfaceCullingEnabled)
                if(self.LeftTexture):
                    self.LeftScene.clearTexture()
                self.LeftWire=1
        elif (self.CurrentQuad==4): # Front View
            if(self.PerspectiveWire): # Wireframe is On so turn it off
                self.PerspectiveScene.setRenderModeWireframe(100)
                self.PerspectiveScene.setTwoSided(1)
                self.PerspectiveScene.setTextureOff(100)
                self.PerspectiveWire=0
            else:
                self.PerspectiveScene.clearRenderMode()
                #self.PerspectiveScene.setTwoSided(not self.backfaceCullingEnabled)
                if(self.PerspectiveTexture):
                         self.PerspectiveScene.clearTexture()
                self.PerspectiveWire=1


    def ToggleTexture(self):
        if (self.CurrentQuad==1): # Front View
            if(self.FrontTexture): # Texture is on so turn it off
                self.FrontScene.setTextureOff(100)
                self.FrontTexture=0
            else:
                self.FrontScene.clearTexture()
                self.FrontTexture=1
        elif (self.CurrentQuad==2): # Top View
            if(self.TopTexture): # Texture is on so turn it off
                self.TopScene.setTextureOff(100)
                self.TopTexture=0
            else:
                self.TopScene.clearTexture()
                self.TopTexture=1
        elif (self.CurrentQuad==3): # Left View
            if(self.LeftTexture): # Texture is on so turn it off
                self.LeftScene.setTextureOff(100)
                self.LeftTexture=0
            else:
                self.LeftScene.clearTexture()
                self.LeftTexture=1
        elif (self.CurrentQuad==4): # Perspective View
            if(self.PerspectiveTexture): # Texture is on so turn it off
                self.PerspectiveScene.setTextureOff(100)
                self.PerspectiveTexture=0
            else:
                self.PerspectiveScene.clearTexture()
                self.PerspectiveTexture=1



    def reparenter(self):
        #self.FrontScene.reparentTo(render)
        #self.Front.setScene(render)
        #self.Top.setScene(render)
        #self.Left.setScene(render)
        #self.Perspective.setScene(render)
        pass

    def unparenter(self):
        #self.PerspectiveScene=render.copyTo(render)
        #self.FrontScene=render.copyTo(render)
        #self.TopScene=render.copyTo(render)
        #self.LeftScene=render.copyTo(render)
        #self.SceneParent.reparentTo(render)
        #self.PerspectiveScene.reparentTo(self.SceneParent)
        #self.FrontScene.reparentTo(self.SceneParent)
        #self.TopScene.reparentTo(self.SceneParent)
        #self.LeftScene.reparentTo(self.SceneParent)
        pass

    def AltHandler(self):
        self.oldX=self.mouseWatcherNode.getMouseX()
        if(self.oldX<-1 or self.oldX>1):
            return
        self.oldY=self.mouseWatcherNode.getMouseY()
        if(self.oldY<-1 or self.oldY>1):
            return
        taskMgr.add(self.DragAction,'DragAction')

    def AltUpHandler(self):
        taskMgr.remove('DragAction')

    def gridtoggle(self):
        #grid=DirectGrid()
        #grid.enable()
        pass

    def resizedr(self,x,y):
        #print "X: " + str(x) + " Y: " + str(y)
        x=(x+1)/2.0
        y=(y+1)/2.0
        self.Perspective.resize(x,y)
        self.Top.resize(x,y)
        self.Front.resize(x,y)
        self.Left.resize(x,y)

    def setAppropriateViewPort(self,x,y):
        #print "SET APPROPRIATE:" + str(x) + " " + str(y)
        if(x<self.VerticalAxis):
            if(y<self.HorizontalAxis):
                self.setLeft()
            else:
                self.setTop()
        else:
            if(y<self.HorizontalAxis):
                self.setPerspective()
            else:
                self.setFront()

    def MouseTell(self,buttonCode):
        self.MouseButton=buttonCode
        self.setAppropriateViewPort(self.mouseWatcherNode.getMouseX(),self.mouseWatcherNode.getMouseY())

        x=base.mouseWatcherNode.getMouseX()
        y=base.mouseWatcherNode.getMouseY()

        #Perspective and Front
        if(self.CurrentQuad==4 or self.CurrentQuad==1):
            x1=abs(x-self.VerticalAxis)
            w1=abs(1-self.VerticalAxis)
            x2=x1*2.0/w1
            ansX=-1+x2


        #Left and top
        if(self.CurrentQuad==2 or self.CurrentQuad==3):
            x1=abs(x-(-1.0))
            w1=abs(self.VerticalAxis-(-1.0))
            x2=x1*2.0/w1
            ansX=-1.0+x2

        #Left and Perspective
        if(self.CurrentQuad==4 or self.CurrentQuad==3):
            y1=abs(y-(-1.0))
            h1=abs(self.HorizontalAxis-(-1.0))
            y2=y1*2.0/h1
            ansY=-1.0+y2


        #Front and top
        if(self.CurrentQuad==1 or self.CurrentQuad==2):
            y1=abs(y-self.HorizontalAxis)
            h1=abs(1.0-self.HorizontalAxis)
            y2=y1*2.0/h1
            ansY=-1.0+y2

        self.xy=[ansX,ansY]
        print("Sent X:%f Sent Y:%f"%(ansX,ansY))
        #SEditor.iRay.pick(render,self.xy)
        SEditor.manipulationControl.manipulationStop(self.xy)
        #print "MouseX " + str(base.mouseWatcherNode.getMouseX()) + "MouseY " + str(base.mouseWatcherNode.getMouseY()) + "\n"
        #print "MouseX " + str(self.mouseWatcherNode.getMouseX()) + "MouseY " + str(self.mouseWatcherNode.getMouseY()) + "\n"


        base.mouseWatcherNode=self.mouseWatcherNode

        self.oldX=self.mouseWatcherNode.getMouseX()
        if(self.oldX<-1 or self.oldX>1):
            return

        self.oldY=self.mouseWatcherNode.getMouseY()
        if(self.oldY<-1 or self.oldY>1):
            return
        self.Mouse_Dragging=1
        taskMgr.add(self.DragAction,'DragAction')

    def MouseTellUp(self,buttoncode):
        #self.MouseButton=0
        self.PanConstantX= 50
        self.PanConstantY= 50
        self.ZoomConstant=1
        taskMgr.remove('DragAction')
        self.Mouse_Draggin=0
        #print "Mouse Up"


    def Max_Style_Mouse_View(self,buttoncode):
        pass

    def ChangeBaseDR(self):
        dr=base.win.getDisplayRegion(0)
        if(self.CurrentQuad==1): #Front
            dr.setDimensions(0.5,1,0.5,1)
        elif(self.CurrentQuad==2): #Top
            dr.setDimensions(0,0.5,0.5,1)
        elif(self.CurrentQuad==3): #Left
            dr.setDimensions(0,0.5,0,0.5)
        elif(self.CurrentQuad==4): #Perspective
            dr.setDimensions(0.5,1,0,0.5)

    def setLeft(self):
        print("LEFT")
        self.CurrentQuad=3
        self.ChangeBaseDR()
        self.Left.setCam()
        #self.Left.setDR(self.mouseWatcherNode)

    def setTop(self):
        print("TOP")
        self.CurrentQuad=2
        self.ChangeBaseDR()
        self.Top.setCam()
        #self.Top.setDR(self.mouseWatcherNode)

    def setPerspective(self):
        print("PERSPECTIVE")
        self.CurrentQuad=4
        self.ChangeBaseDR()
        self.Perspective.setCam()
        #self.Perspective.setDR(self.mouseWatcherNode)

    def setFront(self):
        print("FRONT")
        self.CurrentQuad=1
        self.ChangeBaseDR()
        self.Front.setCam()
        #self.Front.setDR(self.mouseWatcherNode)


    def DragAction(self,task):
        #if(self.MouseDragging==1):
        self.currX= self.mouseWatcherNode.getMouseX()
        if(self.currX<-1 or self.currX>1):
            return
        self.currY= self.mouseWatcherNode.getMouseY()
        if(self.currY<-1 or self.currY>1):
            return


        self.diffX=self.currX-self.oldX
        self.diffY=self.currY-self.oldY

        if(self.ControlPressed): # Change Size of the ViewPorts
        #if(base.getControl()):
             self.VerticalAxis=self.currX
             self.HorizontalAxis=self.currY
             if(self.HorizontalAxis<-1 or self.HorizontalAxis>1 or self.VerticalAxis<-1 or self.VerticalAxis>1):
                 return
             self.resizedr(self.VerticalAxis,self.HorizontalAxis)

        #if(self.AltPressed): # View Camera Transforms -> Maya style
        elif(1):
            #print "ALTPRESSED"
            if(self.PanConstantX<4096):
                self.PanConstantX= self.PanConstantX * 2
                self.PanConstantY= self.PanConstantY * 2
            self.ZoomConstant= self.ZoomConstant + 50
            if(self.MouseButton==1): # TrackBall rotation only for Perspective View
                if(self.CurrentQuad==4):
                    pass
            elif(self.MouseButton==2): # Do Panning
                if(self.CurrentQuad==1): # Y and Z values change meanings for different cameras
                    self.MoveCamera(-self.diffX*self.PanConstantX,0,-self.diffY*self.PanConstantY,self.CurrentQuad)
                elif(self.CurrentQuad==2):
                    self.MoveCamera(-self.diffX*self.PanConstantX,-self.diffY*self.PanConstantY,0,self.CurrentQuad)
                elif(self.CurrentQuad==3):
                    self.MoveCamera(0,self.diffX*self.PanConstantX,-self.diffY*self.PanConstantY,self.CurrentQuad)
                elif(self.CurrentQuad==4):
                    pass
            elif(self.MouseButton==3): # Do Zoom
                if(self.CurrentQuad==1): # Y and Z values change meanings for different cameras
                    #lens = OrthographicLens()
                    #lens.setFilmSize(l,self.VP_height*200)
                    #lens.setFilmOffset((self.VP_X2 + self.VP_X1) * 0.5, (self.VP_Y2 + self.VP_Y1) * 0.5)
                    #lens.setNearFar(-1000, 1000)
                    self.FrontWidth= self.FrontWidth + self.diffX
                    self.FrontHeight= self.FrontHeight + self.diffX
                    self.FrontWidth= self.FrontWidth + self.diffY
                    self.FrontHeight= self.FrontHeight + self.diffY

                    if(self.FrontWidth<=0):
                        Frontwidth=1
                    if(self.FrontHeight<=0):
                        FrontHeight=1
                    self.frontCam.node().getLens().setFilmSize(self.FrontWidth,self.FrontHeight)
                    self.resizedr(self.VerticalAxis,self.HorizontalAxis)
                elif(self.CurrentQuad==2):
                    self.TopWidth= self.TopWidth + self.diffX
                    self.TopHeight= self.TopHeight + self.diffX
                    self.TopWidth= self.TopWidth + self.diffY
                    self.TopHeight= self.TopHeight + self.diffY
                    self.topCam.node().getLens().setFilmSize(self.TopWidth,self.TopHeight)
                    self.resizedr(self.VerticalAxis,self.HorizontalAxis)
                elif(self.CurrentQuad==3):
                    self.LeftWidth= self.LeftWidth + self.diffX
                    self.LeftHeight= self.LeftHeight + self.diffX
                    self.LeftWidth= self.LeftWidth + self.diffY
                    self.LeftHeight= self.LeftHeight + self.diffY
                    self.leftCam.node().getLens().setFilmSize(self.LeftWidth,self.LeftHeight)
                    self.resizedr(self.VerticalAxis,self.HorizontalAxis)
                elif(self.CurrentQuad==4):
                    pass
        else:
             pass

        self.oldX=self.currX
        self.oldY=self.currY
        return Task.cont



    def MoveCamera(self,X_amt,Y_amt,Z_amt,quad):
        if(quad==1):
            self.frontCam.setPos(self.frontCam.getX()+X_amt,self.frontCam.getY()+Y_amt,self.frontCam.getZ()+Z_amt)
        elif(quad==2):
            self.topCam.setPos(self.topCam.getX()+X_amt,self.topCam.getY()+Y_amt,self.topCam.getZ()+Z_amt)
        elif(quad==3):
            self.leftCam.setPos(self.leftCam.getX()+X_amt,self.leftCam.getY()+Y_amt,self.leftCam.getZ()+Z_amt)
        elif(quad==4):
            self.perspectiveCam.setPos(self.perspectiveCam.getX()+X_amt,self.perspectiveCam.getY()+Y_amt,self.perspectiveCam.getZ()+Z_amt)



#View=QuadView()
#run()
