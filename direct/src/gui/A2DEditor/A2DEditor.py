__all__ = ['AutoSaveAspect2d']

from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.interval.IntervalGlobal import *
from direct.gui.OnscreenText import OnscreenText
from direct.gui.DirectGui import *
from direct.showbase import PythonUtil as PU
from direct.task import Task
import os, math, random, cPickle, types
import PauseResume as PR
# collidable-button system
import PolyButton
# collection of some Direct GUI extension classes
from DGUI_extensions import *
# collapsible & expandable scene graph hierarchy tree
from SceneGraphBrowser import *
# I can't work around 1.6's new Messenger, which insists in adding an attribute (_messengerId)
# to every event receiver object. I need all object types able to receive events,
# including C++ objects, so 1.6's Messenger can't serve this purpose.
# So, myMessenger module imported here is 1.5.3's.
from myMessenger import Messenger


MASK_off=BitMask32.allOff()
PolyButton.setup(startNow=False)
getModelPath().appendPath(os.path.dirname(__file__))


class AutoSaveAspect2d(DirectObject):
  MODE_a2dSelect='a2dSelect'
  MODE_a2dEdit='a2dEdit'
  BINext='.bin'    # binary extension
  ASCIIext='.abc'  # ASCII extension
  # load other font
  transMtl=loader.loadFont('Transmetals.ttf')
  transMtl.setLineHeight(.9)
  # check buttons
  nodesListChkBtns=[]
  # new render bins
  CBM=CullBinManager.getGlobalPtr()
  CBM.addBin('selectedNP',CullBinEnums.BTBackToFront,100)
  CBM.addBin('origin',CullBinEnums.BTBackToFront,150)
  CBM.addBin('snapTarget0',CullBinEnums.BTUnsorted,200)
  CBM.addBin('snapTarget1',CullBinEnums.BTUnsorted,210)
  CBM.addBin('snapTarget2',CullBinEnums.BTUnsorted,220)
  CBM.addBin('snapTarget3',CullBinEnums.BTUnsorted,230)
  CBM.addBin('snapTarget4',CullBinEnums.BTUnsorted,240)
  CBM.addBin('snapTarget5',CullBinEnums.BTUnsorted,250)
  CBM.addBin('snapTarget6',CullBinEnums.BTUnsorted,260)
  CBM.addBin('snapPoint',CullBinEnums.BTUnsorted,300)
  CBM.addBin('editingGUI',CullBinEnums.BTBackToFront,400)
  CBM.addBin('XZcoord',CullBinEnums.BTBackToFront,500)
  # get the scale of aspect2d to restore it later
  a2dScale=aspect2d.getScale()
  # create a new sibling of aspect2d with the same scale,
  # which never change over time, used to lock the scale of objects on aspect2d
  # which shouldn't scaled up or down with aspect2d
  a2dNormalScaled=render2d.attachNewNode('',sort=-1000)
  a2dNormalScaled.setScale(a2dScale)
  # create a fake render2d under aspect2d, and undone aspect2d X stretch,
  # used as reference node when typing-in the relative transform using the panel
  fakeRender2d=aspect2d.attachNewNode('fakeR2D')
  fakeRender2d.setScale(1/a2dScale[0],1/a2dScale[1],1/a2dScale[2])
  fakeRender2d.setTag('A2Dedit component','')
  # determine window aspect ratio
  winProps=base.winList[0].getProperties()
  winXsize=winProps.getXSize()
  winYsize=winProps.getYSize()
  winXmaxHalf=(winXsize-1)*.5
  winYmaxHalf=(winYsize-1)*.5
  aspRatio=float(winXsize)/winYsize
  # zoom factor
  zoomFactor=1.15
  # pos & scale compass effect
  PosScale_LOCK = CompassEffect.PPos|CompassEffect.PScale
  # DirectCheckButton indicator text
  DCB_cross=(' ','x')
  DCB_bullet=(' ','.')
  # I want every marker type colored independently and consistently, but
  # since a marker might be parented to a node which has :
  #     1. color & alpha scale
  #     2. applied texture on-the-fly,
  # I disactivate the propagating color (setColorScaleOff) and
  # texture (setTextureOff) from the parent,
  # and use values at geom level instead
  markerAlpha =.75  # transparency value for markers' geom
  markerSize = 32
  # edit session collision bitmask
  A2DeditMaskBit=30
  A2DeditMask = BitMask32.bit(A2DeditMaskBit)
  # scale anchor marker name
  anchorMarkerName={'top':'bottom',
                    'left':'right',
                    'right':'left',
                    'bottom':'top',
                    'topLeft':'bottomRight',
                    'topRight':'bottomLeft',
                    'bottomLeft':'topRight',
                    'bottomRight':'topLeft'}

  def __init__(self, myFile, enterEditSessionKey='f5',
               loadASCII=1, saveBinaryToo=1, enableEditSession=1):
      """
          enterEditSessionKey : keyboard button to trigger edit session

          loadASCII     : 1 = use ASCII file (default)
                          0 = use binary file instead

          saveBinaryToo : 1 = also save as binary file
                          0 = only save the ASCII file

          enableEditSession : 1 = enabled
                              0 = disabled
      """
      self.send2GUIpopupBin(self.a2dNormalScaled)

      self.EditSessionEnabled = enableEditSession
      self.enterEditSessionKey = enterEditSessionKey
      self.saveBinaryToo = saveBinaryToo
      # dictionary for objects' transform
      self.a2dDict={}
      # load transform dictionary from disk
      self.myFile=myFile
      if loadASCII:
         # load ASCII file
         if not self.loadASCIIFile():
            print 'WARNING : could not load ASCII file'
            print '....looking for binary file'
            # load binary file
            if not self.loadBinaryFile():
               print 'WARNING : could not load both ASCII and binary files'
               print '....USING TRANSFORM FROM SCRIPT ONLY'
      else:
         # load binary file
         if not self.loadBinaryFile():
            print 'WARNING : could not load binary file'
            print '....looking for ASCII file'
            # load ASCII file
            if not self.loadASCIIFile():
               print 'WARNING : could not load both binary and ASCII files'
               print '....USING TRANSFORM FROM SCRIPT ONLY'

      # edit session not enabled, get outta here
      if not self.EditSessionEnabled:
         return

      self.__keyBindings()
      self.__createEditSessionLegend()
      self.__createEditNotification()
      self.__setupA2Dcollision()
      self.__createParentsListGUI()
      self.__createA2DscenegraphTreeGUI()
      self.__createNodePropertiesGUI()
      self.__createCopyPasteNotification()
      self.__createInvalidInputNotification()
      self.__createOrthoAxis()
      self.__createNodeOriginVis()
      self.__createXZsnapGrids()

      self.__createSnapMarkers()
      self.__createSnapMarkerPoint()
      self.__createSnapExtensionLines()
      self.__createIntersectionMarker()

      array = GeomVertexArrayFormat()
      array.addColumn(InternalName.make('vertex'), 3, Geom.NTFloat32, Geom.CPoint)
      array.addColumn(InternalName.make('color'), 4, Geom.NTFloat32, Geom.CColor)
      array.addColumn(InternalName.make('texcoord'), 2, Geom.NTFloat32, Geom.CTexcoord)
      format = GeomVertexFormat()
      format.addArray(array)
      self.BBoxGeomVertexFormat= GeomVertexFormat.registerFormat(format)
      self.BBoxTex=loader.loadTexture('markers/dash.png')

  #____________END_OF___I N I T__________________________________



  def __setupA2Dcollision(self):
      # setup collision against aspect2d
      self.A2DeditMouseColNP = self.a2dMouseLoc.attachCollisionRay('buttonCollMouseRay',
           0,-100,0, 0,1,0, self.A2DeditMask,MASK_off)
      self.A2DeditMousecTrav = CollisionTraverser()
      self.A2DeditMousecQueue = CollisionHandlerQueue()

  def __keyBindings(self):
      # key binding
      base.buttonThrowers[0].node().setButtonDownEvent('buttonDown')
      self.listenButtonDown(1)

      self.modes=[self.MODE_a2dSelect]
      self.acceptEvents('space',                  self.toggleSelectEdit,[0])
      self.acceptEvents('escape',                 self.exit)
      # exit & save changes now uses dialog,
      # but if you like 1 button save & exit, enable the next line.
      #self.acceptEvents(self.enterEditSessionKey, self.exitA2DeditSession,[1])
      self.acceptEvents('home',                   self.gotoBeginHistorySelectMode)
      self.acceptEvents('end',                    self.gotoEndHistorySelectMode)
      self.acceptEvents('caps_lock',              self.toggleNodesList)
      self.acceptEvents( (
                         'arrow_up',
                         'w',
                         ), self.cycleNode,[-1,0])
      self.acceptEvents( (
                         'arrow_up-repeat',
                         'w-repeat',
                         ), self.cycleNode,[-1,.1])
      self.acceptEvents( (
                         'arrow_down',
                         's',
                         ), self.cycleNode,[1,0])
      self.acceptEvents( (
                         'arrow_down-repeat',
                         's-repeat',
                         ), self.cycleNode,[1,.1])
      self.acceptEvents('z',                      self.redoSelectMode)
      self.acceptEvents('control-z',              self.undoSelectMode)

      self.modes=[self.MODE_a2dEdit]
      self.acceptEvents( (
                         'mouse1',
                         'shift-mouse1',
                         ), self.startDrag)
      self.acceptEvents('control',          self.hideNonScalingSelfMarkers)
      self.acceptEvents('control-up',       self.showNonScalingSelfMarkers)
      self.acceptEvents('control-mouse1',   self.startScale)
      self.acceptEvents('mouse1-up',        self.stopDragScale)
      self.acceptEvents( (
                         'mouse3',
                         'control-mouse3',
                         ), self.markSnapMarker)
      self.acceptEvents('home',             self.gotoBeginHistoryEditMode)
      self.acceptEvents('end',              self.gotoEndHistoryEditMode)
      self.acceptEvents('escape',           self.cancelDragScale)
      self.acceptEvents('space',            self.toggleSelectEdit,[1])
      self.acceptEvents( (
                         'shift',
                         'shift-up',
                         ), self.toggleOrtho)
      self.acceptEvents('h',                self.toggleEditNotifier)
      self.acceptEvents('z',                self.redoEditMode)
      self.acceptEvents('control-z',        self.undoEditMode)

      self.modes=[self.MODE_a2dSelect,self.MODE_a2dEdit]
      self.acceptEvents('mouse2',           self.startDragA2D)
      self.acceptEvents('mouse2-up',        self.stopDragA2D)
      self.acceptEvents('wheel_up',         self.zoomInA2D)
      self.acceptEvents('wheel_down',       self.zoomOutA2D)
      self.acceptEvents('a',                self.zoomNormalA2D)
      self.acceptEvents('f',                self.focusViewOnNode)
      self.acceptEvents('f1',               self.toggleEditLegend)
      self.acceptEvents('`',                self.toggleNodeProps)
      self.acceptEvents('control-c',        self.copyNodePROPs)
      self.acceptEvents('control-v',        self.pasteNodePROPs)
      self.acceptEvents('1',                self.toggleGrid)
      self.acceptEvents('2',                self.toggleSnapOtherOrigin)
      self.acceptEvents('3',                self.toggleSnapGeomOrigin)
      self.acceptEvents('4',                self.toggleSnapCorners)
      self.acceptEvents('5',                self.toggleSnapMidPoints)
      self.acceptEvents('6',                self.toggleSnapOtherCorners)
      self.acceptEvents('7',                self.toggleSnapOtherMidPoints)


  def __createEditSessionLegend(self):
      # create edit session legend
      self.legendOnOff=0
      self.text_A2dEditHelp=[
             'Aspect2D EDITING SESSION\n[ F1 ] : help',
             'Aspect2D EDITING SESSION\n[ F1 ] : hide help\n\n[ MiddleMouseBtn ] : pan aspect2d\n[ MouseWheel ] : zoom aspect2d\n[ A ] : restore aspect2d size\n[ F ] : set view focus on selected node\n[ ` ] : toggle Properties panel\n[ 1 ] : toggle grid snap\n[ 2 ] : toggle other-type node snap\n[ 3 ] : toggle GeomNode snap\n[ 4 ] : toggle geom corners snap\n[ 5 ] : toggle geom mid-points snap\n[ 6 ] : toggle other-type node corners snap\n[ 7 ] : toggle other-type node mid-points snap\n[ Ctrl + Z ] : undo\n[ Z ] : redo\n[ HOME ] : jump to start of edit history\n[ END ] : jump to end of edit history',
             '\n\n     [ SPACE ] : edit selected node\n     [ CapsLock ] : toggle ParentsList/Aspect2dScenegraph\n     [ ESC ] : exit session'
              ]
      self.text_A2dEditHelp.append(self.text_A2dEditHelp[1]+self.text_A2dEditHelp[2])
      pos=aspect2d.getRelativePoint(render2d,Point3(-.95,0,.9))
      self.A2dEditLegend = OnscreenText( parent=self.a2dNormalScaled, font=self.transMtl,
              text = self.text_A2dEditHelp[self.legendOnOff*3],scale=.05,pos=(pos[0],pos[2]),
              fg=(1,1,1,1), shadow=(0,0,0,1), shadowOffset=(.105,.105),align=TextNode.ALeft, sort=10000, mayChange=1)
      self.A2dEditLegend.setBin('editingGUI',0)
      self.A2dEditLegend.hide()

  def __createEditNotification(self):
      # node to hold collision ray and the notification text
      self.a2dMouseLoc = self.a2dNormalScaled.attachNewNode('',sort=10000)
      self.a2dMouseLoc.setBin('editingGUI',0)
      self.editNotifierOnOff=0
      self.text_EditNotifier=(
           'Time to edit the node !\n[ H ] : help',
           'Time to edit the node !\n[ H ] : hide help\n\n[LeftMouseBtn]: move\n[Ctrl + LeftMouseBtn]: scale\nhold [Shift]: ortho movement\n\ngo to Select Mode :\n[SPACE]: keep changes\n[ESC]: discard changes\n\nwhen transforming :\n[ESC]: abort current transform')
      self.notifierText = OnscreenText( parent=self.a2dMouseLoc,
                          text = self.text_EditNotifier[self.editNotifierOnOff], font=self.transMtl,
                          pos=(.02,-.125),scale = .052,fg=(1,1,1,1), shadow=(0,0,0,.8), mayChange=1)
      self.notifierText.setAlphaScale(.3)
      self.notifierText.hide()

  def __createParentsListGUI(self):
      self.parentsList=1
      self.nodesListGUI=aspect2d.attachNewNode('')
      self.nodesFrameScaleHandler=self.nodesListGUI.attachNewNode('')
      self.nodesFrameScaleHandler.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      # DirectFrame to hold nodes' check buttons
      self.nodesFrame = DirectFrame(parent=self.nodesFrameScaleHandler, scale=.05,frameColor=(.8,.5,.2,.6),
                    relief=DGG.GROOVE, borderWidth=(0.2,0.2), sortOrder=100000, suppressMouse=0)
      OnscreenText( parent=self.nodesFrame, text='DRAG  ME', scale = .8, pos=(-.3,-1.6), roll=90,
                    fg=(0,0,0,1), shadow=(0,0,0,1),align=TextNode.ARight)
      # make it draggable,
      # use the built-in DirectGUI editing, but use LMB instead of MMB
      self.nodesFrame['state']=DGG.NORMAL
      self.nodesFrame.bind(DGG.B1PRESS,self.nodesFrame.editStart)
      self.nodesFrame.bind(DGG.B1RELEASE,self.__releaseNodesFrame)
      self.nailNodesListButton =  myDirectButton(parent=self.nodesFrame, text='T', pos=(0,0,0),
                   text_fg=(1,1,1,1), text_pos=(-.07,-.5), text_scale=(1.5,1.5),
                   frameColor=(.0,.0,.0,.7), frameSize=(-.75,.75,-.75,.75),
                   command=self.nailNodesList, enableEdit=0, suppressMouse=0)
      self.nodesFrame.setBin('editingGUI',0)
      self.nodesListGUI.hide()
      self.nodesListGUI.setTag('A2Dedit component','')
      DirectFrame( parent=self.nodesFrame,frameSize=(0,8,-.5,1),frameColor=(.1,.3,.7,.5),
                   text_font=self.transMtl, pos=(1,0,-1), text='SELECT  NODE :', text_scale=1.1,
                   text_pos=(.4,0), text_fg=(1,1,1,1), text_shadow=(0,0,0,1),
                   text_align=TextNode.ALeft, enableEdit=0, suppressMouse=0)
      self.isNodesListNailed=0
      self.parentsListButtonZspacing=1.2

  def __createA2DscenegraphTreeGUI(self):
      self.A2dSGBrowser = SceneGraphBrowser(
           parent=None, # where to attach SceneGraphBrowser frame
           # user defined method, executed when a node get selected,
           # with the selected node passed to it
           command=self.clearCollNselectA2Dchild,
           contextMenu=None,
           # selectTag and noSelectTag are used to filter the selectable nodes.
           # The unselectable nodes will be grayed.
           # You should use only selectTag or noSelectTag at a time. Don't use both at the same time.
           selectTag=['a2dID'],   # only nodes which have the tag(s) are selectable. You could use multiple tags.
           #noSelectTag=['noSelect'], # only nodes which DO NOT have the tag(s) are selectable. You could use multiple tags.
           # nodes which have exclusionTag wouldn't be displayed at all
           exclusionTag=['A2Dedit component'],
           font=self.transMtl, titleScale=.055, itemScale=.045, itemTextScale=1, itemTextZ=0,
           rolloverColor=(1,.8,.2,1),
           collapseAll=0, # initial tree state
           suppressMouseWheel=1,  # 1 : blocks mouse wheel events from being sent to all other objects.
                                  #     You can scroll the window by putting mouse cursor
                                  #     inside the scrollable window.
                                  # 0 : does not block mouse wheel events from being sent to all other objects.
                                  #     You can scroll the window by holding down the modifier key
                                  #     (defined below) while scrolling your wheel.
           modifier='control'  # shift/control/alt
           )
      self.A2dSGBframe=self.A2dSGBrowser.childrenFrame
      self.A2dSGBframe.setEffect(CompassEffect.make(self.a2dNormalScaled, self.PosScale_LOCK))
      self.A2dSGBframe.setBin('editingGUI',0)
      self.A2dSGBframe.setTag('A2Dedit component','')
      self.A2dSGBrowser.hide()

  def __createNodePropertiesGUI(self):
      # DirectFrame to hold nodes' properties
      self.A2dSGBframeHeight=self.A2dSGBrowser.frameHeight
      height=.928-.5*self.A2dSGBframeHeight
      self.nodePropsGUI=aspect2d.attachNewNode('')
      self.nodePropsGUI.setEffect(
           CompassEffect.make(self.a2dNormalScaled, self.PosScale_LOCK))
      self.nodePropsGUI.setTag('A2Dedit component','')
      self.nodePropsGUI.setBin('editingGUI',0)
      self.nodePropsFrame = DirectFrame(parent=self.nodePropsGUI,
           pos=(0,0,-.5*self.A2dSGBframeHeight-.01),
           frameSize=(-.5,.5,-height,0), frameColor=(0,0,0,.7), enableEdit=0, suppressMouse=0)
      self.nodePropsFrame.hide()

      # properties title
      DirectFrame(parent=self.nodePropsFrame,
           frameColor=(0,0,0,.7), frameSize=(-.045,0,0,-height), pos=(-.51,0,0),
           relief=DGG.FLAT, text_font=self.transMtl,
           text='PROPERTIES', text_pos=(-.012,-height*.5), text_scale=.055, text_roll=90,
           text_fg=(1,1,1,1), text_shadow=(0,0,0,1), enableEdit=0, suppressMouse=0)

      top=-.036
      left=-.47

      OnscreenText( parent=self.nodePropsFrame, text='POSITION', font=self.transMtl,
           scale = .05, pos=(-.48,top-.013), fg=(1,1,1,1), shadow=(0,0,0,1), align=TextNode.ALeft)
      OnscreenText( parent=self.nodePropsFrame, text='SCALE', font=self.transMtl,
           scale = .05, pos=(.48,top-.013), fg=(1,1,1,1), shadow=(0,0,0,1), align=TextNode.ARight)
      OnscreenText( parent=self.nodePropsFrame, text='X\nY\nZ', font=self.transMtl,
           scale = .068, pos=(0,top-.085), fg=(1,1,1,1), shadow=(0,0,0,1))

      indicatorTextPos=(0,.1)
      boxColor=(1,1,1,.7)

      # POSITION
      self.nodePROP_allPosDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left+.375,0,top-.015), relief=None,
             text='all   .', text_fg=(1,1,1,1), text_align=TextNode.ARight,
             indicator_pos=(-.3,0,-.02),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.nodePROP_allPosActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_allPosDCB['indicator_text']=self.DCB_cross
      self.nodePROP_allPosDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_xPosDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left+.43,0,top-.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.75,.7),
             indicatorValue=1, command=self.nodePROP_XposActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_xPosDCB['indicator_text']=self.DCB_cross
      self.nodePROP_xPosDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_yPosDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left+.43,0,top-2*.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.75,.7),
             indicatorValue=1, command=self.nodePROP_YposActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_yPosDCB['indicator_text']=self.DCB_cross
      self.nodePROP_yPosDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_zPosDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left+.43,0,top-3*.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.nodePROP_ZposActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_zPosDCB['indicator_text']=self.DCB_cross
      self.nodePROP_zPosDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_relPos = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.0375, pos=(left+.375,0,top-4.15*.064), relief=None,
             text='relative to render2d   .', text_fg=(1,1,1,1), text_align=TextNode.ARight,
             indicator_pos=(-.3,0,-.02),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.updateNodePROP_pos,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_relPos['indicator_text']=self.DCB_cross
      self.nodePROP_relPos['indicator_text_pos']=indicatorTextPos

      top=-.048
      frameColor=(1,1,1,.5)
      # DirectEntry for X pos
      self.nodePROP_XposEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left,0,top-.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1,
           commandIfValid=self.setPROP_pos, ifValidExtraArgs=['X'],
           commandIfInvalid=self.playInvalidInput)
      # DirectEntry for Y pos
      self.nodePROP_YposEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left,0,top-2*.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1,
           commandIfValid=self.setPROP_pos, ifValidExtraArgs=['Y'],
           commandIfInvalid=self.playInvalidInput)
      # DirectEntry for Z pos
      self.nodePROP_ZposEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left,0,top-3*.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1,
           commandIfValid=self.setPROP_pos, ifValidExtraArgs=['Z'],
           commandIfInvalid=self.playInvalidInput)

      # SCALE
      top=-.036
      left=.08
      self.nodePROP_allScaleDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left+.045,0,top-.015), relief=None,
             text='all', text_fg=(1,1,1,1), text_align=TextNode.ALeft,
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.nodePROP_allScaleActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_allScaleDCB['indicator_text']=self.DCB_cross
      self.nodePROP_allScaleDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_xScaleDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left,0,top-.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.75,.7),
             indicatorValue=1, command=self.nodePROP_XscaleActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_xScaleDCB['indicator_text']=self.DCB_cross
      self.nodePROP_xScaleDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_yScaleDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left,0,top-2*.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.75,.7),
             indicatorValue=1, command=self.nodePROP_YscaleActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_yScaleDCB['indicator_text']=self.DCB_cross
      self.nodePROP_yScaleDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_zScaleDCB = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.05, pos=(left,0,top-3*.064), relief=None,
             text_fg=(1,1,1,1),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.nodePROP_ZscaleActive,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_zScaleDCB['indicator_text']=self.DCB_cross
      self.nodePROP_zScaleDCB['indicator_text_pos']=indicatorTextPos
      self.nodePROP_relScale = myDirectCheckButton(
             parent=self.nodePropsFrame,
             scale=.0375, pos=(left+.048,0,top-4.15*.064), relief=None,
             text='relative to render2d', text_fg=(1,1,1,1), text_align=TextNode.ALeft,
             indicator_pos=(-.6,0,-.02),
             indicator_frameColor=boxColor,
             indicator_frameVisibleScale=(.7,.7),
             indicatorValue=1, command=self.updateNodePROP_scale,
             enableEdit=0, suppressMouse=0)
      self.nodePROP_relScale['indicator_text']=self.DCB_cross
      self.nodePROP_relScale['indicator_text_pos']=indicatorTextPos

      top=-.048
      frameColor=(1,1,1,.5)
      # DirectEntry for X scale
      self.nodePROP_XscaleEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left+.02,0,top-.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1, 
           commandIfValid=self.setPROP_scale, ifValidExtraArgs=['X'],
           commandIfInvalid=self.playInvalidInput)
      # DirectEntry for Y scale
      self.nodePROP_YscaleEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left+.02,0,top-2*.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1, 
           commandIfValid=self.setPROP_scale, ifValidExtraArgs=['Y'],
           commandIfInvalid=self.playInvalidInput)
      # DirectEntry for Z scale
      self.nodePROP_ZscaleEntry = myEntry(parent=self.nodePropsFrame,
           pos=(left+.02,0,top-3*.064), scale=.042, relief=DGG.SUNKEN,
           frameColor=frameColor, frameSize=(-.2,9,-.45,1),
           enableEdit=0, suppressMouse=0,
           # DirectEntry extension arguments
           valueType=1.1, stayFocus=1, 
           commandIfValid=self.setPROP_scale, ifValidExtraArgs=['Z'],
           commandIfInvalid=self.playInvalidInput)
      # COPY button
      self.nodePROP_copy = myDirectButton(
           parent=self.nodePropsFrame, pos=(0,0,top+.01), text_font=self.transMtl,
           text='COPY', text_pos=(0,-.01), text_scale=.05,
           frameColor=(.4,.7,1,.8), frameSize=(-.075,.075,-.027,.027), borderWidth=(.007,.007),
           command=self.copyNodePROPs, enableEdit=0, suppressMouse=0)
      # PASTE button
      self.nodePROP_paste = myDirectButton(
           parent=self.nodePropsFrame, pos=(0,0,top-3.8*.064), text_font=self.transMtl,
           text='PASTE', text_pos=(0,-.01), text_scale=.05,
           frameColor=(.4,.7,1,.8), frameSize=(-.075,.075,-.027,.027), borderWidth=(.007,.007),
           command=self.pasteNodePROPs, enableEdit=0, suppressMouse=0)

  def __createCopyPasteNotification(self):
      # COPIED notification
      self.nodePropsCopied = aspect2d.attachNewNode('')
      self.nodePropsCopied.setEffect(
           CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.nodePropsCopiedText = NodePath( OnscreenText(
           parent=self.nodePropsCopied, text='COPIED', font=self.transMtl,
           scale = .08, pos=(0,-.02),fg=(1,0,0,1), shadow=(0,0,0,1), sort=1000) )
      self.nodePropsCopied.setTag('A2Dedit component','')
      self.nodePropsCopied.setBin('XZcoord',0)
      self.nodePropsCopied.hide()
      self.nodePropsCopiedIval = Sequence(
           Func(self.nodePropsCopied.show),
           Parallel( self.nodePropsCopiedText.scaleInterval(.5,2),
                     self.nodePropsCopiedText.colorScaleInterval(.7,Vec4(1,1,1,0), Vec4(1,1,1,1), blendType='easeIn') ),
           Func(self.nodePropsCopied.hide) )
      # REPLACED notification
      self.nodePropsReplaced = aspect2d.attachNewNode('')
      self.nodePropsReplaced.setEffect(
           CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.nodePropsReplacedText = NodePath( OnscreenText(
           parent=self.nodePropsReplaced, text='REPLACED', font=self.transMtl,
           scale = .08, pos=(0,-.02),fg=(1,0,0,1), shadow=(0,0,0,1), sort=1000) )
      self.nodePropsReplaced.setTag('A2Dedit component','')
      self.nodePropsReplaced.setBin('XZcoord',0)
      self.nodePropsReplaced.hide()
      self.nodePropsReplacedIval = Sequence(
           Func(self.nodePropsReplaced.show),
           Parallel( self.nodePropsReplacedText.scaleInterval(.5,2),
                     self.nodePropsReplacedText.colorScaleInterval(.7,Vec4(1,1,1,0), Vec4(1,1,1,1), blendType='easeIn') ),
           Func(self.nodePropsReplaced.hide) )

  def __createInvalidInputNotification(self):
      # INVALID INPUT notification
      self.invalidInputDummy = aspect2d.attachNewNode('')
      self.invalidInputDummy.setEffect(
           CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.invalidInputDummy.setTag('A2Dedit component','')
      self.invalidInputDummy.setBin('XZcoord',0)
      self.invalidInputText = NodePath( OnscreenText(
           parent=self.invalidInputDummy, text='INVALID', font=self.transMtl,
           scale = .06, pos=(0,-.012),fg=(1,1,1,1), shadow=(0,0,0,1), sort=1000) )
      self.invalidInputText.hide()
      self.invalidInputTextIval = Sequence(
           Func(self.invalidInputText.show),
           Parallel( self.invalidInputText.scaleInterval(.2,1.5,7),
                     self.invalidInputText.colorScaleInterval(.2,Vec4(1,1,1,1), Vec4(1,1,1,0), blendType='easeIn') ),
           self.invalidInputText.scaleInterval(.1,2),
           self.invalidInputText.scaleInterval(.1,1.5),
           self.invalidInputText.colorScaleInterval(.5,Vec4(1,1,1,0), blendType='easeIn'),
           Func(self.invalidInputText.hide) )

  def __createOrthoAxis(self):
      # create an axis for ortho tracking
      self.orthoAxisTrack=aspect2d.attachNewNode('')
      self.orthoAxisTrackGeom = self.orthoAxisTrack.attachNewNode(
           self.createAxisSegs(10,color=(1,.6,.2,1),thickness=5))
      self.orthoAxisTrackGeom.setEffect(
           CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.orthoAxisTrack.setTransparency(1)
      self.orthoAxisTrack.setAlphaScale(.5)
      self.orthoAxisTrack.hide()
      self.orthoAxisTrack.setTag('A2Dedit component','')
      self.send2GUIpopupBin(self.orthoAxisTrack)

  def __createNodeOriginVis(self):
      # create a node to show object's origin
      self.origin=aspect2d.attachNewNode('')
      self.origin.setTransparency(1)
      self.origin.hide()
      self.origin.setTag('A2Dedit component','')
      self.origin.setBin('origin',0)
      # create several circles and load a Z-up-axis
      self.circles=self.origin.attachNewNode('circles')
      self.circles.hide()
      num=6
      for c in range(num):
          circle=self.drawXZCircle(color=(1,0,0,.2),thickness=(num-c)*.5)
          self.circles.attachNewNode(circle,sort=500).setScale(1.-pow(c*.17,2))
      # create an axis for easy layout
      axis=self.origin.attachNewNode(
          self.createAxisSegs(20,color=(0,0,0,.3),thickness=3),sort=500)
      axis.attachNewNode(self.createAxisSegs(20,color=(1,1,0,.5)))
      axis.attachNewNode(self.createAxisSegs(1.2,color=(0,0,0,.9),endColor=(0,0,0,0)))
      axis.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
#       zupaxis=loader.loadModel('zup-axis')
#       zupaxis.reparentTo(self.axis)
#       zupaxis.setScale(.025)
#       zupaxis.find('**/y-axis').removeNode()

      self.node_XZtext=self.origin.attachNewNode('')
      self.node_XZtext.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.node_XZtext.setBin('XZcoord',0)
      textscale=.04
      # node's X position text
      self.nodeXtextZ=-.987
      self.nodeXtextBottom = OnscreenText( parent=self.node_XZtext, scale = textscale, font=self.transMtl,
                          frame=(1,1,1,.5), bg=(0,0,0,.5), fg=(1,1,1,1),
                          shadow=(0,0,0,1), mayChange=1)
      self.nodeXtextTop = self.nodeXtextBottom.instanceUnderNode(self.node_XZtext,'x2')
      # node's Z position text
      self.nodeZtextX=-.975
      self.nodeZtextLeft = OnscreenText( parent=self.node_XZtext, scale = textscale, font=self.transMtl,
                          frame=(1,1,1,.5), bg=(0,0,0,.5), fg=(1,1,1,1),
                          roll=90, shadow=(0,0,0,1), mayChange=1)
      self.nodeZtextRight = self.nodeZtextLeft.instanceUnderNode(self.node_XZtext,'z2')

  def __createXZsnapGrids(self):
      # create XZ grids for snap
      self.XgridSpacing=.02
      self.ZgridSpacing=.02
      self.XgridSpacingR2D=self.XgridSpacing/self.aspRatio
      self.a2dGrid=aspect2d.attachNewNode('grid')
      self.a2dGrid.setTransparency(1)
      self.a2dGrid.setBin('snapTarget0',0)
      self.a2dGrid.setTag('A2Dedit component','')
      self.a2dGrid.setTwoSided(0,100)
      self.a2dGrid.hide()
      # grids along Z axis
      lineZ=self.a2dGrid.attachNewNode(
           self.createLine(length=2.*self.aspRatio,color=(1,1,1,.3)))
      lineZ.setZ(-self.ZgridSpacing*int(1./self.ZgridSpacing))
      for z in range(int(2/self.ZgridSpacing)):
          lineZ.instanceUnderNode(self.a2dGrid,'').setZ(self.ZgridSpacing*(1+z))
      lineZ.instanceUnderNode(self.a2dGrid,'').setZ(self.ZgridSpacing*int(1./self.ZgridSpacing))
      # grids along X axis
      gridX=self.a2dGrid.attachNewNode('')
      gridX.setR(90)
      lineX=gridX.attachNewNode(self.createLine(length=2,color=(1,1,1,.3)))
      lineX.setZ(-self.XgridSpacing*int(self.aspRatio/self.XgridSpacing))
      for x in range(int(2*self.aspRatio/self.XgridSpacing)):
          lineX.instanceUnderNode(gridX,'').setZ(self.XgridSpacing*(1+x))
      lineX.instanceUnderNode(gridX,'').setZ(self.XgridSpacing*int(self.aspRatio/self.XgridSpacing))
      # I don't need to move the individual grid line in the future,
      # so let's trade it for some render speed
      self.a2dGrid.flattenStrong()

  def __setupSnapMarker(self,np,binName):
      np.setTag('A2Dedit component','')
      np.setBin(binName,0)
      np.setTransparency(1)
      np.setColorScaleOff(100)
      np.setTwoSided(0,100)
      fn=np.getTexture().getFilename()
      editTex=loader.loadTexture('markers/%s-twirl.%s' %(fn.getBasenameWoExtension(),fn.getExtension()))
      snapTex=loader.loadTexture('markers/%s-twirl-inverted.%s' %(fn.getBasenameWoExtension(),fn.getExtension()))
      np.setPythonTag('editTex',editTex)
      np.setPythonTag('snapTex',snapTex)
      np.setName('markerMaster')

  def __createSnapMarkers(self):
      # marker for GeomNode origin
      self.geomNodeSnapRadius=.022
      self.GeomOriginMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/geomNode.png' )
      self.__setupSnapMarker(self.GeomOriginMarker,'snapTarget6')
      # marker for other-type node origin
      self.othNodeSnapRadius=.032
      self.otherOriginMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/otherNode.png' )
      self.__setupSnapMarker(self.otherOriginMarker,'snapTarget5')
      # marker for GeomNode bounding box corners
      self.CornerMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/geomCorner.png' )
      self.__setupSnapMarker(self.CornerMarker,'snapTarget4')
      # marker for GeomNode bounding box middle points
      self.MidPointMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/geomMidPoint.png' )
      self.__setupSnapMarker(self.MidPointMarker,'snapTarget3')
      # marker for other-type node bounding box corners
      self.OtherNodeCornerMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/otherCorner.png' )
      self.__setupSnapMarker(self.OtherNodeCornerMarker,'snapTarget2')
      # marker for other-type node bounding box middle points
      self.OtherNodeMidPointMarker=self.createTexturedPoint( thickness=self.markerSize,
         texName='markers/otherMidPoint.png' )
      self.__setupSnapMarker(self.OtherNodeMidPointMarker,'snapTarget1')

  def __createSnapMarkerPoint(self):
      # create point to mark the snap marker which has extension lines
      self.snapMarkerPoint=NodePath(self.createPoint(color=(1,.8,.4,1), thickness=8))
      self.snapMarkerPoint.attachNewNode(self.createPoint(color=(1,0,0,1), thickness=5))
      self.snapMarkerPoint.setTag('A2Dedit component','')
      self.snapMarkerPoint.setBin('snapPoint',0)
      self.snapMarkerPoint.flattenStrong()
      self.setColorNtextureOff(self.snapMarkerPoint)

  def __createSnapExtensionLines(self):
      # create horisontal snap extension line
      lineLen=100
      self.horisontalSnapLine=NodePath(
           self.createLine(lineLen,color=(0,0,0,1),thickness=3,centered=0))
      self.horisontalSnapLine.setBin('origin',0)
      self.horisontalSnapLine.setTwoSided(0,100)
      self.setColorNtextureOff(self.horisontalSnapLine)
      self.horisontalSnapLine.setTag('A2Dedit component','')
      self.horisontalSnapLine.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      horisontalSnapLine2=NodePath(self.createDashedLine(lineLen,line=.015,gap=.015,centered=0))
      horisontalSnapLine2.reparentTo(self.horisontalSnapLine)
      horisontalSnapLine2.setX(.01)
      # create vertical snap extension line
      self.verticalSnapLine=NodePath(
           self.createLine(lineLen,color=(0,0,0,1),thickness=3,centered=0))
      self.verticalSnapLine.setBin('origin',0)
      self.setColorNtextureOff(self.verticalSnapLine)
      self.verticalSnapLine.setTag('A2Dedit component','')
      self.verticalSnapLine.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      verticalSnapLine2=NodePath(self.createDashedLine(lineLen,line=.015,gap=.015,centered=0))
      verticalSnapLine2.reparentTo(self.verticalSnapLine)
      verticalSnapLine2.setX(.01)

  def __createIntersectionMarker(self):
      # create intersection marker
      length=.16
      offset=.015
      color=(0,0,0,0)
      cornerColor=(0,0,0,1)
      self.intersectionMarker = aspect2d.attachNewNode('')
      axis=self.intersectionMarker.attachNewNode('')
      for r in range(4):
          axisGeomDum=axis.attachNewNode('')
          axisGeomDum.attachNewNode( self.createLshape(
             length,color,cornerColor,thickness=4)).setPos(offset,0,offset)
          axisGeomDum.setR(r*90)
      #__________
      length+=.02
      color=(.5,.5,.5,0)
      cornerColor=(1,0,0,1)
      for r in range(4):
          axisGeomDum=axis.attachNewNode('')
          axisGeomDum.attachNewNode( self.createLshape(
             length,color,cornerColor,thickness=2)).setPos(offset,0,offset)
          axisGeomDum.setR(r*90)
      #__________
      axis.attachNewNode(self.createPoint(color=(0,0,0,.8), thickness=10))
      axis.attachNewNode(self.createPoint(color=(1,0,0,1), thickness=5))
      axis.setEffect(CompassEffect.make(self.a2dNormalScaled, CompassEffect.PScale))
      self.intersectionMarker.flattenMedium()
      self.intersectionMarker.setTransparency(1)
      self.intersectionMarker.setBin('snapPoint',0)
      self.intersectionMarker.setTag('A2Dedit component','')
      self.intersectionMarker.hide()

  def __releaseNodesFrame(self,e):
      self.nodesFrame.editStop(e)
      self.__relocateNodesFrame()

  def __relocateNodesFrame(self,pos=None):
      if pos:
         self.nodesFrame.setPos(pos)
      self.nodesListGUI.setPos(self.nodesFrame.getPos(aspect2d))
      self.nodesFrame.setPos(0,0,0)



  def setColorNtextureOff(self,obj):
      obj.setTextureOff(100)
      obj.setColorScaleOff(100)

  def createAxisSegs(self, length=2, color=(1,1,1,1), endColor=None, thickness=1):
      LS=LineSegs()
      LS.setColor(*color)
      LS.setThickness(thickness)
      LS.moveTo(-length*.5,0,0)
      if endColor:
         LS.drawTo(0,0,0)
      LS.drawTo(length*.5,0,0)
      LS.moveTo(0,0,-length*.5)
      if endColor:
         LS.drawTo(0,0,0)
      LS.drawTo(0,0,length*.5)
      node=LS.create()
      if endColor:
         LS.setVertexColor(0,*endColor)
         LS.setVertexColor(2,*endColor)
         LS.setVertexColor(3,*endColor)
         LS.setVertexColor(5,*endColor)
      return node

  def createPoint(self, color=(1,1,1,1), thickness=1):
      LS=LineSegs()
      LS.setColor(*color)
      LS.setThickness(thickness)
      LS.moveTo(0,0,0)
      return LS.create()

  def createTexturedPoint(self, thickness=1, texName=None):
      LS=LineSegs()
      LS.setThickness(thickness)
      LS.moveTo(0,0,0)
      point=NodePath(LS.create())
      if texName:
         tex=loader.loadTexture(texName)
         point.setTexGen(TextureStage.getDefault(),TexGenAttrib.MPointSprite)
         point.setTexture(tex)
      return point

  def createLine(self, length=1, color=(1,1,1,1), endColor=None, thickness=1, centered=1):
      LS=LineSegs()
      LS.setColor(*color)
      LS.setThickness(thickness)
      LS.moveTo(-length*.5*centered,0,0)
      LS.drawTo(length*(1-.5*centered),0,0)
      node=LS.create()
      if endColor:
         LS.setVertexColor(1,*endColor)
      return node

  def createLshape(self, length=1, color=(0,0,0,1), cornerColor=None, thickness=1):
      LS=LineSegs()
      LS.setColor(*color)
      LS.setThickness(thickness)
      LS.moveTo(0,0,length)
      LS.drawTo(0,0,0)
      LS.drawTo(length,0,0)
      node=LS.create()
      if cornerColor:
         LS.setVertexColor(1,*cornerColor)
      return node

  def createDashedLine(self, length=1, color=(1,1,1,1), thickness=1, line=.02, gap=.02,centered=1):
      currPos=Point3(-length*.5*centered,0,0)
      LS=LineSegs()
      LS.setColor(*color)
      LS.setThickness(thickness)
      while currPos[0]<length*(1-.5*centered):
            LS.moveTo(currPos)
            LS.drawTo(currPos+Point3(min(length*(1-.5*centered)-currPos[0],line),0,0))
            currPos=LS.getCurrentPosition()+Point3(gap,0,0)
      return LS.create()

  def loadBinaryFile(self):
      try:
        file2d=open(self.myFile+self.BINext,'r')
        self.a2dDict=cPickle.load(file2d)
        file2d.close()
        print 'BINARY FILE "%s" SUCCESSFULLY LOADED.' %(self.myFile+self.BINext)
        return 1
      except:
        return 0

  def loadASCIIFile(self):
      try:
        file2d=open(self.myFile+self.ASCIIext,'r')
        objID=str(file2d.readline()).rstrip('\n')
        while objID:
          objMat=str(file2d.readline()).split()
          self.a2dDict[objID]=[float(el) for el in objMat]
          objID=str(file2d.readline()).rstrip('\n')
        file2d.close()
        print 'ASCII FILE "%s" SUCCESSFULLY LOADED.' %(self.myFile+self.ASCIIext)
        return 1
      except:
        return 0

  def acceptEvents(self,events,method,args=None,repeat=0):
      if type(events)==types.StringType:
         events=(events,)
      for m in self.modes:
          for e in events:
              if args:
                 self.accept(m+e,method,args)
                 if repeat:
                    self.accept(m+e+'-repeat',method,args)
              else:
                 self.accept(m+e,method)
                 if repeat:
                    self.accept(m+e+'-repeat',method)

  def filterButton(self,button):
      print button
      if button==self.enterEditSessionKey:
         self.enterA2DeditSession()
      else:
         self.listenButtonDown(1)

  def listenButtonDown(self,x):
      self.acceptOnce('buttonDown', self.filterButton)

  def send2GUIpopupBin(self,obj):
      obj.setBin('gui-popup',0)

  def isHoverOnNodesListGUI(self,pos):
      hover=0
      try:
         for region in self.nodesListGUIregions:
             r=region.getFrame()
             hover |= (pos[0]>r[0] and pos[0]<r[1]) and (pos[1]>r[2] and pos[1]<r[3])
             if hover:
                return 1
      except:
          pass
      return 0

  def disableMouse2Cam(self):
      base.mouseInterface.stash()

  def enableMouse2Cam(self):
      base.mouseInterface.unstash()

  def renderFrame(self):
      base.winList[0].getGsg().getEngine().renderFrame()

  def framebufferShot(self):
      # preparing for render-to-texture of render (3d world),
      # so wipe out all 2d scene
      base.setFrameRateMeter(0)
      render2d.hide()
      # render to texture initialization
      base.winList[0].addRenderTexture(Texture(),mode=GraphicsOutput.RTMCopyTexture)
      # render the next frame, the 3D world only
      self.renderFrame()
      # put it on a card, covering the whole screen
      self.renderCapture=base.winList[0].getTextureCard()
      self.renderCapture.reparentTo(render2d)
      self.renderCapture.setBin('background',-1000)
      # stop render-to-texture mode
      base.winList[0].clearRenderTextures()
      # show the 2d scene & hide the 3d world
      base.setFrameRateMeter(1)
      render2d.show()
      render.hide()
      # edit session preparation will take a while,
      # so render the whole scene again before doing that,
      # so the user will not see the rendercapture card without the 2d scene,
      # but the complete scene, with once blinking screen
      self.sessionPrepText=DirectFrame( frameSize=(-.9,.9,-.15,.15),frameColor=(0,0,0,.7),
              text = 'PREPARING  ASPECT2D  EDITING  SESSION\nplease wait a moment ........',
              text_font=self.transMtl, text_scale=.07, text_pos=(0,.02),
              text_fg=(1,.5,.2,1), text_shadow=(0,0,0,1))
      self.sessionPrepText.setBin('XZcoord',0)
      self.renderFrame()
      self.renderFrame()

  def enterA2DeditSession(self):
      if taskMgr.hasTaskNamed('A2DeditLoop'):
         return
      PR.pause(
         allAnims=1,
         allAudios=1,
         allMovies=1,
         collision=1
         )
      self.wasMouseEnabled=base.mouse2cam.getParent()==base.mouseInterface
      self.disableMouse2Cam()
      self.framebufferShot()
      # remove session preparation text
      self.sessionPrepText.destroy()
      self.MODE_lastPrefix=base.buttonThrowers[0].node().getPrefix()
      self.hiddenObj=[]
      self.snapMarker_GeomOrigin=()
      self.snapMarker_Corners=()
      self.snapMarker_MidPoints=()
      self.snapMarker_OtherOrigin=()
      self.snapMarker_OtherCorners=()
      self.snapMarker_OtherMidPoints=()
      self.Xthreshold=.85*self.aspRatio
      self.Zthreshold=.78
      self.selectedNP=None
      self.collideInto=None
      self.editMode=0
      self.ortho=0
      self.lastMpos=Point2(0,0)
      self.focusViewOnNodeIval=Wait(0)
      self.zoomNormalA2D(0)
      self.A2dSGBrowser.setRoot(aspect2d)
      self.getDGUIObj()
      self.disableDGUI()
      for p in aspect2d.getChildren():
          self.recurPutMarkers(p)
      # update the properties entries
      self.updateNodePROP_pos()
      self.updateNodePROP_scale()
      self.nodePROP_copy.setColorScale(.3,.3,.3,1)
      self.nodePROP_copy.node().setActive(0)
      self.disableNodePROP_paste()
      self.nodePROP_clipboard=None
      __builtins__['FOCUS_Entry']=None
      # snap markers
      self.isSnapGeomOrigin=1
      self.isSnapCorners=1
      self.isSnapMidPoints=1
      self.isSnapOtherOrigin=1
      self.isSnapOtherCorners=1
      self.isSnapOtherMidPoints=1
      self.toggleSnapGeomOrigin()
      self.toggleSnapCorners()
      self.toggleSnapMidPoints()
      self.toggleSnapOtherOrigin()
      self.toggleSnapOtherCorners()
      self.toggleSnapOtherMidPoints()
      self.toggleCollision(1)
      self.history_SelectMode=None
      self.history_SelectMode=[]
      self.history_SelectModeIdx=-1
      #base.mouseWatcherNode.showRegions(render2d,'gui-popup',0)
      PolyButton.setBTprefix(self.MODE_a2dSelect)
      taskMgr.add(self.editLoop,'A2DeditLoop')
      self.A2dEditLegend.show()
      if not self.parentsList:
         self.A2dSGBrowser.show()

  def exitA2DeditSession(self,saveChanges):
      if not taskMgr.hasTaskNamed('A2DeditLoop'):
         return
      taskMgr.remove('A2DeditLoop')
      self.A2dEditLegend.hide()
      self.zoomNormalA2D(.8)
      self.toggleCollision(0)
      self.nodesListGUI.hide()
      self.A2dSGBrowser.hide()
      self.nodePropsFrame.hide()
      self.origin.hide()
      self.a2dGrid.hide()
      self.enableDGUI()
      try:
          self.nodeIval.finish()
          self.nodeIval=None
      except:
          pass
      if self.selectedNP:
         # restore it back to it's original render bin
         if self.selectedNodeBin:
            self.selectedNP.setBin(self.selectedNodeBin,self.selectedNodeDrawOrder)
         else:
            self.selectedNP.clearBin()
         self.BBoxAnimIval.finish()
         self.selectedNP.find('**/BBoxVis').removeNode()
      self.A2dSGBrowser.clear()
      base.mouseWatcherNode.hideRegions()
      PolyButton.setBTprefix(self.MODE_lastPrefix)
      taskMgr.doMethodLater(.1,self.listenButtonDown,'listenAgain')
      # discard changes & leave edit session
      if not saveChanges:
         self.gotoBeginHistorySelectMode()
      # before saving, remove :
      #     1. snap markers
      #     2. collision cards of Direct GUIs
      # so that they won't be added to dictionary
      for m in self.snapMarker_GeomOrigin:
          m.removeNode()
      print ':) GeomNode origin marker REMOVED !!'
      self.snapMarker_GeomOrigin=None
      for m in self.snapMarker_Corners:
          m.removeNode()
      print ':) corner marker REMOVED !!'
      self.snapMarker_Corners=None
      for m in self.snapMarker_MidPoints:
          m.removeNode()
      print ':) middle point marker REMOVED !!'
      self.snapMarker_MidPoints=None
      for m in self.snapMarker_OtherOrigin:
          m.removeNode()
      print ':) other node origin marker REMOVED !!'
      self.snapMarker_OtherOrigin=None
      for m in self.snapMarker_OtherCorners:
          m.removeNode()
      print ':) other corner marker REMOVED !!'
      self.snapMarker_OtherCorners=None
      for m in self.snapMarker_OtherMidPoints:
          m.removeNode()
      print ':) other middle point marker REMOVED !!'
      self.snapMarker_OtherMidPoints=None
      for c in aspect2d.findAllMatches('**/card 4 DGUI collision'):
          c.removeNode()
      print ':) DGUI card REMOVED !!'
      # save changes & leave edit session
      if saveChanges:
         # write transform dictionary to disk
         self.__writeA2DtoFile()
         print '(:D) SAVED TO FILE !!!'
      # discard changes & leave edit session
      else:
         print '... and exit'
      PR.resume()
      # remove the captured image of render
      self.renderCapture.getTexture().releaseAll()
      self.renderCapture.removeNode()
      # restore render visibility
      render.show()
      # re-hide the originally hidden objects
      for o in self.hiddenObj:
          o.hide()
      self.hiddenObj=None
      # restore the mouse if it was enabled
      if self.wasMouseEnabled:
         self.enableMouse2Cam()
         base.enableMouse()
      # clear DGUI objects list
      if hasattr(self,'DGUIobjects'):
         self.DGUIobjects=None

  def exit(self):
      self.exitConfirmation=loader.loadModelCopy('exitDialog/but')
      self.exitConfirmation.setAlphaScale(.9)
      self.exitConfirmation.setBin('XZcoord',0)
      self.BTlastPrefix=PolyButton.getBTprefix()
      PolyButton.setBTprefix('dialogInFocus-')
      self.exitDialog = PolyButton.myDialog(
          root = self.exitConfirmation,
          parent = self.a2dNormalScaled,
          pos = (0,0,1.5),
          scale = .8,
          buttons = (
             PolyButton.myButton(self.exitConfirmation, 'y', ('hover','pressed','disabled'),
                hitKey='y', command=self.exitSaveChangesYes, stayAlive=0),
             # let's set the cancel button as default button, so pack it in a sequence
             [PolyButton.myButton(self.exitConfirmation, 'c', ('hover','pressed','disabled'),
                hitKey=('c','escape'), command=self.exitSaveChangesCancel, stayAlive=0)],
             PolyButton.myButton(self.exitConfirmation, 'n', ('hover','pressed','disabled'),
                hitKey='n', command=self.exitSaveChangesNo, stayAlive=0),
                    )  # y, c, n are geom's name, will be searched under root node
          )
      Sequence(
          # disable all editing DGUI items
          Func(self.disableAllPGItemUnder,aspect2d),
          # disable A2Dedit collision (mouse ray) node's mask
          Func(self.A2DeditMouseColNP.node().setFromCollideMask,MASK_off),
          # disable dialog's buttons
          Func(self.exitDialog.disableDialogButtons),
          # slide the dialog onto screen
          self.exitConfirmation.posInterval(.5,Point3(0,0,1),blendType='easeOut'),
          # enable dialog's buttons
          Func(self.exitDialog.enableDialogButtons),
          # enable dialog's navigation keys
          Func(self.exitDialog.setDialogKeysActive),
          # starts PolyButton's collision loop
          Func(PolyButton.start)
      ).start()

  def exitSaveChangesYes(self):
      print '\n<YES YES YES>\n'
      self.exitSaveChangesCleanup( Func(self.exitA2DeditSession,1) )

  def exitSaveChangesCancel(self):
      print '\n<CANCEL CANCEL CANCEL>\n'
      self.exitSaveChangesCleanup()

  def exitSaveChangesNo(self):
      print '\n<NO NO NO>\n'
      self.exitSaveChangesCleanup( Func(self.exitA2DeditSession,0) )

  def exitSaveChangesCleanup(self,finalCommand=None):
      exitSequence = Sequence(
          # stops PolyButton's collision loop
          Func(PolyButton.stop),
          # disable dialog buttons' events and shortcut keys
          Func(self.exitDialog.disableDialogButtons),
          # slide it back off screen
          self.exitConfirmation.posInterval(.5,Point3(0,0,1.5)),
          # destroy it
          Func(self.exitDialog.cleanup),
          # restore the last button thrower prefix
          Func(PolyButton.setBTprefix,self.BTlastPrefix),
          # "reset" CollisionHandlerEvent
          Func(PolyButton.reset),
          # restore A2Dedit collision (mouse ray) node's mask
          Func(self.A2DeditMouseColNP.node().setFromCollideMask,self.A2DeditMask),
          # re-activate last active editing GUI items
          Func(self.enableLastActivePGItems)
      )
      if finalCommand:
         exitSequence.append(finalCommand)
      exitSequence.start()

  def disableAllPGItemUnder(self,np):
      activePGItems = np.findAllMatches('**/+PGItem')
      self.lastActivePGItems=[]
      for i in activePGItems:
          n=i.node()
          if n.getActive():
             n.setActive(0)
             self.lastActivePGItems.append(n)

  def enableLastActivePGItems(self):
      for n in self.lastActivePGItems:
          n.setActive(1)
      self.lastActivePGItems=None

#   def followA2Drot(self,obj):
#       obj.setEffect(CompassEffect.make(aspect2d, CompassEffect.PRot))

  def putGeomNodeCornersNmidPointsMarkers(self,obj):
      # exclude all children's snap markers from contributing to their parent's bounds
      objChildren=obj.findAllMatches('**/=A2Dedit component')
      objChildren.stash()
      # save original rotation and clear it to get the "real" bounding box
      origHpr=obj.getHpr()
      obj.setHpr(0,0,0)
      bounds=obj.getTightBounds()
      dumNode=obj.getParent().attachNewNode('markerDummy')
      #_______________________________________
      bl=self.CornerMarker.instanceUnderNode(dumNode,'CornerMarker')
      bl.setTag('markerName','bottomLeft')
      bl.setPos(bounds[0][0],0,bounds[0][2])
      self.snapMarker_Corners+=(bl,)
      #_______________________________________
      tl=self.CornerMarker.instanceUnderNode(dumNode,'CornerMarker')
      tl.setTag('markerName','topLeft')
      tl.setPos(bounds[0][0],0,bounds[1][2])
      self.snapMarker_Corners+=(tl,)
      #_______________________________________
      br=self.CornerMarker.instanceUnderNode(dumNode,'CornerMarker')
      br.setTag('markerName','bottomRight')
      br.setPos(bounds[1][0],0,bounds[0][2])
      self.snapMarker_Corners+=(br,)
      #_______________________________________
      tr=self.CornerMarker.instanceUnderNode(dumNode,'CornerMarker')
      tr.setTag('markerName','topRight')
      tr.setPos(bounds[1][0],0,bounds[1][2])
      self.snapMarker_Corners+=(tr,)
      #===================================================
      t=self.MidPointMarker.instanceUnderNode(dumNode,'MidPointMarker')
      t.setTag('markerName','top')
      t.setPos(PU.average(bounds[0][0],bounds[1][0]),0,bounds[1][2])
      self.snapMarker_MidPoints+=(t,)
      #_______________________________________
      b=self.MidPointMarker.instanceUnderNode(dumNode,'MidPointMarker')
      b.setTag('markerName','bottom')
      b.setPos(PU.average(bounds[0][0],bounds[1][0]),0,bounds[0][2])
      self.snapMarker_MidPoints+=(b,)
      #_______________________________________
      l=self.MidPointMarker.instanceUnderNode(dumNode,'MidPointMarker')
      l.setTag('markerName','left')
      l.setPos(bounds[0][0],0,PU.average(bounds[0][2],bounds[1][2]))
      self.snapMarker_MidPoints+=(l,)
      #_______________________________________
      r=self.MidPointMarker.instanceUnderNode(dumNode,'MidPointMarker')
      r.setTag('markerName','right')
      r.setPos(bounds[1][0],0,PU.average(bounds[0][2],bounds[1][2]))
      self.snapMarker_MidPoints+=(r,)
      #_______________________________________
      dumNode.getChildren().wrtReparentTo(obj)
      dumNode.removeNode()
      obj.setHpr(origHpr)
      objChildren.unstash()

  def putOtherNodeCornersNmidPointsMarkers(self,obj):
      # exclude all children's snap markers from contributing to their parent's bounds
      objChildren=obj.findAllMatches('**/=A2Dedit component')
      objChildren.stash()
      # save original rotation and clear it to get the "real" bounding box
      origHpr=obj.getHpr()
      obj.setHpr(0,0,0)
      bounds=obj.getTightBounds()
      dumNode=obj.getParent().attachNewNode('markerDummy')
      #_______________________________________
      bl=self.OtherNodeCornerMarker.instanceUnderNode(dumNode,'OtherNodeCornerMarker')
      bl.setTag('markerName','bottomLeft')
      bl.setPos(bounds[0][0],0,bounds[0][2])
      self.snapMarker_OtherCorners+=(bl,)
      #_______________________________________
      tl=self.OtherNodeCornerMarker.instanceUnderNode(dumNode,'OtherNodeCornerMarker')
      tl.setTag('markerName','topLeft')
      tl.setPos(bounds[0][0],0,bounds[1][2])
      self.snapMarker_OtherCorners+=(tl,)
      #_______________________________________
      br=self.OtherNodeCornerMarker.instanceUnderNode(dumNode,'OtherNodeCornerMarker')
      br.setTag('markerName','bottomRight')
      br.setPos(bounds[1][0],0,bounds[0][2])
      self.snapMarker_OtherCorners+=(br,)
      #_______________________________________
      tr=self.OtherNodeCornerMarker.instanceUnderNode(dumNode,'OtherNodeCornerMarker')
      tr.setTag('markerName','topRight')
      tr.setPos(bounds[1][0],0,bounds[1][2])
      self.snapMarker_OtherCorners+=(tr,)
      #===================================================
      t=self.OtherNodeMidPointMarker.instanceUnderNode(dumNode,'OtherNodeMidPointMarker')
      t.setTag('markerName','top')
      t.setPos(PU.average(bounds[0][0],bounds[1][0]),0,bounds[1][2])
      self.snapMarker_OtherMidPoints+=(t,)
      #_______________________________________
      b=self.OtherNodeMidPointMarker.instanceUnderNode(dumNode,'OtherNodeMidPointMarker')
      b.setTag('markerName','bottom')
      b.setPos(PU.average(bounds[0][0],bounds[1][0]),0,bounds[0][2])
      self.snapMarker_OtherMidPoints+=(b,)
      #_______________________________________
      l=self.OtherNodeMidPointMarker.instanceUnderNode(dumNode,'OtherNodeMidPointMarker')
      l.setTag('markerName','left')
      l.setPos(bounds[0][0],0,PU.average(bounds[0][2],bounds[1][2]))
      self.snapMarker_OtherMidPoints+=(l,)
      #_______________________________________
      r=self.OtherNodeMidPointMarker.instanceUnderNode(dumNode,'OtherNodeMidPointMarker')
      r.setTag('markerName','right')
      r.setPos(bounds[1][0],0,PU.average(bounds[0][2],bounds[1][2]))
      self.snapMarker_OtherMidPoints+=(r,)
      #_______________________________________
      for c in dumNode.getChildren():
          c.wrtReparentTo(obj)
      dumNode.removeNode()
      obj.setHpr(origHpr)
      objChildren.unstash()

  def recurPutMarkers(self,p):
      for c in p.getChildren():
          # iterate down to object's children
          if c.hasTag('a2dID'):
             self.recurPutMarkers(c)
      if p.hasTag('a2dID'):
         nodetype=str(p.node().getType())
         if nodetype=='GeomNode' or nodetype[:2]=='PG':
            # origin markers
            marker=p.attachNewNode('geomOriginMarker')
            self.GeomOriginMarker.instanceTo(marker,0)
            self.snapMarker_GeomOrigin+=(marker,)
            # corners & mid-points markers
            self.putGeomNodeCornersNmidPointsMarkers(p)
         else:
            # other-type node origin markers
            marker=p.attachNewNode('othOriginMarker')
            self.otherOriginMarker.instanceTo(marker,0)
            self.snapMarker_OtherOrigin+=(marker,)
            # other-type node corners & mid-points markers
            self.putOtherNodeCornersNmidPointsMarkers(p)
      if p.isHidden() and not p.hasTag('A2Dedit component'):
         self.hiddenObj.append(p)
         p.show()

  def recurGetDGUIobj(self,obj):
      if obj.hasTag('A2Dedit component'):
         return
      for child in obj.getChildren():
          if str(child.node().getType())[:2]=='PG':
             self.recurGetDGUIobj(child)
      if str(obj.node().getType())[:2]=='PG':
         self.DGUIobjects+=(obj,)
         if obj.hasTag('a2dID'):
            # create a card under DirectGUI object to make it collidable
            CM=CardMaker('card 4 DGUI collision')
            CM.setFrame(obj.node().getFrame())
            card=obj.attachNewNode(CM.generate())
            bitmask=card.getCollideMask()
            bitmask.setBit(self.A2DeditMaskBit)
            card.setCollideMask(bitmask)
            card.hide()
      if obj.isHidden():
         self.hiddenObj.append(obj)
         obj.show()

  def getDGUIObj(self):
      self.DGUIobjects=()
      for obj in aspect2d.getChildren():
          if str(obj.node().getType())[:2]=='PG' and not obj.hasTag('A2Dedit component'):
             self.DGUIobjects+=(obj,)
          self.recurGetDGUIobj(obj)

  def screenPixel(self,posR2D):
      posSPx=int(1+(posR2D[0]+1)*self.winXmaxHalf)
      posSPy=int(1+(-posR2D[1]+1)*self.winYmaxHalf)
      return (posSPx, posSPy)

#####################################################################
#_______________________M A I N  L O O P____________________________#
#                                                                   #
  '''   _      _            ___         __     ___     __
       /#\    /#\          /@#@\       |##|   |###\   |##|
      /###\  /###\        /##_##\      |##|   |#####\ |##|
     /##_######_##\      /##/_\##\     |##|   |##|\##\|##|
    |##/ \####/ \##|    /#########\    |##|   |##| \#####|
    |##|  |##|  |##|   |##/^^^^\###|   |##|   |##|   \###|
     ^^    ^^    ^^     ^^      ^^^     ^^     ^^     ^^^
     
        __           ______       _____       ______
       |##|         /######\     /######\    |######@\
       |##|        |##/^^\##|   |##/^^\##|   |##/^^\##|
       |##|____    |##|  |##|   |##|  |##|   |##\__/##|
       |#######|   |##\__/##|   |##\__/##|   |#######/
       |#######|    \######/     \######/    |##|^^^^
        ^^^^^^^      ^^^^^^       ^^^^^^     ^^^^
  '''
  def editLoop(self,task):
      ''' main loop for aspect2d editing '''

      if not base.mouseWatcherNode.hasMouse():
         return Task.cont
      mpos=base.mouseWatcherNode.getMouse()
      # update mouse node (collision ray & notifier text) only on mouse move
      if self.lastMpos==mpos:
         return Task.cont
      self.lastMpos=Point2(mpos)
      self.a2dMouseLoc.setPos(render2d,mpos[0],0,mpos[1])
      # at editing time or while any Direct Entry in focus,
      # we don't need the rest of this loop
      if self.editMode or FOCUS_Entry!=None:
         return Task.cont
      # do collision only on mouse move & not hover on nodes list GUI or
      # the properties panel, otherwise collision may occur
      # when the user is using the GUI, which is painful, so get outta here
      if (
         not self.parentsList or
         self.isHoverOnNodesListGUI(mpos) or
         self.isHoverOnNodePropsFrame(mpos)
         ):
         return Task.cont
      self.A2DeditMousecTrav.traverse(aspect2d)
      # no collision..... get outta here
      if not self.A2DeditMousecQueue.getNumEntries():
         return Task.cont

      minRadius=100000
      for g in range(self.A2DeditMousecQueue.getNumEntries()):
          intonode=self.A2DeditMousecQueue.getEntry(g).getIntoNodePath()
          radius = aspect2d.getRelativeVector(
                   intonode.getParent(),Point3(intonode.getBounds().getRadius(),0,0))[0]
          # respects the smallest object, so that it's easy to select it,
          # especially when it's covered by larger geom
          if radius<minRadius:
             minRadius=radius
             smallestIntonode=intonode

      # if the last collided geom is still the one now,
      # no interesting new things to do, get outta here
      if self.collideInto==smallestIntonode:
         return Task.cont

      self.collideInto=smallestIntonode
      if self.parentsList:
         self.nodesListGUI.show()

      currNode=self.collideInto
      # if the collided geom is the Direct GUI card,
      # use the Direct GUI object as the lowest selectable node, not the card
      if smallestIntonode.getName()=='card 4 DGUI collision':
         currNode=currNode.getParent()
      # if not already selected, then select it
      if currNode!=self.selectedNP:
         self.selectA2Dchild(currNode,self.lastMpos)
      return Task.cont
#
#____________________END OF  M A I N  L O O P_______________________#
#####################################################################

  def clearCollNselectA2Dchild(self,currNode):
      self.collideInto=None
      self.selectA2Dchild(currNode)

  def selectA2Dchild(self,currNode,mpos=None):
      self.origin.show()
      self.nodeList=[]
      # walk up to find parents
      while currNode!=aspect2d:
            self.nodeList.append(currNode)
            currNode=currNode.getParent()
            if not currNode.hasTag('a2dID'):
               break
      self.nodeList.reverse()

      # update the nodes list GUI frame upon changes of the selectable nodes
      self.nodesFrame['frameSize']=(-1.1,1.1,(-len(self.nodeList)-5)*self.parentsListButtonZspacing,-1.2)
      self.nailNodesListButton.setPos(0,0,(-len(self.nodeList)-5)*self.parentsListButtonZspacing+1.15)
      # update the nodes list GUI if it's not nailed
      if not self.isNodesListNailed:
         if not mpos:
            mpos=Point2(0,0)
         pos=Point3( min(.55,.07+mpos[0]) ,0, min(.9,mpos[1]+.15+len(self.nodeList)*.05*self.parentsListButtonZspacing) )
         self.nodesFrame.setPos(render2d,pos)
         self.__relocateNodesFrame()

      # render check buttons, reuse the existing or create new one if needed
      for n in range(len(self.nodeList)):
          nodeName=' '.join(self.nodeList[n].getName().splitlines())[:25]
          if len(nodeName)==25:
             nodeName+='....'
          text='<%s>   %s' %(self.nodeList[n].node().getType(),nodeName)
          # create new button if no more reusable one
          if n>=len(self.nodesListChkBtns):
             DCB=myDirectCheckButton(parent=self.nodesFrame, text=text,
                   text_font=self.transMtl, text_scale=.9, text_align=TextNode.ALeft,
                   pos=Point3(1,0,(-n-1.8)*self.parentsListButtonZspacing), frameColor=(.6,.8,1,.8),
                   indicatorValue=0, indicator_frameVisibleScale=(.6,.6),
                   command=self.selectNode, extraArgs=[n],
                   enableEdit=0, suppressMouse=0)
             DCB.indicator.setTransparency(1)
             DCB.indicator['text']=self.DCB_bullet
             DCB.indicator['text_pos']=(-.05,.3)
             DCB.indicator['text_scale']=2.5
             self.nodesListChkBtns.append(DCB)
          else:
          # or reuse the existing one
             self.nodesListChkBtns[n]['text']=text
             self.nodesListChkBtns[n].resetFrameSize()
             self.nodesListChkBtns[n].show()
      # clean up the remaining (unused) check buttons
      for n in range(len(self.nodesListChkBtns)-len(self.nodeList)):
          self.nodesListChkBtns[n+len(self.nodeList)].hide()
      # update the nodes list GUI and A2D scenegraph List
      status=self.nodeList[-1]!=self.selectedNP
      self.selectNode(status,len(self.nodeList)-1)
      # need to get the mouse watcher regions immediately, so force render frame now
      self.renderFrame()
      # get the mouse watcher regions to check if the mouse is hovering on them
      MWgroup=base.mouseWatcherNode.getGroup(0)
      self.nodesListGUIregions = [MWgroup.getRegion(num) for num in range(MWgroup.getNumRegions())]

  def selectNode(self,status,curr):
      # those check buttons must act like radio buttons, so hack it here
      if status:
         for cb in self.nodesListChkBtns:
             cb['indicatorValue']=0
             cb['indicator_frameColor']=(.1,.1,.1,.7)
             cb['frameColor']=(.2,.2,.2,.8)
             cb['text_fg']=(1,1,1,1)
      self.nodesListChkBtns[curr]['indicatorValue']=1
      self.nodesListChkBtns[curr]['indicator_frameColor']=(1,1,0,1)
      self.nodesListChkBtns[curr]['frameColor']=(.5,0,0,.8)
      self.nodesListChkBtns[curr]['text_fg']=(1,1,0,1)
      # set and highlight the node only if the indicatorValue=1 upon click
      if status:
         if self.selectedNP:
            # restore it back to it's original render bin
            if self.selectedNodeBin:
               self.selectedNP.setBin(self.selectedNodeBin,self.selectedNodeDrawOrder)
            else:
               self.selectedNP.clearBin()
            self.BBoxAnimIval.finish()
            self.selectedNP.find('**/BBoxVis').removeNode()
            # clear the highlight of the last selected node on A2D scenegraph list
#             self.A2dSGBrowser.restoreNodeButton2Normal()
         self.selectedNP=self.nodeList[curr]
         print self.selectedNP.getName(),'SELECTED'
         self.selectedNodeBin=self.selectedNP.getBinName()
         self.selectedNodeDrawOrder=self.selectedNP.getBinDrawOrder()
         self.selectedNP.setBin('selectedNP',10)
         self.selectedNodeIdx=curr
         self.moveOrigin(self.selectedNP)
         self.nodeColorIval(0)
         # update the properties entries
         self.updateNodePROP_pos()
         self.updateNodePROP_scale()
         # activate COPY properties button
         self.nodePROP_copy.setColorScale(1,1,1,1)
         self.nodePROP_copy.node().setActive(1)
         if self.nodePROP_clipboard!=None:
            if self.nodePROP_clipboard[0]==self.selectedNP:
               # disable PASTE properties button
               self.disableNodePROP_paste()
            else:
               # enable PASTE properties button
               self.nodePROP_paste.setColorScale(1,1,1,1)
               self.nodePROP_paste.node().setActive(1)
         if not self.A2dSGBrowser.isHidden():
            # highlight the node on A2d scenegraph list
            self.A2dSGBrowser.selectNode(self.selectedNP,runUserCommand=False)
         # get the markers under the selected node,
         # to build bounding box visualizer
         self.getSelfSnapMarkers()
         self.drawBBox(self.selectedNP)
      # status=0 means you clicked an already selected button,
      # so let's edit the node :D
      else:
         self.toggleSelectEdit(0)

  def nodeColorIval(self,edit):
      try:
          self.nodeIval.finish()
          self.nodeIval=None
      except:
          pass
      if edit:
         color=Vec4(0,1,.4,.5)
      else:
         color=Vec4(.8,0,0,.5)
      origColor=self.selectedNP.getColorScale()
      self.nodeIval= Sequence(
                     LerpColorScaleInterval(self.selectedNP,.25,color,origColor,blendType='easeOut'),
                     LerpColorScaleInterval(self.selectedNP,.25,origColor,blendType='easeIn') )
      self.nodeIval.loop()

  def isAnyNodeSelected(self):
      if not hasattr(self,'selectedNP'):
         return 0
      if not self.selectedNP:
         return 0
      return 1

  def getSelfSnapMarkers(self):
      self.selfSnapMarkers=None
      self.selfSnapMarkers=self.selectedNP.findAllMatches('**/othOriginMarker')
      self.selfSnapMarkers+=self.selectedNP.findAllMatches('**/geomOriginMarker')
      self.selfSnapMarkers+=self.selectedNP.findAllMatches('**/CornerMarker')
      self.selfSnapMarkers+=self.selectedNP.findAllMatches('**/MidPointMarker')
      self.selfSnapMarkers+=self.selectedNP.findAllMatches('**/OtherNodeCornerMarker')
      self.selfSnapMarkers+=self.selectedNP.findAllMatches('**/OtherNodeMidPointMarker')
      self.selfScalingSnapMarkers=None
      nodeType=str(self.selectedNP.node().getType())
      if nodeType=='GeomNode' or nodeType[:2]=='PG':
         self.selfScalingSnapMarkers=self.selectedNP.findAllMatches('CornerMarker')
         self.selfScalingSnapMarkers+=self.selectedNP.findAllMatches('MidPointMarker')
      else:
         self.selfScalingSnapMarkers=self.selectedNP.findAllMatches('OtherNodeCornerMarker')
         self.selfScalingSnapMarkers+=self.selectedNP.findAllMatches('OtherNodeMidPointMarker')
      for m in self.selfScalingSnapMarkers:
          self.selfSnapMarkers.removePath(m)
      self.allSelfSnapMarkers=None
      self.allSelfSnapMarkers=self.selfSnapMarkers+self.selfScalingSnapMarkers

  def isInsideMarker(self,marker,posA2D,excludedSnapMarkers,snapRadius):
      radius=1.1*snapRadius/self.zoom
      for m in marker:
          if excludedSnapMarkers!=None:
             if m in excludedSnapMarkers:
                continue
          markerPos=m.getPos(aspect2d)
          markerPos.setY(0)
          if (posA2D-markerPos).length()<=radius:
             markerPosR2D=m.getPos(render2d)
             posR2D=Point2(markerPosR2D[0],markerPosR2D[2])
             tex=m.getTexture()
             snapTex=m.find('**/markerMaster').getPythonTag('snapTex')
             m.setTexture(snapTex,1)
             if m in self.allSelfSnapMarkers:
                mustBeRestored=1
                if taskMgr.hasTaskNamed('restoreSnappedSelfMarkerTex'):
                   delayedTask=taskMgr.getTasksNamed('restoreSnappedSelfMarkerTex')
                   for t in delayedTask:
                       if t.marker==m:
                          t.wakeTime = globalClock.getFrameTime() + 1
                          mustBeRestored=0
                          break
                if mustBeRestored:
                   t=taskMgr.doMethodLater(1,m.setTexture,'restoreSnappedSelfMarkerTex',extraArgs=[tex,1])
                   t.marker=m
             else:
                taskMgr.doMethodLater(1,m.clearTexture,'clearMarkerTex',extraArgs=[])
             return True,posR2D,m.getParent().getName(),m
      return False,None,None,None

  def checkSnap(self,posR2D,useSelfMarkers=0,useScalingSelfMarkers=0,noExtension=0,markMarker=0):
      if useScalingSelfMarkers:
         excludedSnapMarkers=self.selfSnapMarkers
      elif not useSelfMarkers:
         excludedSnapMarkers=self.allSelfSnapMarkers
      else:
         excludedSnapMarkers=None
      isSnap=0
      isExtSnap=0
      markerNP=None
      posR2D=Point2(posR2D)
      posA2D=aspect2d.getRelativePoint(render2d,Point3(posR2D[0],0,posR2D[1]))
      # snap to grid
      if not self.a2dGrid.isHidden():
         xGrid=self.XgridSpacingR2D*self.zoom
         zGrid=self.ZgridSpacing*self.zoom
         a2dOffset= Point2( aspect2d.getX()%xGrid,
                            aspect2d.getZ()%zGrid)
         posR2D = Point2( self.ROUND_TO(posR2D[0]-a2dOffset[0],xGrid),
                         self.ROUND_TO(posR2D[1]-a2dOffset[1],zGrid) ) + a2dOffset
      # snap to other node origin
      if self.isSnapOtherOrigin:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_OtherOrigin,posA2D,excludedSnapMarkers,self.othNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'other node SNAP :',parent
      # snap to GeomNode origin
      if self.isSnapGeomOrigin:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_GeomOrigin,posA2D,excludedSnapMarkers,self.geomNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'geom origin SNAP :',parent
      # snap to bounding box's corner
      if self.isSnapCorners:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_Corners,posA2D,excludedSnapMarkers,self.geomNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'corner SNAP :',parent
      if self.isSnapOtherCorners:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_OtherCorners,posA2D,excludedSnapMarkers,self.othNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'other corner SNAP :',parent
      # snap to bounding box's middle point
      if self.isSnapMidPoints:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_MidPoints,posA2D,excludedSnapMarkers,self.geomNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'mid point SNAP :',parent
      if self.isSnapOtherMidPoints:
         snap,pos,parent,marker = self.isInsideMarker(
             self.snapMarker_OtherMidPoints,posA2D,excludedSnapMarkers,self.othNodeSnapRadius)
         if snap:
            posR2D=pos
            isSnap=1
            markerNP=marker
            print 'other mid point SNAP :',parent

      # attach the extension lines to the newly snapped marker
      if (
         markerNP and
         markerNP not in self.lastSnappedMarkers and
         markerNP not in self.allSelfSnapMarkers and
         markMarker
         ):
         self.add2LastSnappedMarkers(markerNP)
      # check for adjacent X or Z position of the previously snapped markers
      if not isSnap and not noExtension:
         Xsnap=None
         Zsnap=None
         radius=1.1*self.geomNodeSnapRadius
         for m in self.lastSnappedMarkers:
             markerName=m.getName()
             if (
                (markerName=='geomOriginMarker' and not self.isSnapGeomOrigin) or
                (markerName=='othOriginMarker' and not self.isSnapOtherOrigin) or
                (markerName=='CornerMarker' and not self.isSnapCorners) or
                (markerName=='MidPointMarker' and not self.isSnapMidPoints) or
                (markerName=='OtherNodeCornerMarker' and not self.isSnapOtherCorners) or
                (markerName=='OtherNodeMidPointMarker' and not self.isSnapOtherMidPoints)
                ):
                continue
             hors, vert=self.lastSnappedMarkers[m][:2]
             hors.hide()
             vert.hide()
             Xpos=m.getX(render2d)
             Zpos=m.getZ(render2d)
             if Xsnap==None:
                if abs(posR2D[0]-Xpos)<=radius:
                   Xsnap=Xpos
                   vert.setR(render2d,-90+180*(posR2D[1]<Zpos))
                   posR2D.setX(Xsnap)
                   isExtSnap=1
                   Xmarker=m
             if Zsnap==None:
                if abs(posR2D[1]-Zpos)<=radius:
                   Zsnap=Zpos
                   hors.setR(render2d,180*(posR2D[0]<Xpos))
                   posR2D.setY(Zsnap)
                   isExtSnap=1
                   Zmarker=m
         if Zsnap!=None:
            hors=self.lastSnappedMarkers[Zmarker][0]
            hors.getChild(0).setX(render2d,posR2D[0])
            hors.show()
         if Xsnap!=None:
            vert=self.lastSnappedMarkers[Xmarker][1]
            vert.getChild(0).setZ(render2d,posR2D[1])
            vert.show()
         if Xsnap!=None and Zsnap!=None and Zmarker!=Xmarker:
            self.intersectionMarker.show()
            self.intersectionMarker.setPos(render2d,Point3(posR2D[0],0,posR2D[1]))
         else:
            self.intersectionMarker.hide()
      else:
         self.intersectionMarker.hide()
         for m in self.lastSnappedMarkers:
             self.lastSnappedMarkers[m][0].hide()
             self.lastSnappedMarkers[m][1].hide()
      return posR2D, isSnap, isExtSnap, markerNP

  def add2LastSnappedMarkers(self,markerNP):
      # horisontal extension line
      hors=markerNP.attachNewNode('')
      clipNode = PlaneNode('',Plane(Vec3(-1,0,0), Point3(0,0,0)))
      clipNP = hors.attachNewNode(clipNode)
      hors.setClipPlane( clipNP )
      self.horisontalSnapLine.instanceUnderNode(hors,'')
      hors.setTag('A2Dedit component','')
      hors.hide()
      # vertical extension line
      vert=markerNP.attachNewNode('')
      clipNode = PlaneNode('',Plane(Vec3(-1,0,0), Point3(0,0,0)))
      clipNP = vert.attachNewNode(clipNode)
      vert.setClipPlane( clipNP )
      self.verticalSnapLine.instanceUnderNode(vert,'')
      vert.setTag('A2Dedit component','')
      vert.hide()
      # attach a notifier point to the marker
      notifierPoint=self.snapMarkerPoint.instanceUnderNode(markerNP,'')
      # store the lines to dict
      self.lastSnappedMarkers[markerNP]=(hors,vert,notifierPoint)

  def moveOrigin(self,np):
      self.origin.setPos(np,0,0,0)
      self.updateXZtext(0,np.getX(self.fakeRender2d),np.getZ(self.fakeRender2d))

  def updateXZtext(self,time=0,x=None,y=None):
      if x is not None:
         self.nodeXtextBottom.setText('(  Xrel : %.4f  )'%x)
      OposX=self.origin.getX(self.a2dNormalScaled)
      x=0
      if abs(OposX)>self.Xthreshold:
         x=-OposX+self.Xthreshold*abs(OposX)/OposX
      pos=(x,self.nodeXtextZ-self.origin.getZ(self.a2dNormalScaled))
      pos2=Point3(0,0,-2.*self.nodeXtextZ-.023)
      self.nodeXtextBottom['pos']=pos
      self.nodeXtextTop.setPos(pos2)
      if y is not None:
         self.nodeZtextLeft.setText('(  Zrel : %.4f  )'%y)
      OposZ=self.origin.getZ(self.a2dNormalScaled)
      z=0
      if abs(OposZ)>self.Zthreshold:
         z=-OposZ+self.Zthreshold*abs(OposZ)/OposZ
      pos=self.a2dNormalScaled.getRelativePoint(render2d,Point3(self.nodeZtextX-self.origin.getX(render2d),z,0))
      pos2=self.a2dNormalScaled.getRelativePoint(render2d,Point3(-2.*self.nodeZtextX+.015,0,0))
      self.nodeZtextLeft['pos']=pos
      self.nodeZtextRight.setPos(pos2)

  def cycleNode(self,dir,delay):
      # cycle on the nodes list using keyboard arrows
      try:
         if not taskMgr.hasTaskNamed('steppingOnNode') and len(self.nodeList)>1:
            # give a little delay
            taskMgr.doMethodLater(delay,self.stepOnNode,'steppingOnNode',[dir])
      except:
          pass

  def stepOnNode(self,dir):
      self.selectedNodeIdx+=dir
      self.selectedNodeIdx%=len(self.nodeList)
      self.selectNode(1,self.selectedNodeIdx)

  def isHoverOnNodePropsFrame(self,pos):
      hover=0
      if not self.nodePropsFrame.isHidden():
         b1,b2=self.nodePropsFrame.getTightBounds()
         hover = ( pos[0]>b1[0]/self.aspRatio and pos[0]<b2[0]/self.aspRatio and
                   pos[1]>b1[2] and pos[1]<b2[2] )
      return hover

  def startDrag(self):
      mpos=self.lastMpos
      if self.isHoverOnNodePropsFrame(mpos):
         return
      posSnap, self.isMouseSnap, isExtSnap, markerNP=self.checkSnap(Point2(mpos),useSelfMarkers=1,noExtension=1,markMarker=1)
      if self.isMouseSnap:
         mpos=posSnap
         base.winList[0].movePointer(0, *self.screenPixel(mpos))
      self.editTask=taskMgr.add(self.dragTask,'draggingNode')
      self.editTask.pos=self.selectedNP.getPos(render2d)
      self.editTask.pos2d=Point2(self.editTask.pos[0],self.editTask.pos[2])
      self.editTask.offset=self.editTask.pos2d-mpos
      self.orthoAxisTrack.setPos(self.selectedNP.getPos(aspect2d))
#       self.editingMpos=self.lastMpos
      self.editingMpos=Point2(101010,101010)

  def dragTask(self,obj):
      if self.editingMpos==self.lastMpos:
         return Task.cont
      self.editingMpos=self.lastMpos
      if self.isMouseSnap:
         posR2D=self.checkSnap(self.editingMpos,markMarker=1)[0]
         posR2D+=obj.offset
      else:
         posR2D=self.checkSnap(Point2(self.editingMpos+obj.offset),markMarker=1)[0]

      # ortho lock
      if self.ortho:
         vec=obj.pos2d-posR2D
         delta=aspect2d.getRelativeVector(render2d,Vec3(vec[0],0,vec[1]))
         if abs(delta[0])>abs(delta[2]):
            posR2D.setY(obj.pos2d[1])
         else:
            posR2D.setX(obj.pos2d[0])
         base.winList[0].movePointer(0, *self.screenPixel(posR2D-obj.offset))

      self.selectedNP.setPos(render2d,Point3(posR2D[0],obj.pos[1],posR2D[1]))
      self.moveOrigin(self.selectedNP)
      return Task.cont

  def getScalingAnchor(self,markerName):
      anchorName=self.anchorMarkerName[markerName]
      for m in self.selfScalingSnapMarkers:
          if m.getTag('markerName')==anchorName:
             return m

  def hideNonScalingSelfMarkers(self):
      for m in self.selfSnapMarkers:
          m.stash() # don't change visibility state, stash it instead
          
  def showNonScalingSelfMarkers(self):
      for m in self.selfSnapMarkers:
          m.unstash() # don't change visibility state, unstash it instead

  def startScale(self):
      mpos=self.lastMpos
      if self.isHoverOnNodePropsFrame(mpos):
         return
      self.isMouseSnap=0
      posA2D=aspect2d.getRelativePoint(render2d,Point3(mpos[0],0,mpos[1]))
      snap,pos,parent,marker = self.isInsideMarker(
          self.selfScalingSnapMarkers,posA2D,None,self.othNodeSnapRadius)
      if snap:
         markerType=marker.getName()
         if not (
            (markerType=='CornerMarker' and not self.isSnapCorners) or
            (markerType=='MidPointMarker' and not self.isSnapMidPoints) or
            (markerType=='OtherNodeCornerMarker' and not self.isSnapOtherCorners) or
            (markerType=='OtherNodeMidPointMarker' and not self.isSnapOtherMidPoints)
            ):
            markerName=marker.getTag('markerName')
            anchorMarker=self.getScalingAnchor(markerName)
            self.anchor=self.selectedNP.getParent().attachNewNode('')
            self.anchor.setPos(anchorMarker.getPos(self.selectedNP.getParent()))
            self.selectedNP.wrtReparentTo(self.anchor)
            base.winList[0].movePointer(0, *self.screenPixel(mpos))
            self.isMouseSnap=1
#       else:
# #          nodeType=str(self.selectedNP.node().getType())
# #          if nodeType=='GeomNode' or nodeType[:2]=='PG':  # GeomNode or PG*
#          mpos,snap,isExtSnap,marker=self.checkSnap(Point2(mpos),useScalingSelfMarkers=1,noExtension=1,markMarker=1)
#          if snap:
#             if marker.getParent()==self.selectedNP:
#                markerName=marker.getTag('markerName')
#                anchorMarker=self.getScalingAnchor(markerName)
#                self.anchor=self.selectedNP.getParent().attachNewNode('')
#                self.anchor.setPos(anchorMarker.getPos(self.selectedNP.getParent()))
#                self.selectedNP.wrtReparentTo(self.anchor)
#                base.winList[0].movePointer(0, *self.screenPixel(mpos))
#                self.isMouseSnap=1
      self.editTask=taskMgr.add(self.scaleTask,'scalingNode')
      if self.isMouseSnap:
         posA2D=Point3(self.anchor.getX(aspect2d),0,self.anchor.getZ(aspect2d))
         posR2D=self.anchor.getPos(render2d)
         self.editTask.posR2D=Point2(posR2D[0],posR2D[2])
         self.editTask.scale=self.anchor.getScale()
         self.editTask.len=(posA2D-marker.getPos(aspect2d)).length()
         self.editTask.marker=marker
         vec=marker.getPos(render2d)-posR2D
         if abs(vec[0])<=1e-6:
            self.editTask.XdivZR2D=0
            self.editTask.zeroDiv=1
         elif abs(vec[2])<=1e-6:
            self.editTask.XdivZR2D=1
            self.editTask.zeroDiv=1
         else:
            self.editTask.XdivZR2D=vec[0]/vec[2]
            self.editTask.zeroDiv=0
         self.moveOrigin(self.anchor)
      else:
         posA2D=Point3(self.selectedNP.getX(aspect2d),0,self.selectedNP.getZ(aspect2d))
         mposA2D=aspect2d.getRelativePoint(render2d,Point3(mpos[0],posA2D[1],mpos[1]))
         self.editTask.scale=self.selectedNP.getScale()
         self.editTask.len=(posA2D-mposA2D).length()
         self.moveOrigin(self.selectedNP)
      self.editTask.pos=posA2D
      self.circles.setScale(self.editTask.len)
      self.circles.show()
#       self.editingMpos=self.lastMpos
      self.editingMpos=Point2(101010,101010)

  def scaleTask(self,obj):
      if self.editingMpos==self.lastMpos:
         return Task.cont
      self.editingMpos=self.lastMpos
      mposA2D=aspect2d.getRelativePoint(render2d,Point3(self.lastMpos[0],0,self.lastMpos[1]))
      vecLen=(mposA2D-obj.pos).length()
      if self.isMouseSnap:
         self.anchor.setScale(obj.scale*vecLen/obj.len)
         pos=obj.marker.getPos(render2d)
         posR2D=Point2(pos[0],pos[2])
         posSnap, isSnap, isExtSnap, markerNP=self.checkSnap(posR2D,markMarker=1)
         if posSnap!=posR2D:
            vec=Point2(posR2D-posSnap)
            if isExtSnap:
               swap=vec[0]
               vec.setX(vec[1])
               vec.setY(swap)
            if obj.zeroDiv:
#                posR2D=posSnap
               if obj.XdivZR2D:
                  posR2D.setX(posSnap[0])
               else:
                  posR2D.setY(posSnap[1])
            elif abs(vec[0])<abs(vec[1]):
               posR2D.setX(posSnap[0])
               posR2D.setY(obj.posR2D[1]+(posSnap[0]-obj.posR2D[0])/obj.XdivZR2D )
            else:
               posR2D.setY(posSnap[1])
               posR2D.setX(obj.posR2D[0]+(posSnap[1]-obj.posR2D[1])*obj.XdivZR2D )
            posA2D=aspect2d.getRelativePoint(render2d,Point3(posR2D[0],0,posR2D[1]))
            vecLen=Vec3(posA2D-obj.pos).length()
            self.anchor.setScale(obj.scale*vecLen/obj.len)
      else:
         self.selectedNP.setScale(obj.scale*vecLen/obj.len)
      self.circles.setScale(vecLen)
      return Task.cont

  def stopDragScale(self,updateHistory=1):
      if not (taskMgr.hasTaskNamed('draggingNode') or taskMgr.hasTaskNamed('scalingNode')):
         return
      taskMgr.remove('draggingNode')
      if taskMgr.hasTaskNamed('scalingNode'):
         self.circles.hide()
         if self.isMouseSnap:
#             mpos,snap,isExtSnap,marker=self.checkSnap(self.lastMpos)
#             if snap:
#                task=taskMgr.getTasksNamed('scalingNode')[0]
#                mposA2D=aspect2d.getRelativePoint(render2d,Point3(mpos[0],0,mpos[1]))
#                vec=(mposA2D-task.pos).length()
#                self.anchor.setScale(task.scale*vec/task.len)
#                base.winList[0].movePointer(0, *self.screenPixel(mpos))
            self.selectedNP.wrtReparentTo(self.anchor.getParent())
            self.anchor.removeNode()
         taskMgr.remove('scalingNode')
      self.orthoAxisTrack.setPos(self.selectedNP.getPos(aspect2d))
      self.moveOrigin(self.selectedNP)
      self.intersectionMarker.hide()
      self.editTask=None
      for m in self.lastSnappedMarkers:
          self.lastSnappedMarkers[m][0].hide()
          self.lastSnappedMarkers[m][1].hide()
      if updateHistory:
         self.transfMat=Mat4(self.selectedNP.getMat())
         self.history_EditMode=self.history_EditMode[:self.history_EditModeIdx+1]
         self.history_EditMode.append(self.transfMat)
         self.history_EditModeIdx+=1
         # update the properties entries
         self.updateNodePROP_pos()
         self.updateNodePROP_scale()
      self.updateParentsBBox(self.selectedNP)

  def cancelDragScale(self):
      # user cancels it in the middle of transforming the node
      if taskMgr.hasTaskNamed('draggingNode') or taskMgr.hasTaskNamed('scalingNode'):
         if taskMgr.hasTaskNamed('scalingNode') and self.isMouseSnap:
            self.selectedNP.reparentTo(self.anchor.getParent())
            self.anchor.removeNode()
         self.selectedNP.setMat(self.transfMat)
         self.isMouseSnap=0
         self.stopDragScale(updateHistory=0)
      else:
         self.toggleSelectEdit(0)

  def markSnapMarker(self):
      posSnap, snap, isExtSnap, markerNP=self.checkSnap(self.lastMpos,noExtension=1)
      if snap:
         if markerNP in self.lastSnappedMarkers:
            self.lastSnappedMarkers[markerNP][0].removeNode()
            self.lastSnappedMarkers[markerNP][1].removeNode()
            self.lastSnappedMarkers[markerNP][2].removeNode()
            del self.lastSnappedMarkers[markerNP]
         else:
            self.add2LastSnappedMarkers(markerNP)

  def toggleSelectEdit(self,saveChanges):
      if (
         self.selectedNP==None or
         taskMgr.hasTaskNamed('draggingNode') or
         taskMgr.hasTaskNamed('scalingNode') or
         taskMgr.hasTaskNamed('dragA2D')
         ):
         return
      self.editMode=not self.editMode
      if self.editMode:
         taskMgr.remove('steppingOnNode')
         self.notifierText.show()
         self.orthoAxisTrack.setPos(self.selectedNP.getPos(aspect2d))
         self.nodesListGUI.hide()
         self.A2dSGBrowser.hide()
         self.toggleCollision(0)
         self.lastSnappedMarkers={}
         # distinguish snap markers under the edited node, override the texture
         for m in self.allSelfSnapMarkers:
             editTex=m.find('**/markerMaster').getPythonTag('editTex')
             m.setTexture(editTex,1)
         self.text_A2dEditHelp[3]=self.text_A2dEditHelp[1]
         self.A2dEditLegend.setText(self.text_A2dEditHelp[self.legendOnOff*3])
         PolyButton.setBTprefix(self.MODE_a2dEdit)
         self.transfMat=Mat4(self.selectedNP.getMat())
         self.initTransfMat=Mat4(self.transfMat)
         self.nodeColorIval(1)
         self.history_EditMode=None
         self.history_EditMode=[self.transfMat]
         self.history_EditModeIdx=0
         print 'EDITING',self.nodesListChkBtns[self.selectedNodeIdx]['text']
      else:
         self.stopDragScale(updateHistory=0)
         self.toggleCollision(1)
         self.notifierText.hide()
         if self.parentsList:
            self.nodesListGUI.show()
         else:
            self.A2dSGBrowser.show()
            # highlight the node on A2d scenegraph list
            if self.selectedNP is not None:
               self.A2dSGBrowser.selectNode(self.selectedNP,runUserCommand=False)
         self.text_A2dEditHelp[3]=self.text_A2dEditHelp[1]+self.text_A2dEditHelp[2]
         self.A2dEditLegend.setText(self.text_A2dEditHelp[self.legendOnOff*3])
         self.lastMpos=Point2(100,100)
         PolyButton.setBTprefix(self.MODE_a2dSelect)
         if not saveChanges:
            self.selectedNP.setMat(self.history_EditMode[0])
         else:
            self.history_SelectMode=self.history_SelectMode[:self.history_SelectModeIdx+1]
            self.history_SelectMode.append([self.selectedNP,[self.initTransfMat,Mat4(self.selectedNP.getMat())]])
            self.history_SelectModeIdx+=1
         self.moveOrigin(self.selectedNP)
         self.nodeColorIval(0)
         if taskMgr.hasTaskNamed('restoreSnappedSelfMarkerTex'):
            taskMgr.remove('restoreSnappedSelfMarkerTex')
         for m in self.allSelfSnapMarkers:
             m.clearTexture()
         for m in self.lastSnappedMarkers:
             self.lastSnappedMarkers[m][0].removeNode()
             self.lastSnappedMarkers[m][1].removeNode()
             self.lastSnappedMarkers[m][2].removeNode()
             self.lastSnappedMarkers[m]=None
         self.lastSnappedMarkers=None
         if self.selectedNodeIdx>0:
            self.updateParentsBBox(self.selectedNP)
            # redraw the bounding box
            self.BBoxAnimIval.finish()
            self.selectedNP.find('**/BBoxVis').removeNode()
            self.drawBBox(self.selectedNP)

  def updateParentsBBox(self,np):
      currNode=np.getParent()
      if currNode==aspect2d or not currNode.hasTag('a2dID'):
         return
      parentsList=[]
      # walk up to find parents
      while currNode!=aspect2d:
            parentsList.append(currNode)
            currNode=currNode.getParent()
            if not currNode.hasTag('a2dID'):
               break
      parentsList.reverse()
      # find all children's snap markers to be excluded from contributing to their parent's bounds
      objChildren=parentsList[0].findAllMatches('**/=A2Dedit component')

      for parent in parentsList:
          parent2=parent.getParent()
          # save original rotation and clear it to get the "real" bounding box
          origHpr=parent.getHpr()
          parent.setHpr(0,0,0)
          objChildren.stash() # exclude markers
          bounds=parent.getTightBounds() # bounding box without rotation
          objChildren.unstash() # restore them back so I can find them
          #_______________________________________
          bl=parent.find('=markerName=bottomLeft')
          bl.setPos(parent2,bounds[0][0],0,bounds[0][2])
          #_______________________________________
          tl=parent.find('=markerName=topLeft')
          tl.setPos(parent2,bounds[0][0],0,bounds[1][2])
          #_______________________________________
          br=parent.find('=markerName=bottomRight')
          br.setPos(parent2,bounds[1][0],0,bounds[0][2])
          #_______________________________________
          tr=parent.find('=markerName=topRight')
          tr.setPos(parent2,bounds[1][0],0,bounds[1][2])
          #===================================================
          t=parent.find('=markerName=top')
          t.setPos(parent2,PU.average(bounds[0][0],bounds[1][0]),0,bounds[1][2])
          #_______________________________________
          b=parent.find('=markerName=bottom')
          b.setPos(parent2,PU.average(bounds[0][0],bounds[1][0]),0,bounds[0][2])
          #_______________________________________
          l=parent.find('=markerName=left')
          l.setPos(parent2,bounds[0][0],0,PU.average(bounds[0][2],bounds[1][2]))
          #_______________________________________
          r=parent.find('=markerName=right')
          r.setPos(parent2,bounds[1][0],0,PU.average(bounds[0][2],bounds[1][2]))
          #_______________________________________
          parent.setHpr(origHpr)

  def drawBBox(self,np):
      tl=np.find('=markerName=topLeft').getPos()
      tr=np.find('=markerName=topRight').getPos()
      br=np.find('=markerName=bottomRight').getPos()
      bl=np.find('=markerName=bottomLeft').getPos()
      BBoxVec=tr-tl
      length=render2d.getRelativeVector(np,tr-tl).length()
      width=render2d.getRelativeVector(np,tr-br).length()
      WdivL=width/length

      vdata = GeomVertexData('bbox', self.BBoxGeomVertexFormat, Geom.UHStatic)
      vertex = GeomVertexWriter(vdata, 'vertex')
      color = GeomVertexWriter(vdata, 'color')
      texcoord = GeomVertexWriter(vdata, 'texcoord')
      #_____________________________________
      vertex.addData3f(tl)
      color.addData4f(1, 1, 1, 1)
      texcoord.addData2f(0, 0)
      #_____________________________________
      vertex.addData3f(tr)
      color.addData4f(1, 1, 1, 1)
      texcoord.addData2f(1, 0)
      #_____________________________________
      vertex.addData3f(br)
      color.addData4f(1, 1, 1, 1)
      texcoord.addData2f(1+WdivL, 0)
      #_____________________________________
      vertex.addData3f(bl)
      color.addData4f(1, 1, 1, 1)
      texcoord.addData2f(2.+WdivL, 0)
      #_____________________________________
      vertex.addData3f(tl)
      color.addData4f(1, 1, 1, 1)
      texcoord.addData2f(2.*(1+WdivL), 0)
      #_____________________________________
      box = GeomLinestrips(Geom.UHStatic)
      box.addConsecutiveVertices(0,5)
      box.closePrimitive()

      geom = Geom(vdata)
      geom.addPrimitive(box)
      node = GeomNode('BBoxVis')
      node.addGeom(geom)
      vis=np.attachNewNode(node)
      vis.setRenderModeThickness(3)
      vis.setTexture(self.BBoxTex)
      self.BBoxAnimIval=LerpFunc(self.animBBoxU,duration=.3,extraArgs=[TextureStage.getDefault(),vis,BBoxVec])
      self.BBoxAnimIval.loop()
      
  def animBBoxU(self,u,TS,vis,BBoxVec):
      texScale=render2d.getRelativeVector(self.selectedNP,BBoxVec).length()*20
      vis.setTexScale(TS,texScale,1)
      vis.setTexOffset(TS,u,0)

  def undoSelectMode(self):
      if self.history_SelectModeIdx==-1:
         print 'UNDO : no more'
         return
      self.applyHistorySelectMode(edited=0)
      self.history_SelectModeIdx-=1
      self.moveOrigin(self.selectedNP)

  def redoSelectMode(self):
      if self.history_SelectModeIdx==len(self.history_SelectMode)-1:
         print 'REDO : no more'
         return
      self.history_SelectModeIdx+=1
      self.applyHistorySelectMode(edited=1)
      self.moveOrigin(self.selectedNP)

  def gotoBeginHistorySelectMode(self):
      if self.history_SelectModeIdx==-1:
         return
      while self.history_SelectModeIdx>-1:
          self.applyHistorySelectMode(edited=0)
          self.history_SelectModeIdx-=1
      self.moveOrigin(self.selectedNP)
      # the only way to update the collision visualizer
      self.A2DeditMousecTrav.traverse(aspect2d)
      print '(;P) all session changes discarded ......'

  def gotoEndHistorySelectMode(self):
      if self.history_SelectModeIdx==len(self.history_SelectMode)-1:
         return
      while self.history_SelectModeIdx<len(self.history_SelectMode)-1:
          self.history_SelectModeIdx+=1
          self.applyHistorySelectMode(edited=1)
      self.moveOrigin(self.selectedNP)
      # the only way to update the collision visualizer
      self.A2DeditMousecTrav.traverse(aspect2d)

  def applyHistorySelectMode(self,edited):
      #print 'history :',self.history_SelectModeIdx
      np,transfMat=self.history_SelectMode[self.history_SelectModeIdx]
      np.setMat(transfMat[edited])
      self.updateParentsBBox(np)
      if np==self.selectedNP:
         # redraw the bounding box
         self.BBoxAnimIval.finish()
         try:
            np.find('**/BBoxVis').removeNode()
            self.drawBBox(np)
         except:
            pass
      # update the properties entry
      self.updateNodePROP_pos()
      self.updateNodePROP_scale()

  def undoEditMode(self):
      if self.history_EditModeIdx==0:
         print 'UNDO : no more'
         return
      self.history_EditModeIdx-=1
      self.applyHistoryEditMode()

  def redoEditMode(self):
      if self.history_EditModeIdx==len(self.history_EditMode)-1:
         print 'REDO : no more'
         return
      self.history_EditModeIdx+=1
      self.applyHistoryEditMode()

  def gotoBeginHistoryEditMode(self):
      # restore the original transform
      self.isMouseSnap=0
      self.history_EditModeIdx=0
      self.applyHistoryEditMode()
  
  def gotoEndHistoryEditMode(self):
      # restore the latest transform
      self.isMouseSnap=0
      self.history_EditModeIdx=len(self.history_EditMode)-1
      self.applyHistoryEditMode()
  
  def applyHistoryEditMode(self):
      #print 'history :',self.history_EditModeIdx
      self.transfMat=self.history_EditMode[self.history_EditModeIdx]
      self.selectedNP.setMat(self.transfMat)
      self.orthoAxisTrack.setPos(self.selectedNP.getPos(aspect2d))
      self.moveOrigin(self.selectedNP)
      self.updateParentsBBox(self.selectedNP)
      # redraw the bounding box
      self.BBoxAnimIval.finish()
      self.selectedNP.find('**/BBoxVis').removeNode()
      self.drawBBox(self.selectedNP)
      # update the properties entry
      self.updateNodePROP_pos()
      self.updateNodePROP_scale()

  def startDragA2D(self):
      if (
          FOCUS_Entry!=None or
            (self.parentsList and
             self.isHoverOnNodesListGUI(self.lastMpos) and
             not self.editMode
            ) or
         self.restoreA2DposNscaleIval.isPlaying() or
         self.focusViewOnNodeIval.isPlaying()
         ):
         return Task.cont
      self.dragA2DTask=taskMgr.add(self.dragA2D,'dragA2D')
      self.dragA2DTask.offset=self.lastMpos-Point2(aspect2d.getX(),aspect2d.getZ())

  def dragA2D(self,t):
      pos=self.lastMpos-t.offset
      newPos=Point3(pos[0],0,pos[1])
      if aspect2d.getPos()!=newPos:
         aspect2d.setPos(newPos)
         self.updateXZtext()
      return Task.cont

  def stopDragA2D(self):
      taskMgr.remove('dragA2D')

  def toggleOrtho(self):
      if taskMgr.hasTaskNamed('scalingNode'):
         return
      self.ortho=not self.ortho
      if self.ortho:
         self.orthoAxisTrack.show()
      else:
         self.orthoAxisTrack.hide()

  def toggleGrid(self):
      if self.a2dGrid.isHidden():
         self.a2dGrid.show()
      else:
         self.a2dGrid.hide()

  def toggleSnapGeomOrigin(self):
      self.isSnapGeomOrigin=not self.isSnapGeomOrigin
      if self.isSnapGeomOrigin:
         for m in self.snapMarker_GeomOrigin:  m.show()
      else:
         for m in self.snapMarker_GeomOrigin:  m.hide()

  def toggleSnapOtherOrigin(self):
      self.isSnapOtherOrigin=not self.isSnapOtherOrigin
      if self.isSnapOtherOrigin:
         for m in self.snapMarker_OtherOrigin:  m.show()
      else:
         for m in self.snapMarker_OtherOrigin:  m.hide()

  def toggleSnapCorners(self):
      self.isSnapCorners=not self.isSnapCorners
      if self.isSnapCorners:
         for m in self.snapMarker_Corners:  m.show()
      else:
         for m in self.snapMarker_Corners:  m.hide()

  def toggleSnapMidPoints(self):
      self.isSnapMidPoints=not self.isSnapMidPoints
      if self.isSnapMidPoints:
         for m in self.snapMarker_MidPoints:  m.show()
      else:
         for m in self.snapMarker_MidPoints:  m.hide()

  def toggleSnapOtherCorners(self):
      self.isSnapOtherCorners=not self.isSnapOtherCorners
      if self.isSnapOtherCorners:
         for m in self.snapMarker_OtherCorners:  m.show()
      else:
         for m in self.snapMarker_OtherCorners:  m.hide()

  def toggleSnapOtherMidPoints(self):
      self.isSnapOtherMidPoints=not self.isSnapOtherMidPoints
      if self.isSnapOtherMidPoints:
         for m in self.snapMarker_OtherMidPoints:  m.show()
      else:
         for m in self.snapMarker_OtherMidPoints:  m.hide()

  def toggleNodesList(self):
      self.parentsList=not self.parentsList
      if self.parentsList:
         if self.selectedNP:
            self.nodesListGUI.show()
         self.A2dSGBrowser.hide()
         self.toggleCollision(1)
      else:
         self.nodesListGUI.hide()
         self.A2dSGBrowser.show()
         # highlight the node on A2d scenegraph list
         if self.selectedNP is not None:
            self.A2dSGBrowser.selectNode(self.selectedNP,runUserCommand=False)
         self.toggleCollision(0)

  def toggleNodeProps(self):
      if self.nodePropsFrame.isHidden():
         self.nodePropsFrame.show()
      else:
         self.nodePropsFrame.hide()

  def toggleCollision(self,status):
      if status:
         if not self.A2DeditMousecTrav.hasCollider(self.A2DeditMouseColNP):
            self.A2DeditMousecTrav.addCollider(self.A2DeditMouseColNP, self.A2DeditMousecQueue)
#          self.A2DeditMousecTrav.showCollisions(aspect2d)
         # scale down the visualized collision point, it's just too big
         for v in aspect2d.findAllMatches('**/+CollisionVisualizer'):
             v.node().setPointScale(.2)
      else:
         self.A2DeditMousecTrav.hideCollisions()
         self.A2DeditMousecQueue.clearEntries()

  def disableDGUI(self):
      self.DGUIlastState=[]
      for g in self.DGUIobjects:
          self.DGUIlastState.append(g.node().getActive())
          g.node().setActive(0)

  def enableDGUI(self):
      for g in range(len(self.DGUIobjects)):
          if self.DGUIlastState[g]:
             self.DGUIobjects[g].node().setActive(1)

  def zoomInA2D(self):
      """ zoom in aspect2d """
      if (
         taskMgr.hasTaskNamed('draggingNode') or
         taskMgr.hasTaskNamed('scalingNode') or
         taskMgr.hasTaskNamed('dragA2D') or
         self.restoreA2DposNscaleIval.isPlaying() or
         self.focusViewOnNodeIval.isPlaying() or
         self.zoom>10000
         ):
         return

      self.zoom*=self.zoomFactor
      oldPos=aspect2d.getPos()
      delta=(Point3(self.lastMpos[0],0,self.lastMpos[1])-oldPos)*(1-self.zoomFactor)
      aspect2d.setPos(oldPos+delta)
      aspect2d.setScale(self.a2dScale*self.zoom)
      self.updateXZtext()
      # the only way to update the collision visualizer
      self.A2DeditMousecTrav.traverse(aspect2d)
#       print self.zoom

  def zoomOutA2D(self):
      """ zoom out aspect2d """
      if (
         self.zoom<.1 or
         taskMgr.hasTaskNamed('draggingNode') or
         taskMgr.hasTaskNamed('scalingNode') or
         taskMgr.hasTaskNamed('dragA2D') or
         self.restoreA2DposNscaleIval.isPlaying() or
         self.focusViewOnNodeIval.isPlaying()
         ):
         return

      self.zoom/=self.zoomFactor
      oldPos=aspect2d.getPos()
      delta=(Point3(self.lastMpos[0],0,self.lastMpos[1])-oldPos)*(1-1./self.zoomFactor)
      aspect2d.setPos(oldPos+delta)
      aspect2d.setScale(self.a2dScale*self.zoom)
      self.updateXZtext()
      # the only way to update the collision visualizer
      self.A2DeditMousecTrav.traverse(aspect2d)

  def zoomNormalA2D(self,duration=.5):
      """ restore aspect2d scale """
      if (
         taskMgr.hasTaskNamed('draggingNode') or
         taskMgr.hasTaskNamed('scalingNode') or
         taskMgr.hasTaskNamed('dragA2D') or
         self.focusViewOnNodeIval.isPlaying()
         ):
         return
      self.zoom=1
      self.restoreA2DposNscaleIval=Sequence(
           Parallel( LerpScaleInterval(aspect2d,duration,self.a2dScale,blendType='easeOut'),
                     LerpPosInterval(aspect2d,duration,Point3(0,0,0),blendType='easeOut'),
                     LerpFunc(self.updateXZtext,duration) ),
           # the only way to update the collision visualizer
           Func(self.A2DeditMousecTrav.traverse, aspect2d) )
      self.restoreA2DposNscaleIval.start()
      
  def focusViewOnNode(self):
      if (
         not self.selectedNP or
         taskMgr.hasTaskNamed('draggingNode') or
         taskMgr.hasTaskNamed('scalingNode') or
         taskMgr.hasTaskNamed('dragA2D') or
         self.restoreA2DposNscaleIval.isPlaying() or
         self.focusViewOnNodeIval.isPlaying()
         ):
         return
      # exclude all children's snap markers from contributing to their parent's bounds
      editingComponent=self.selectedNP.findAllMatches('**/=A2Dedit component')
      editingComponent.stash()
      # check bounds
      try:
         boundsCenter=self.selectedNP.getBounds().getCenter()
         boundsRadius=self.selectedNP.getBounds().getRadius()
         centerR2D=render2d.getRelativePoint(self.selectedNP.getParent(),boundsCenter)
         center=aspect2d.getPos()-centerR2D
         radius=render2d.getRelativeVector(self.selectedNP.getParent(),Vec3(boundsRadius,0,0))[0]
         scaleFactor=(.5/radius)
         self.zoom*=scaleFactor
         newScale=aspect2d.getScale()*scaleFactor
         self.focusViewOnNodeIval=Sequence(
              Parallel( LerpScaleInterval(aspect2d,.5,newScale,blendType='easeOut'),
                        LerpPosInterval(aspect2d,.5,center*scaleFactor,blendType='easeOut'),
                        LerpFunc(self.updateXZtext,.5) ),
              # the only way to update the collision visualizer
              Func(self.A2DeditMousecTrav.traverse, aspect2d) )
         self.focusViewOnNodeIval.start()
      except:
         print 'NODE HAS NO BOUNDS'
      editingComponent.unstash()

  
  def toggleEditLegend(self):
      self.legendOnOff=not self.legendOnOff
      self.A2dEditLegend.setText(self.text_A2dEditHelp[self.legendOnOff*3])

  def toggleEditNotifier(self):
      self.editNotifierOnOff=not self.editNotifierOnOff
      self.notifierText.setText(self.text_EditNotifier[self.editNotifierOnOff])
      self.notifierText.setAlphaScale(.3+.7*self.editNotifierOnOff)

  def nailNodesList(self):
      self.isNodesListNailed=not self.isNodesListNailed
      if self.isNodesListNailed:
         self.nailNodesListButton['text_scale']=(1.5,.7)
      else:
         self.nailNodesListButton['text_scale']=(1.5,1.5)

  # Routines to adjust values (copied from DirectUtils.py)
  def ROUND_TO(self,value, divisor):
      return round(value/float(divisor)) * divisor

  def disableNodePROP_paste(self):
      self.nodePROP_paste.setColorScale(.3,.3,.3,1)
      self.nodePROP_paste.node().setActive(0)

  def nodePROP_allPosActive(self,active):
      self.nodePROP_xPosDCB['indicatorValue']=active
      self.nodePROP_yPosDCB['indicatorValue']=active
      self.nodePROP_zPosDCB['indicatorValue']=active
      self.nodePROP_XposActive(active)
      self.nodePROP_YposActive(active)
      self.nodePROP_ZposActive(active)

  def nodePROP_XposActive(self,active):
      self.nodePROP_XposEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_XposEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allPosDCB()

  def nodePROP_YposActive(self,active):
      self.nodePROP_YposEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_YposEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allPosDCB()

  def nodePROP_ZposActive(self,active):
      self.nodePROP_ZposEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_ZposEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allPosDCB()

  def refreshNodePROP_allPosDCB(self):
      active = self.nodePROP_xPosDCB['indicatorValue']
      active &= self.nodePROP_yPosDCB['indicatorValue']
      active &= self.nodePROP_zPosDCB['indicatorValue']
      self.nodePROP_allPosDCB['indicatorValue']=active

  def nodePROP_allScaleActive(self,active):
      self.nodePROP_xScaleDCB['indicatorValue']=active
      self.nodePROP_yScaleDCB['indicatorValue']=active
      self.nodePROP_zScaleDCB['indicatorValue']=active
      self.nodePROP_XscaleActive(active)
      self.nodePROP_YscaleActive(active)
      self.nodePROP_ZscaleActive(active)

  def nodePROP_XscaleActive(self,active):
      self.nodePROP_XscaleEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_XscaleEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allScaleDCB()

  def nodePROP_YscaleActive(self,active):
      self.nodePROP_YscaleEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_YscaleEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allScaleDCB()

  def nodePROP_ZscaleActive(self,active):
      self.nodePROP_ZscaleEntry.node().setActive(active)
      active=min(active+.3,1)
      self.nodePROP_ZscaleEntry.setColorScale(active,active,active,1)
      self.refreshNodePROP_allScaleDCB()

  def refreshNodePROP_allScaleDCB(self):
      active = self.nodePROP_xScaleDCB['indicatorValue']
      active &= self.nodePROP_yScaleDCB['indicatorValue']
      active &= self.nodePROP_zScaleDCB['indicatorValue']
      self.nodePROP_allScaleDCB['indicatorValue']=active

  def updateNodePROP_pos(self,rel=None):
      if not self.isAnyNodeSelected():
         self.nodePROP_XposEntry.set('')
         self.nodePROP_YposEntry.set('')
         self.nodePROP_ZposEntry.set('')
         return
      if rel==None:
         rel=self.nodePROP_relPos['indicatorValue']
      if rel:
         self.nodePROP_XposEntry.set(str(self.selectedNP.getX(self.fakeRender2d)))
         self.nodePROP_YposEntry.set(str(self.selectedNP.getY(self.fakeRender2d)))
         self.nodePROP_ZposEntry.set(str(self.selectedNP.getZ(self.fakeRender2d)))
      else:
         self.nodePROP_XposEntry.set(str(self.selectedNP.getX()))
         self.nodePROP_YposEntry.set(str(self.selectedNP.getY()))
         self.nodePROP_ZposEntry.set(str(self.selectedNP.getZ()))

  def updateNodePROP_scale(self,rel=None):
      if not self.isAnyNodeSelected():
         self.nodePROP_XscaleEntry.set('')
         self.nodePROP_YscaleEntry.set('')
         self.nodePROP_ZscaleEntry.set('')
         return
      if rel==None:
         rel=self.nodePROP_relScale['indicatorValue']
      if rel:
         self.nodePROP_XscaleEntry.set(str(self.selectedNP.getSx(self.fakeRender2d)))
         self.nodePROP_YscaleEntry.set(str(self.selectedNP.getSy(self.fakeRender2d)))
         self.nodePROP_ZscaleEntry.set(str(self.selectedNP.getSz(self.fakeRender2d)))
      else:
         self.nodePROP_XscaleEntry.set(str(self.selectedNP.getSx()))
         self.nodePROP_YscaleEntry.set(str(self.selectedNP.getSy()))
         self.nodePROP_ZscaleEntry.set(str(self.selectedNP.getSz()))

  def copyNodePROPs(self):
      if not self.isAnyNodeSelected():
         return
      relPos=self.nodePROP_relPos['indicatorValue']
      if relPos:
         pos=self.selectedNP.getPos(self.fakeRender2d)
      else:
         pos=self.selectedNP.getPos()
      relScale=self.nodePROP_relScale['indicatorValue']
      if relScale:
         scale=self.selectedNP.getScale(self.fakeRender2d)
      else:
         scale=self.selectedNP.getScale()
      # store to "clipboard" list
      self.nodePROP_clipboard=[self.selectedNP,relPos,relScale,pos[0],pos[1],pos[2],scale[0],scale[1],scale[2]]
      # update the origin pos
      self.moveOrigin(self.selectedNP)
      # disable PASTE properties button
      self.disableNodePROP_paste()
      # play COPIED text interval
      self.nodePropsCopiedText.setPos(self.selectedNP,0,0,0)
      self.nodePropsCopiedIval.start()

  def pasteNodePROPs(self):
      if not self.isAnyNodeSelected() or self.nodePROP_clipboard==None:
         return
      if self.selectedNP==self.nodePROP_clipboard[0]:
         return
      if not self.editMode:
         # save the original transform matrix to store it to history later
         self.initTransfMat=Mat4(self.selectedNP.getMat())
      # replace pos
      if self.nodePROP_xPosDCB['indicatorValue']:  # X pos filter
         if self.nodePROP_clipboard[1]:  # rel to render2d
            self.selectedNP.setX(self.fakeRender2d,self.nodePROP_clipboard[3])
         else:
            self.selectedNP.setX(self.nodePROP_clipboard[3])
      if self.nodePROP_yPosDCB['indicatorValue']:  # Y pos filter
         if self.nodePROP_clipboard[1]:  # rel to render2d
            self.selectedNP.setY(self.fakeRender2d,self.nodePROP_clipboard[4])
         else:
            self.selectedNP.setY(self.nodePROP_clipboard[4])
      if self.nodePROP_zPosDCB['indicatorValue']:  # Z pos filter
         if self.nodePROP_clipboard[1]:  # rel to render2d
            self.selectedNP.setZ(self.fakeRender2d,self.nodePROP_clipboard[5])
         else:
            self.selectedNP.setZ(self.nodePROP_clipboard[5])
      # replace scale
      if self.nodePROP_xScaleDCB['indicatorValue']:  # X scale filter
         if self.nodePROP_clipboard[2]:  # rel to render2d
            self.selectedNP.setSx(self.fakeRender2d,self.nodePROP_clipboard[6])
         else:
            self.selectedNP.setSx(self.nodePROP_clipboard[6])
      if self.nodePROP_yScaleDCB['indicatorValue']:  # Y scale filter
         if self.nodePROP_clipboard[2]:  # rel to render2d
            self.selectedNP.setSy(self.fakeRender2d,self.nodePROP_clipboard[7])
         else:
            self.selectedNP.setSy(self.nodePROP_clipboard[7])
      if self.nodePROP_zScaleDCB['indicatorValue']:  # Z scale filter
         if self.nodePROP_clipboard[2]:  # rel to render2d
            self.selectedNP.setSz(self.fakeRender2d,self.nodePROP_clipboard[8])
         else:
            self.selectedNP.setSz(self.nodePROP_clipboard[8])
      # update the properties entry
      self.updateNodePROP_pos()
      self.updateNodePROP_scale()
      self.updateAfterNodePROPchanged()
      # play REPLACED text interval
      self.nodePropsReplacedText.setPos(self.selectedNP,0,0,0)
      self.nodePropsReplacedIval.start()

  def playInvalidInput(self,DE):
      # let the user fix the input, so set it to be the focus again
      DE['focus']=1
      self.invalidInputText.setPos(DE.getParent(),NodePath(DE).getBounds().getCenter())
      self.invalidInputTextIval.start()

  def setPROP_pos(self,num,element):
      # nothing selected yet !
      if not self.isAnyNodeSelected() and FOCUS_Entry!=None:
         Sequence(
            Wait(.05),
            Func(FOCUS_Entry.__setattr__,'prevValue',''),
            Func(FOCUS_Entry.restoreEntry),
            Func(FOCUS_Entry.setProp,'focus',0)
         ) .start()
         return
      if not self.editMode:
         # save the original transform matrix to store it to history later
         self.initTransfMat=Mat4(self.selectedNP.getMat())
      if self.nodePROP_relPos['indicatorValue']:
         getattr(self.selectedNP,'set'+element.upper())(self.fakeRender2d,num)
      else:
         getattr(self.selectedNP,'set'+element.upper())(num)
      self.updateAfterNodePROPchanged()

  def setPROP_scale(self,num,element):
      if not self.isAnyNodeSelected() and FOCUS_Entry!=None:
         Sequence(
            Wait(.05),
            Func(FOCUS_Entry.__setattr__,'prevValue',''),
            Func(FOCUS_Entry.restoreEntry),
            Func(FOCUS_Entry.setProp,'focus',0)
         ) .start()
         return
      if not self.editMode:
         # save the original transform matrix to store it to history later
         self.initTransfMat=Mat4(self.selectedNP.getMat())
      if self.nodePROP_relScale['indicatorValue']:
         getattr(self.selectedNP,'setS'+element.lower())(self.fakeRender2d,num)
      else:
         getattr(self.selectedNP,'setS'+element.lower())(num)
      self.updateAfterNodePROPchanged()

  def updateAfterNodePROPchanged(self):
      if self.editMode:
         # add the changes to Edit Mode history
         self.transfMat=Mat4(self.selectedNP.getMat())
         self.history_EditMode=self.history_EditMode[:self.history_EditModeIdx+1]
         self.history_EditMode.append(self.transfMat)
         self.history_EditModeIdx+=1
      else:
         # add the changes to Select Mode history
         self.history_SelectMode=self.history_SelectMode[:self.history_SelectModeIdx+1]
         self.history_SelectMode.append([self.selectedNP,[self.initTransfMat,Mat4(self.selectedNP.getMat())]])
         self.history_SelectModeIdx+=1
      # update the origin pos
      self.moveOrigin(self.selectedNP)
      # update bounding box
      self.updateParentsBBox(self.selectedNP)
      # redraw the bounding box
      self.BBoxAnimIval.finish()
      self.selectedNP.find('**/BBoxVis').removeNode()
      self.drawBBox(self.selectedNP)
      # the only way to update the collision visualizer
      self.A2DeditMousecTrav.traverse(aspect2d)

  def recurApplyTransform(self,p,tagChildren=0,excluded=[]):
      if p.getName()=='card 4 DGUI collision':
         return
      parentID=p.getTag('a2dID')
      children=p.getChildren()
      for c in range(len(children)):
          if children[c].getName()!='card 4 DGUI collision':
             # if this is an excluded child, ignore it and step to the next one
             if c in excluded:
                continue
             childID=children[c].getTag('a2dID')
             if tagChildren and not childID and parentID:
                childID=parentID+'-'+str(c)
                children[c].setTag('a2dID',childID)
             if childID in self.a2dDict:
                NodePath(children[c]).setMat(Mat4(*self.a2dDict[childID]))
             # iterate down to object's children
             self.recurApplyTransform(children[c],tagChildren)

  def recurStore2Dict(self,p,tagChildren=0,excluded=[]):
      parentID=p.getTag('a2dID')
      children=p.getChildren()
      for c in range(len(children)):
          # if this is an excluded child, ignore it and step up to the next one
          if c in excluded:
             continue
          childID=children[c].getTag('a2dID')
          if tagChildren and not childID and parentID:
             childID=parentID+'-'+str(c)
             children[c].setTag('a2dID',childID)
          if childID:
             self.a2dDict[childID]=self.decomposeMatrix(children[c].getMat())
          # iterate down to object's children
          self.recurStore2Dict(children[c],tagChildren)

  def save(self,obj,id,useFile=1,excludeChildren=[]):
      """ method to prepare brand new object, or
          to apply transform if it was saved before
             id      : unique hardcoded ID-number
             useFile : 1 = use transform from file
                       0 = use transform from script
      """
      s=str(id)
      # set a tag to link object to the saved-to-file transform
      if not obj.hasTag('a2dID'):
         obj.setTag('a2dID',s)
      # only collide the ray into must-be-saved objects,
      # not into all objects under aspect2d,
      # to save me from fooling myself by editing not-saved objects
      bitmask=obj.getCollideMask()
      bitmask.setBit(self.A2DeditMaskBit)
      obj.setCollideMask(bitmask)
      # if the unique ID already exist in the dictionary,
      # apply the transform
      if s in self.a2dDict and useFile:
         NodePath(obj).setMat(Mat4(*self.a2dDict[s]))
         if not excludeChildren:
            # iterate down to object's children
            self.recurApplyTransform(obj, tagChildren=1)
         elif type(excludeChildren)==types.ListType:
            # iterate down to the desired object's children only
            self.recurApplyTransform(obj, tagChildren=1,excluded=excludeChildren)
      # or store the matrix instead
      else:
         self.a2dDict[s]=self.decomposeMatrix(obj.getMat())
         if not excludeChildren:
            # iterate down to object's children
            self.recurStore2Dict(obj, tagChildren=1)
         elif type(excludeChildren)==types.ListType:
            # iterate down to the desired object's children only
            self.recurStore2Dict(obj, tagChildren=1,excluded=excludeChildren)

  def decomposeMatrix(self,mat):
      elements=[]
      for r in range(4):
          row=mat.getRow(r)
          for c in range(4):
              elements.append(row[c])
      return elements

  def __writeA2DtoFile(self):
      """ method to save the transform dictionary to disk

          Iterate through all nodes currently attached to aspect2d,
          to store the transform to the dictionary, and write it out to disk.
      """
      # iterate down to aspect2d's children
      self.recurStore2Dict(aspect2d)
      # write transform dictionary to disk (as ASCII file)
      self.saveAsASCII()
      # write transform dictionary to disk (as binary file)
      if self.saveBinaryToo:
         file2d=open(self.myFile+self.BINext,'w')
         cPickle.dump(self.a2dDict,file2d)
         file2d.close()

  def saveAsASCII(self):
      file2d=open(self.myFile+self.ASCIIext,'w')
      for o in self.a2dDict:
          file2d.write(o+'\n')
          for el in self.a2dDict[o]:
              file2d.write(' '+str(el))
          file2d.write('\n')
      file2d.close()

  # CHOMBEE's code, altered a bit
  def drawXZCircle(self,radius=1,numSegs=100,angle=360,pos=None,color=None,
                   thickness=None):
      """Draw a circle (if angle = 360) or arc (if angle < 360) on the XZ
      plane at position pos (Vec3)."""

      LS=LineSegs()
      if pos is None: pos = Vec3(0,0,0)
      if angle>360: angle=360

      if color is None: color = (1,1,1,1)
      if thickness is None: thickness = 1

      LS.setColor(*color)
      LS.setThickness(thickness)

      angleRadians = math.radians(angle)
      numSteps = numSegs
      y = math.sin(0) * radius
      x = math.cos(0) * radius
      LS.moveTo(pos + Point3(x,y,0))
      for i in range(1,numSteps + 1):
          a = angleRadians * i / numSteps
          y = math.sin(a) * radius
          x = math.cos(a) * radius
          LS.drawTo(pos + Point3(x,0,y))
      return LS.create()

if __name__=='__main__':
   OnscreenText( text='please run WORLD.PY instead', scale = .05, fg=(1,1,1,1), shadow=(0,0,0,1))
   run()
