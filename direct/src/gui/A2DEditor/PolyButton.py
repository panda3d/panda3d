'''
COLLISION-BASED POLYGONAL BUTTON SYSTEM.

Features :
[=] 4 button states : ready, hover, pressed, disabled.
    ready state uses the already applied texture on the geom (texBaseName-ready.<ext>),
    the other states use texture with -<state> suffix added to the name, replacing -ready.
[=] able to use keyboard arrows and (SHIFT+)TAB to select dialog's buttons, and
    ENTER/SPACE to execute button's command
[=] multiple shortcut keys for each button
[=] button active state setup
[=] dialog and buttons' shortcut keys are activated on demand,
    providing flexible dialog/button in-out effects.
[=] can execute a command when a button is hilighted or not.
'''
__all__ = ['getBTprefix','setBTprefix','myButton','myDialog']

from pandac.PandaModules import *
from direct.showbase.DirectObject import DirectObject
from direct.interval.IntervalGlobal import *
from direct.gui.OnscreenText import OnscreenText
from direct.task import Task
import types


MASK_button=BitMask32.bit(31)
MASK_off=BitMask32.allOff()
buttonCollisionEvent = CollisionHandlerEvent()
listORtuple=(types.ListType,types.TupleType)

BUTTON_PRESSED=None   # to store the pressed/down button
BUTTON_IN=None  # to store button under the mouse pointer


def getBTprefix():
    return collButtonThrower.getPrefix()

def setBTprefix(prefix):
    global BUTTON_PRESSED, BUTTON_IN
    # if any button pressed
    if BUTTON_PRESSED==BUTTON_IN!=None or BUTTON_IN is not None:
       BUTTON_IN.setState(0)
       BUTTON_IN.geom.setScale(BUTTON_IN.geomOrigScale)
       BTprefix=getBTprefix()
       BUTTON_IN.ignore(BTprefix+'mouse1')
       BUTTON_IN.ignore(BTprefix+'mouse1-up')
       if BUTTON_PRESSED:
          BUTTON_PRESSED.offFunc()
       if BUTTON_IN:
          BUTTON_IN.offFunc()
       BUTTON_PRESSED=BUTTON_IN=None
#        print "I'm RESTORED :D"
    collButtonThrower.setPrefix(prefix)
    refreshCollEventPrefix()

def refreshCollEventPrefix():
    BTprefix=getBTprefix()
    buttonCollisionEvent.setInPattern(BTprefix+'buttonIn-%in%(ID)it')
    buttonCollisionEvent.setOutPattern(BTprefix+'buttonOut-%in%(ID)it')

def btnCollLoop(task):
    if base.mouseWatcherNode.hasMouse():
       mpos=base.mouseWatcherNode.getMouse()
       buttonMouseLoc.setPos(render2d,mpos[0],0,mpos[1])
       buttonCTrav.traverse(topParent)
    return Task.cont

def setButtonThrower(bt):
    global collButtonThrower
    collButtonThrower=bt

def reset():
    '''
    clears button collision events
    '''
    buttonCollisionEvent.clear()

def start():
    '''
    starts button collision loop
    '''
    taskMgr.add(btnCollLoop,collTaskNamePrefix+'myButtonCollisionLoop')
    refreshCollEventPrefix()

def stop():
    '''
    stops button collision loop
    '''
    taskMgr.remove(collTaskNamePrefix+'myButtonCollisionLoop')
    reset()

def setup(parent=None,buttonThrower=None,taskNamePrefix='',startNow=True):
    '''
    Sets up button collision system.
      parent   : where to attach the collision ray
      startNow : starts the collision traversal.
         If you don't want to start it immediately, you should call start() or stop()
         on your own whenever you need to start or stop it.
    '''
    global topParent,collButtonThrower,buttonMouseLoc,buttonCTrav,\
           collTaskNamePrefix,delayedCommandExecTaskName
    topParent=parent if parent else render2d
    collTaskNamePrefix=taskNamePrefix
    delayedCommandExecTaskName=collTaskNamePrefix+'doingButtonCommand'
    collButtonThrower=buttonThrower if buttonThrower else base.buttonThrowers[0].node()
    buttonMouseLoc=topParent.attachCollisionRay('mouseRay',
         0,-100,0, 0,1,0, MASK_button,MASK_off)
    buttonCTrav=CollisionTraverser()
    buttonCTrav.addCollider(buttonMouseLoc,buttonCollisionEvent)
    #~ buttonCTrav.showCollisions(topParent)
    refreshCollEventPrefix()
    if startNow:
       start()



class myButtonBehaviour(DirectObject):
  def hoverOnButton(self,button,acceptMousePress=1,entry=None):
      '''
      called when collision IN event occurs
      '''
      global BUTTON_PRESSED, BUTTON_IN
      if BUTTON_IN is not None:
         BUTTON_IN.offButton(BUTTON_IN)
      BUTTON_IN=button
      if BUTTON_PRESSED is None:
         BUTTON_IN.setState(1)
         if acceptMousePress:
            self.acceptOnce(getBTprefix()+'mouse1',self.buttonPressed)
         self.hoverFunc()
      elif BUTTON_PRESSED==BUTTON_IN:
         BUTTON_IN.setState(2)
         BUTTON_PRESSED.geom.setScale(BUTTON_PRESSED.geomOrigScale*.975)
         self.hoverFunc()
#       print 'HOVER ON :',BUTTON_IN.geom.getName()

  def offButton(self,button,removeMousePress=1,entry=None):
      '''
      called when collision OUT event occurs
      '''
      global BUTTON_PRESSED, BUTTON_IN
      button.geom.setScale(button.geomOrigScale)
      button.setState(0)
      if self==BUTTON_IN:
         BUTTON_IN=None
      if removeMousePress:
         self.ignore(getBTprefix()+'mouse1')
      self.offFunc()

  def buttonPressed(self):
      '''
      called when button is depressed
      '''
      global BUTTON_PRESSED, BUTTON_IN
      if BUTTON_IN is not self:
         BUTTON_IN.offButton(BUTTON_IN)
         BUTTON_IN=self
      BUTTON_IN.geom.setScale(BUTTON_IN.geomOrigScale*.975)
      BUTTON_IN.setState(2)
      BTprefix=getBTprefix()
      if self!=BUTTON_PRESSED!=None:
         BUTTON_PRESSED.offButton(BUTTON_PRESSED)
         BUTTON_PRESSED.ignore(BTprefix+'mouse1-up')
      BUTTON_PRESSED=BUTTON_IN
      self.ignore(BTprefix+'mouse1')
      self.acceptOnce(BTprefix+'mouse1-up',self.buttonReleased,[False])
#       print 'PRESSED'

  def buttonReleased(self,delay=True):
      '''
      called when button is released
      '''
      global BUTTON_PRESSED, BUTTON_IN
#       print 'RELEASED'
      self.ignore(getBTprefix()+'mouse1-up')
      if BUTTON_IN is not BUTTON_PRESSED:
         buttonCollisionEvent.clear()
         BUTTON_PRESSED=None
         return
      if not self.stayAlive:
         for c in self.acceptOnceCommand:
             c()
      if delay:
         taskMgr.doMethodLater(.1,self.__doButtonCmd,delayedCommandExecTaskName)
      else:
         self.__doButtonCmd()

  def __doButtonCmd(self,task=None):
      '''
      part of buttonReleased, separated so a pressed state may be rendered
      for a little while if a shortcut key is pressed
      '''
      global BUTTON_PRESSED, BUTTON_IN
      if BUTTON_IN is not None:
         BUTTON_IN.geom.setScale(BUTTON_IN.geomOrigScale)
         # if stayAlive, set state to "hover" (1), else to "ready" (0)
         BUTTON_IN.setState(self.stayAlive)
      buttonCollisionEvent.clear()
      BUTTON_PRESSED=None
      if not self.stayAlive:
         BUTTON_IN=None
      # finally, execute all commands for this button
      if len(self.commands):
         for c in self.commands: c()



class myButton(myButtonBehaviour):
  def __init__(self, np, buttonName, states=(), hitKey='', command=None, arg=None,
        appearEnabled=1, enable=1, stayAlive=1,
        text='', font=None, textScale=.3, textPos=(0,0), textColor=(1,1,1,1),
        hoverFunc=lambda:0, offFunc=lambda:0):
      '''
      Sets the given nodepath as collidable button.

           np : the root node where to find the button geom

           buttonName : name of the geom which will be set as a button

           states : collection of button states texture name suffix
              format :
                 (ready state uses the already applied geom's texture, example :
                     ...path.../texBaseName-ready.png
                      OR
                     ...path.../texBaseName-whatever.png
                 )
                 1st item : hover state texture name suffix, example :
                     given name suffix: 'hover'
                     result : ...path.../texBaseName-hover.png
                      OR
                     given name suffix : 'rollover'
                     result : ...path.../texBaseName-rollover.png

                 2nd item : pressed state texture name suffix, example :
                     given name suffix : 'pressed'
                     result : ...path.../texBaseName-pressed.png
                      OR
                     given name suffix : 'down'
                     result : ...path.../texBaseName-down.png

                 3rd item : disabled state texture name suffix, example :
                     given name suffix : 'disabled'
                     result : ...path.../texBaseName-disabled.png
                      OR
                     given name suffix : 'grayed'
                     result : ...path.../texBaseName-grayed.png

           hitKey : shortcut key to execute button's command(s)

           command : a single or collection of function object executed if this button get selected
              For multiple commands, pass a sequence :
              ( Functor(function1,arg), Functor(function2,arg), ... )

           arg : argument passed to command (for a single command)

           appearEnabled : button's active appearance at the beginning, but doesn't enable button's events

           enable : enable button's events at the beginning

           stayAlive : accept hitKey once OR always accept it.
                To use it as dialog button, you must want stayAlive=0
                to unhook all dialog buttons' events after a dialog button get selected.
                If you leave the events alive after a dialog button get selected
                (but before the buttons get removed), and you hit other button's key,
                that other button's command would be executed. That's obviously dangerous.

           text : button's text string, if any

           font : font to render button's text

           textScale : button's text scale
           textPos   : button's text position, text's center is relative to button geom's center,
                       in range -.5 to .5 relative to button's geom bounds
           textColor : button's text color

           hoverFunc : a function called when the button is hilighted
           offFunc   : a function called when the button is no longer hilighted
      '''
      self.geom=np.find('**/'+buttonName)

      self.geomOrigScale=self.geom.getScale()
      if type(hitKey) in listORtuple:
         self.hitKey=hitKey
      elif hitKey:
         self.hitKey=(hitKey,)
      else:
         self.hitKey=[]
      self.stayAlive = stayAlive
      self.commands=[]
      if callable(command):
         if arg is not None:
            self.commands.append(Functor(command,arg))
         else:
            self.commands.append(command)
      elif type(command) in listORtuple:
            self.commands+=list(command)
      # ready state texture
      readyTex=self.geom.findTexture('*')
      if readyTex:
         readyTexName=readyTex.getFilename()
         readyTexNameStr=str(readyTexName)
         texBaseName=readyTexNameStr[:readyTexNameStr.rfind('-')+1]
         ext='.'+readyTexName.getExtension()
         # hover state texture
         try:
            inTex=loader.loadTexture(texBaseName+states[0]+ext)
         except:
            inTex=None
         # pressed state texture
         try:
            pressedTex=loader.loadTexture(texBaseName+states[1]+ext)
         except:
            pressedTex=None
         # disabled state texture
         try:
            disabledTex=loader.loadTexture(texBaseName+states[2]+ext)
         except:
            disabledTex=None
      else:
         readyTex=None
         inTex=None
         pressedTex=None
         disabledTex=None
      self.stateTextures=[readyTex,inTex,pressedTex,disabledTex]
      self.npID=str(id(self.geom))
      self.geom.setTag('ID',self.npID)
      if text:
         b3=self.geom.getTightBounds()
         bc=(b3[0]+b3[1])*.5
         dims=b3[1]-b3[0]
         self.text=OnscreenText(parent=self.geom,text=text,font=font,
           scale=textScale,fg=textColor)
         dz=self.text.getZ()-self.text.getBounds().getCenter()[2]+textPos[1]*dims[2]
         NodePath(self.text).setPos(self.geom.getParent(),bc[0],-1,bc[2]+dz*abs(self.geom.getSz()))
#          self.text.setAlphaScale(1,1)
         if self.text.find('**/+GeomNode').isEmpty(): # due to the new text generation method
            textTex=self.text.getFont().getPage(0)
         else:
            textTex=self.text.findTexture('*')
         self.text.setTexture(textTex, 10)
      self.hoverFunc=hoverFunc
      self.offFunc=offFunc
      if appearEnabled:
         self.setState(0)
         if enable:
            self.enable()
      else:
         self.disable()

  def setHitKeyActive(self,status):
      BTprefix=getBTprefix()
      if status:
         for k in self.hitKey:
             if self.stayAlive:
                self.accept(BTprefix+k,self.hitMeNow)
             else:
                self.acceptOnce(BTprefix+k,self.hitMeNow)
      else:
         for k in self.hitKey:
             self.ignore(BTprefix+k)

  def hitMeNow(self):
      global BUTTON_IN
      if taskMgr.hasTaskNamed(delayedCommandExecTaskName):
         return
      if BUTTON_IN!=self and BUTTON_IN!=None:
         BUTTON_IN.offButton(BUTTON_IN)
      BUTTON_IN=self
      BUTTON_IN.buttonPressed()
      BUTTON_IN.buttonReleased()

  def setState(self,state):
      '''
      button states :
         0 : ready
         1 : hover
         2 : pressed
         3 : disabled
      '''
      if self.stateTextures[state] is not None:
         self.geom.setTexture(self.stateTextures[state],1)
      self.state=state

  def hookEvents(self):
      BTprefix=getBTprefix()
      self.accept(BTprefix+'buttonIn-'+self.geom.getName()+self.npID,
           self.hoverOnButton, [self])
      self.accept(BTprefix+'buttonOut-'+self.geom.getName()+self.npID,
           self.offButton, [self])

  def unhookEvents(self):
      self.geom.setCollideMask(MASK_off)
      self.ignoreAll()

  def enable(self):
      self.geom.setCollideMask(MASK_button)
      self.setHitKeyActive(1)
      self.hookEvents()
      self.setState(0)

  def disable(self):
      self.geom.setCollideMask(MASK_off)
      self.setHitKeyActive(0)
      self.setState(3)

  def removeButton(self):
      self.unhookEvents()
      self.geom.removeNode()




class myDialog(DirectObject):
  def __init__(self, root, parent=None, pos=(0,0,0), scale=None, keyboard=False, buttons=()):
      '''
      Sets the given root node as a dialog
           parent   : where to attach root node
           buttons  : a collection of myButton instances
           keyboard : use keyboard (left/right arrows, tab, enter, space) too for button selection
      '''
      global BUTTON_PRESSED, BUTTON_IN
      if parent is not None:
         root.reparentTo(parent)
      root.setPos(*pos)
      if scale is not None:
         if type(scale) not in listORtuple:
            root.setScale(scale)
         else:
            root.setScale(*scale)
      self.root=root
      self.inFocusButton=None
      self.defaultButton=buttons[0] if len(buttons)==1 else None
      self.buttonList=[]
      for b in buttons:
          if type(b) in listORtuple:
             if BUTTON_IN is not None:
                BUTTON_IN.offButton(BUTTON_IN)
             # set as in-focus button, but don't accept mouse press
             self.defaultButton=b[0]
             self.buttonList.append(b[0])
          else:
             self.buttonList.append(b)
          if not self.buttonList[-1].stayAlive:
             self.buttonList[-1].acceptOnceCommand=( Functor(self.disableDialogButtons),
                                                     self.ignoreAll )
      if keyboard:
         self.setDialogKeysActive()

  def setDialogKeysActive(self):
      BTprefix=getBTprefix()
      self.acceptEvents( (
                         BTprefix+'arrow_right',
                         BTprefix+'tab',
                         ),
                         self.nextButton)
      self.acceptEvents( (
                         BTprefix+'arrow_right-repeat',
                         BTprefix+'tab-repeat',
                         ),
                         self.nextButton,[.1]) # if repeat, give some delay
      self.acceptEvents( (
                         BTprefix+'arrow_left',
                         BTprefix+'shift-tab',
                         ),
                         self.prevButton)
      self.acceptEvents( (
                         BTprefix+'arrow_left-repeat',
                         BTprefix+'shift-tab-repeat',
                         ),
                         self.prevButton,[.1]) # if repeat, give some delay
      self.acceptEvents( (
                         BTprefix+'enter',
                         BTprefix+'space',
                         ),
                         self.hitButtonNow)

  def acceptEvents(self,events,method,args=None):
      if type(events)==types.StringType:
         events=(events,)
      for e in events:
          if args:
             self.accept(e,method,args)
          else:
             self.accept(e,method)

  def enableDialogButtons(self):
      for b in self.buttonList:
          if b==self.defaultButton:
             b.enable()
             BUTTON_IN=b
             BUTTON_IN.hoverOnButton(BUTTON_IN,0)
          else:
             b.enable()

  def disableDialogButtons(self):
      for b in self.buttonList:
          b.unhookEvents()
          b.setHitKeyActive(0)

  def cleanup(self):
      global BUTTON_PRESSED, BUTTON_IN
      for b in self.buttonList:
          b.removeButton()
      self.buttonList=None
      self.root.removeNode()
      self.ignoreAll()
      BUTTON_PRESSED=BUTTON_IN=None

  def nextButton(self,delay=0):
      if taskMgr.hasTaskNamed(collTaskNamePrefix+'gotoNextButtonInDialog'):
         return
      # respects the pressed button, get out if it's pressed
      if BUTTON_PRESSED is not None:
         return
      # if delayed, suspend it, otherwise let's proceed right away
      if delay!=0:
         taskMgr.doMethodLater(delay,self.nextButton,collTaskNamePrefix+'gotoNextButtonInDialog')
      btnIdx=self.buttonList.index(BUTTON_IN) if BUTTON_IN else -1
      nextBtns=filter(lambda b: b.state!=3, self.buttonList[btnIdx+1:])
      if nextBtns:
         # set the in-focus button back to ready state,
         # but don't remove it's mouse press event hook
         if BUTTON_IN:
            BUTTON_IN.offButton(BUTTON_IN,0)
         # set the next button in-focus,
         # but don't accept mouse press event,
         # let it handled by the collision
         nextBtns[0].hoverOnButton(nextBtns[0],0)

  def prevButton(self,delay=0):
      if taskMgr.hasTaskNamed(collTaskNamePrefix+'gotoPrevButtonInDialog'):
         return
      # respects the pressed button, get out if it's pressed
      if BUTTON_PRESSED is not None:
         return
      # if delayed, suspend it, otherwise let's proceed right away
      if delay!=0:
         taskMgr.doMethodLater(delay,self.prevButton,collTaskNamePrefix+'gotoPrevButtonInDialog')
      btnIdx=self.buttonList.index(BUTTON_IN) if BUTTON_IN else len(self.buttonList)
      prevBtns=filter(lambda b: b.state!=3, self.buttonList[:btnIdx])
      if prevBtns:
         # set the in-focus button back to ready state,
         # but don't remove it's mouse press event hook
         if BUTTON_IN:
            BUTTON_IN.offButton(BUTTON_IN,0)
         # set the previous button in-focus,
         # but don't accept mouse press event,
         # let it handled by the collision
         prevBtns[-1].hoverOnButton(prevBtns[-1],0)

  def hitButtonNow(self):
      if BUTTON_PRESSED is not None:
         return
      # press and release the in-focus button
      if BUTTON_IN is not None:
         BUTTON_IN.buttonPressed()
         BUTTON_IN.buttonReleased()

