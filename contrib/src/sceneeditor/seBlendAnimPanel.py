#################################################################
# collisionWindow.py
# Written by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#################################################################
# Import Tkinter, Pmw, and the floater code from this directory tree.
from direct.tkwidgets.AppShell import *
from direct.showbase.TkGlobal import *
import string
import math
import types
from direct.task import Task

if sys.version_info >= (3, 0):
    from tkinter.simpledialog import askfloat
else:
    from tkSimpleDialog import askfloat

FRAMES = 0
SECONDS = 1


#####################################################################################
# BlendAnimPanel(AppShell)
# This Panel will allow user to blend tow animations
# that have already been loaded for this actor.
# user can play and manipulate this blended animation
# just like in the animation panel. And, they can save this blended animation.
#####################################################################################
class BlendAnimPanel(AppShell):
    # Override class variables
    appname = 'Blend Anim Panel'
    frameWidth  = 575
    frameHeight = 450
    usecommandarea = 0
    usestatusarea  = 0
    index = 0
    dragMode = False
    blendRatio = 0
    rateList= ['1/24.0', '0.1', '0.5', '1.0', '2.0', '5.0' , '10.0']
    enableBlend = False
    currentBlendName = None


    def __init__(self, aNode =  None, blendDict={}, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        self.id = 'BlendAnimPanel '+ aNode.getName()
        self.appname = self.id
        self.actorNode = aNode
        self.blendDict = blendDict.copy()
        if len(blendDict)>0:
            self.blendList = blendDict.keys()
        else:
            self.blendList = []
        optiondefs = (
            ('title',               self.appname,       None),
            ('actor',               aNode,              None),
            ('animList',            [],                 None),
            ('blendAnimList',       self.blendList,          None),
            )
        self.defineoptions(kw, optiondefs)

        self.id = 'Blend AnimPanel '+ aNode.getName()
        self.nodeName = aNode.getName()
        # Initialize the superclass
        AppShell.__init__(self)

        # Execute option callbacks
        self.initialiseoptions(BlendAnimPanel)

        self.currTime = 0.0
        self.animNameA = None
        self.animNameB = None

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def createInterface(self):
        # Handle to the toplevels interior
        interior = self.interior()
        self.menuBar.destroy()

        # show the actor's name
        actorFrame = Frame(interior)
        name_label = Label(actorFrame, text= self.nodeName,font=('MSSansSerif', 14),
                           relief = SUNKEN, borderwidth=3)
        name_label.pack(side = TOP, expand = False)
        actorFrame.pack(side = TOP, expand = False, fill = X)

        # Create a frame to show is there any ore-blended animation and save, edit, rename button.
        group = Pmw.Group(interior, tag_pyclass=None)
        actorFrame = group.interior()
        group.pack(side = TOP, expand = False, fill = X)

        Label(actorFrame, text= "Blended:", font=('MSSansSerif', 10)).pack(side=LEFT)
        self.blendAnimEntry = self.createcomponent(
            'Blended Animation', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, selectioncommand = self.setBlendAnim,
            scrolledlist_items = self['blendAnimList'])
        self.blendAnimEntry.pack(side=LEFT)

        Label(actorFrame, text= "   ", font=('MSSansSerif', 10)).pack(side=LEFT)

        button = Button(actorFrame, text="Save", font=('MSSansSerif', 10),width = 12,
                        command = self.saveButtonPushed).pack(side=LEFT)
        button = Button(actorFrame, text="Remove", font=('MSSansSerif', 10),width = 12,
                        command = self.removeButtonPushed).pack(side=LEFT)
        button = Button(actorFrame, text="Rename", font=('MSSansSerif', 10),width = 12,
                        command = self.renameButtonPushed).pack(side=LEFT)

        actorFrame.pack(side = TOP, expand = False, fill = X)

        # Create a frame to hold all the animation setting
        group = Pmw.Group(interior, tag_pyclass=None)
        actorFrame = group.interior()
        group.pack(side = TOP, expand = False, fill = X)
        Label(actorFrame, text= "Animation A:", font=('MSSansSerif', 10)).pack(side=LEFT)
        self['animList'] = self['actor'].getAnimNames()
        self.AnimEntryA = self.createcomponent(
            'AnimationMenuA', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, entry_state = DISABLED,
            selectioncommand = lambda name, a = 'a' : self.setAnimation(name, AB=a),
            scrolledlist_items = self['animList'])
        self.AnimEntryA.pack(side=LEFT)

        Label(actorFrame, text= "   ", font=('MSSansSerif', 10)).pack(side=LEFT,)
        Label(actorFrame, text= "Animation B:", font=('MSSansSerif', 10)).pack(side=LEFT)
        self['animList'] = self['actor'].getAnimNames()
        self.AnimEntryB = self.createcomponent(
            'AnimationMenuB', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, entry_state = DISABLED,
            selectioncommand = lambda name, a = 'b' : self.setAnimation(name, AB=a),
            scrolledlist_items = self['animList'])
        self.AnimEntryB.pack(side=LEFT)
        actorFrame.pack(side = TOP, expand = False, fill = X)

        ### Blend Enable checkbox
        actorFrame = Frame(interior, relief = SUNKEN, bd = 1)
        Label(actorFrame, text= "Enable Blending:", font=('MSSansSerif', 10)).pack(side=LEFT,)
        self.blendVar = IntVar()
        self.blendVar.set(0)
        self.blendButton = self.createcomponent(
            'blendButton', (), None,
            Checkbutton, (actorFrame,),
            variable = self.blendVar,
            command = self.toggleBlend)
        self.blendButton.pack(side=LEFT)
        actorFrame.pack(side = TOP, expand = False, fill = X)

        ## Ratio control
        actorFrame = Frame(interior)
        frameFrame = Frame(actorFrame, relief = SUNKEN, bd = 1)
        minRatioLabel = self.createcomponent(
            'minRatioLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 0.00)
        minRatioLabel.pack(side = LEFT)

        self.ratioControl = self.createcomponent(
            'ratio', (), None,
            Scale, (frameFrame,),
            from_ = 0.0, to = 1.0, resolution = 0.01,
            command = self.setRatio, length = 500,
            orient = HORIZONTAL, showvalue = 1)
        self.ratioControl.pack(side = LEFT, expand = 1)
        self.ratioControl.set(1.0)

        self.maxRatioLabel = self.createcomponent(
            'maxRatioLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 1.00)
        self.maxRatioLabel.pack(side = LEFT)
        frameFrame.pack(side = LEFT, expand = 1, fill = X)
        actorFrame.pack(side = TOP, expand = True, fill = X)

        ###################################################################################
        ###################################################################################
        actorFrame = Frame(interior)
        Label(actorFrame, text= "Play Rate:", font=('MSSansSerif', 10)).pack(side=LEFT)
        self.playRateEntry = self.createcomponent(
            'playRateMenu', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, selectioncommand = self.setPlayRate,
            scrolledlist_items = self.rateList)
        self.playRateEntry.pack(side=LEFT)
        self.playRateEntry.selectitem('1.0')

        ### Loop checkbox
        Label(actorFrame, text= "   ", font=('MSSansSerif', 10)).pack(side=LEFT,)
        Label(actorFrame, text= "Loop:", font=('MSSansSerif', 10)).pack(side=LEFT,)

        self.loopVar = IntVar()
        self.loopVar.set(0)
        self.loopButton = self.createcomponent(
            'loopButton', (), None,
            Checkbutton, (actorFrame,),
            variable = self.loopVar)
        self.loopButton.pack(side=LEFT)

        actorFrame.pack(side = TOP, expand = True, fill = X)



        ### Display Frames/Seconds
        actorFrame = Frame(interior)

        Label(actorFrame, text= "Frame/Second:", font=('MSSansSerif', 10)).pack(side=LEFT)

        self.unitsVar = IntVar()
        self.unitsVar.set(FRAMES)
        self.displayButton = self.createcomponent(
            'displayButton', (), None,
            Checkbutton, (actorFrame,),
            command = self.updateDisplay,
            variable = self.unitsVar)
        self.displayButton.pack(side=LEFT)

        actorFrame.pack(side = TOP, expand = True, fill = X)

        ## scale control
        actorFrame = Frame(interior)
        frameFrame = Frame(actorFrame, relief = SUNKEN, bd = 1)
        self.minLabel = self.createcomponent(
            'minLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 0)
        self.minLabel.pack(side = LEFT)

        self.frameControl = self.createcomponent(
            'scale', (), None,
            Scale, (frameFrame,),
            from_ = 0, to = 24, resolution = 1.0,
            command = self.goTo, length = 500,
            orient = HORIZONTAL, showvalue = 1)
        self.frameControl.pack(side = LEFT, expand = 1)
        self.frameControl.bind('<Button-1>', self.onPress)
        self.frameControl.bind('<ButtonRelease-1>', self.onRelease)

        self.maxLabel = self.createcomponent(
            'maxLabel', (), 'sLabel',
            Label, (frameFrame,),
            text = 24)
        self.maxLabel.pack(side = LEFT)
        frameFrame.pack(side = LEFT, expand = 1, fill = X)
        actorFrame.pack(side = TOP, expand = True, fill = X)

        ## button contorl
        actorFrame = Frame(interior)
        ButtomFrame = Frame(actorFrame, relief = SUNKEN, bd = 1,borderwidth=5)
        self.toStartButton = self.createcomponent(
            'toStart', (), None,
            Button, (ButtomFrame,),
            text = '<<',
            width = 8,
            command = self.resetAllToZero)
        self.toStartButton.pack(side = LEFT, expand = 1, fill = X)

        self.playButton = self.createcomponent(
            'playButton', (), None,
            Button, (ButtomFrame,),
            text = 'Play', width = 8,
            command = self.play)
        self.playButton.pack(side = LEFT, expand = 1, fill = X)

        self.stopButton = self.createcomponent(
            'stopButton', (), None,
            Button, (ButtomFrame,),
            text = 'Stop', width = 8, state=DISABLED,
            command = self.stop)
        self.stopButton.pack(side = LEFT, expand = 1, fill = X)

        self.toEndButton = self.createcomponent(
            'toEnd', (), None,
            Button, (ButtomFrame,),
            text = '>>',
            width = 8,
            command = self.resetAllToEnd)
        self.toEndButton.pack(side = LEFT, expand = 1, fill = X)

        ButtomFrame.pack(side = TOP, expand = True, fill = X)
        actorFrame.pack(expand = 1, fill = BOTH)

    def updateList(self):
        #################################################################
        # updateList(self)
        # This will reset the list of all animations that this actor has
        # to the animation entry A and B.
        #################################################################
        self['animList'] = self['actor'].getAnimNames()
        animL = self['actor'].getAnimNames()
        self.AnimEntryA.setlist(animL)
        self.AnimEntryB.setlist(animL)

    def play(self):
        #################################################################
        # play(self)
        # It works pretty much like what we have in the Animation Panel.
        # The only different now is that we set two "pose" here.
        # When you do the blending animation by setPose, you don't have
        # to set them simultaneously.
        #################################################################
        self.animNameA = self.AnimEntryA.get()
        self.animNameB = self.AnimEntryB.get()
        if (self.animNameA in self['animList'])and(self.animNameB in self['animList']):
            self.playButton.config(state=DISABLED)
            self.lastT = globalClock.getFrameTime()
            taskMgr.add(self.playTask, self.id + '_UpdateTask')
            self.stopButton.config(state=NORMAL)
        else:
            print('----Illegal Animaion name!!', self.animNameA +  ',  '+ self.animNameB)
        return

    def playTask(self, task):
        #################################################################
        # playTask(self, task)
        # see play(self)
        #################################################################
        fLoop = self.loopVar.get()
        currT = globalClock.getFrameTime()
        deltaT = currT - self.lastT
        self.lastT = currT
        if self.dragMode:
            return Task.cont
        self.currTime = self.currTime + deltaT
        if (self.currTime > self.maxSeconds):
            if fLoop:
                self.currTime = self.currTime%self.duration
                self.gotoT(self.currTime)
            else:
                self.currTime = 0.0
                self.gotoT(0.0)
                self.playButton.config(state=NORMAL)
                self.stopButton.config(state=DISABLED)
                return Task.done
        else:
            self.gotoT(self.currTime)
        return Task.cont

    def stop(self):
        #################################################################
        # stop(self)
        # see play(self)
        #################################################################
        taskMgr.remove(self.id + '_UpdateTask')
        self.playButton.config(state=NORMAL)
        self.stopButton.config(state=DISABLED)
        return

    def setAnimation(self, animation, AB = 'a'):
        #################################################################
        # setAnimation(self, animation, AB = 'a')
        # see play(self)
        #################################################################
        print('OK!!!')
        if AB == 'a':
            if self.animNameA != None:
                self['actor'].setControlEffect(self.animNameA, 1.0, 'modelRoot','lodRoot')
            self.animNameA = self.AnimEntryA.get()
        else:
            if self.animNameB != None:
                self['actor'].setControlEffect(self.animNameB, 1.0, 'modelRoot','lodRoot')
            self.animNameB = self.AnimEntryB.get()
        self.currTime = 0.0
        self.frameControl.set(0)
        self.updateDisplay()
        self.setRatio(self.blendRatio)
        return

    def setPlayRate(self,rate):
        #################################################################
        # setPlayRate(self,rate)
        # see play(self)
        #################################################################
        self.animNameA = self.AnimEntryA.get()
        if self.animNameA in self['animList']:
            self['actor'].setPlayRate(eval(rate), self.animNameA)
            self.updateDisplay()
        if self.animNameB in self['animList']:
            self['actor'].setPlayRate(eval(rate), self.animNameB)
            self.updateDisplay()
        return

    def updateDisplay(self):
        #################################################################
        # updateDisplay(self)
        # see play(self)
        #################################################################
        if not (self.animNameA in self['animList']):
            return
        self.fps = self['actor'].getFrameRate(self.animNameA)
        self.duration = self['actor'].getDuration(self.animNameA)
        self.maxFrame = self['actor'].getNumFrames(self.animNameA) - 1
        if not (self.animNameB in self['animList']):
            return
        if self.duration > self['actor'].getDuration(self.animNameB):
            self.duration = self['actor'].getDuration(self.animNameB)
        if self.maxFrame > self['actor'].getNumFrames(self.animNameB) - 1:
            self.maxFrame = self['actor'].getNumFrames(self.animNameB) - 1
        self.maxSeconds = self.duration
        if self.unitsVar.get() == FRAMES:
            fromFrame = 0
            toFrame = self.maxFrame
            self.minLabel['text'] = fromFrame
            self.maxLabel['text'] = toFrame
            self.frameControl.configure(from_ = fromFrame,
                                        to = toFrame,
                                        resolution = 1.0)
        else:
            self.minLabel['text'] = '0.0'
            self.maxLabel['text'] = "%.2f" % self.duration
            self.frameControl.configure(from_ = 0.0,
                                        to = self.duration,
                                        resolution = 0.01)

    def gotoT(self,time):
        #################################################################
        # gotoT(self,time)
        # see play(self)
        #################################################################
        if self.unitsVar.get() == FRAMES:
            self.frameControl.set(time * self.fps)
        else:
            self.frameControl.set(time)
        return

    def goTo(self,frame):
        #################################################################
        # goTo(self,frame)
        # see play(self)
        #################################################################
        if (self.animNameA in self['animList'])and(self.animNameB in self['animList']):
            # Convert scale value to float
            frame = string.atof(frame)
            # Now convert t to seconds for offset calculations
            if self.unitsVar.get() == FRAMES:
                frame = frame / self.fps
            if self.dragMode:
                self.currTime = frame
            self['actor'].pose(self.animNameA,
                            min(self.maxFrame, int(frame * self.fps)))
            self['actor'].pose(self.animNameB,
                            min(self.maxFrame, int(frame * self.fps)))
        return

    def onRelease(self,frame):
        #################################################################
        # onRelease(self,frame)
        # see play(self)
        #################################################################
        self.dragMode = False
        return

    def onPress(self,frame):
        #################################################################
        # onPress(self,frame)
        # see play(self)
        #################################################################
        self.dragMode = True
        return

    def resetAllToZero(self):
        #################################################################
        # resetAllToZero(self)
        # see play(self)
        #################################################################
        self.currTime = 0.0
        self.gotoT(0)
        return

    def resetAllToEnd(self):
        #################################################################
        # resetAllToEnd(self)
        # see play(self)
        #################################################################
        self.currTime = self.maxSeconds
        self.gotoT(self.duration)
        return

    def toggleBlend(self):
        #################################################################
        # toggleBlend(self)
        # This function will enable the blending option for the actor.
        # and call set ratio function to set the blending animation mixing in
        # current ratio.
        #
        # This blending enable will not be keep when you close the window!
        #
        #################################################################
        if self.blendVar.get():
            self.enableBlend = True
            self['actor'].enableBlend()
            self.setRatio(self.blendRatio)
        else:
            self.enableBlend = False
            self['actor'].disableBlend()
        return

    def setRatio(self, ratio):
        #################################################################
        # setRatio(self, ratio)
        # callback funtion
        # This one will be called each time when user drag the blend ratio
        # slider on the panel. This will set the blening ratio to both animation.
        # (Which is "setControlEffect")
        #################################################################
        self.blendRatio = float(ratio)
        if self.enableBlend:
            if self.animNameA in self['animList']:
                self['actor'].setControlEffect(self.animNameA, self.blendRatio, 'modelRoot','lodRoot')
            if self.animNameB in self['animList']:
                self['actor'].setControlEffect(self.animNameB, 1-self.blendRatio, 'modelRoot','lodRoot')
            return

    def setBlendAnim(self, name):
        #################################################################
        # setBlendAnim(self, name)
        # This function will be called each time when user try to select
        # a existing blending animation from the comboBox on the panel
        # This function will re-set every varaibles on the panel to what
        # it should be. For example, when user choose blending anim "R,"
        # which was blended by anim "a" and "b" with ratio "c,"
        # then this function will set Animation A to "a" and animation B
        # to "b" and set the ratio slider to "c" position.
        #################################################################
        if name in self.blendDict:
            self.currentBlendName = name
            animA = self.blendDict[name][0]
            animB = self.blendDict[name][1]
            ratio = self.blendDict[name][2]
            self.AnimEntryA.selectitem(animA)
            self.AnimEntryB.selectitem(animB)
            self.setAnimation(animA, AB = 'a')
            self.setAnimation(animB, AB = 'b')
            self.ratioControl.set(ratio)
        return

    def setBlendAnimList(self, dict, select=False):
        #################################################################
        # setBlendAnimList(self, dict, select=False)
        # This function will be called when we need to reset the dropdown list
        # of "Blend Anim."
        # About "selec" option, this now is mainly used when we remove
        # a blended animation from the actor. When it has been specified to True,
        # the function will not only reset the list, but will also automatically
        # select one from the top of list, if it is not empty.
        #################################################################
        self.blendDict.clear()
        del self.blendDict
        self.blendDict = dict.copy()
        print(self.blendDict)
        if len(self.blendDict)>0:
            self.blendList = self.blendDict.keys()
        else:
            self.blendList = []
        self.blendAnimEntry.setlist(self.blendList)
        if select:
            if len(self.blendList)>0:
                self.blendAnimEntry.selectitem(self.blendList[0])
                self.setBlendAnim(self.blendList[0])
                self.currentBlendName = self.blendList[0]
            else:
                self.blendAnimEntry.clear()
                self.currentBlendName = None
        return

    def saveButtonPushed(self):
        #################################################################
        # saveButtonPushed(self)
        # This function will be called when user clicked on the "Save" button
        # This functiont will collect all data on the panel and send them with
        # a message to sceneEditor to save the current blending animation
        # into the dataHolder.
        #################################################################
        name = self.blendAnimEntry.get()
        if name=='':
            Pmw.MessageDialog(None, title='Caution!',
                              message_text = 'You have to give the blending animation a name first!',
                              iconpos='s',
                              defaultbutton = 'Close'
                              )
            return
        elif (not(self.animNameA in self['animList']))or(not(self.animNameB in self['animList'])):
            Pmw.MessageDialog(None, title='Caution!',
                              message_text = 'The Animations you have selected are not exist!',
                              iconpos='s',
                              defaultbutton = 'Close'
                              )
            return
        else:
            messenger.send('BAW_saveBlendAnim', [self['actor'].getName(),
                                                 name,
                                                 self.animNameA,
                                                 self.animNameB,
                                                 self.blendRatio])
            self.currentBlendName = name
        return

    def removeButtonPushed(self):
        #################################################################
        # removeButtonPushed(self)
        # remove the current seleted blended animation from the actor.
        # This will send out a message to sceneEditor to delete the data inside
        # the dataHolder and then reset the list of here.
        #################################################################
        name = self.blendAnimEntry.get()
        messenger.send('BAW_removeBlendAnim', [self['actor'].getName(),name])
        return

    def renameButtonPushed(self):
        #################################################################
        # renameButtonPushed(self)
        # this function will be called when user click on the "Rename" button.
        # This function will collect all data on the panel and send them out
        # with a message to sceneEditor to rename and re-save all setting about
        # current animation.
        #################################################################
        oName = self.currentBlendName
        name = self.blendAnimEntry.get()
        if self.currentBlendName == None:
            Pmw.MessageDialog(None, title='Caution!',
                              message_text = "You haven't select any blended animation!!",
                              iconpos='s',
                              defaultbutton = 'Close'
                              )
            return
        elif name=='':
            Pmw.MessageDialog(None, title='Caution!',
                              message_text = 'You have to give the blending animation a name first!',
                              iconpos='s',
                              defaultbutton = 'Close'
                              )
            return
        elif (not(self.animNameA in self['animList']))or(not(self.animNameB in self['animList'])):
            Pmw.MessageDialog(None, title='Caution!',
                              message_text = 'The Animations you have selected are not exist!',
                              iconpos='s',
                              defaultbutton = 'Close'
                              )
            return
        else:
            messenger.send('BAW_renameBlendAnim', [self['actor'].getName(),
                                                   name,
                                                   oName,
                                                   self.animNameA,
                                                   self.animNameB,
                                                   self.blendRatio]
                           )
            self.currentBlendName = name
        return

    def onDestroy(self, event):
        #################################################################
        # onDestroy(self, event)
        # This function will be call when user try to close the window.
        # In here we will stop all tasks we have opend and disable the
        # blend setting of actor.
        # If we didn't disable the blend option, the next time you play
        # the animation via animation panel will cause some error.
        #################################################################
        if taskMgr.hasTaskNamed(self.id + '_UpdateTask'):
            taskMgr.remove(self.id + '_UpdateTask')
        messenger.send('BAW_close',[self.nodeName])
        self.actorNode.setControlEffect(self.animNameA, 1.0, 'modelRoot','lodRoot')
        self.actorNode.setControlEffect(self.animNameB, 1.0, 'modelRoot','lodRoot')
        self.actorNode.disableBlend()
        '''
        If you have open any thing, please rewrite here!
        '''
        pass
