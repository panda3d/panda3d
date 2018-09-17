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

class AnimPanel(AppShell):
    #################################################################
    # This class will generate a animation panel for an actor
    # which user assigned. Inside this panel, instead of using actorInterval
    # or just simply calling the play function in Actor, we create a task to
    # set animation frame by frame using setPose.
    #################################################################
    # Override class variables
    appname = 'Anim Panel'
    frameWidth  = 575
    frameHeight = 250
    usecommandarea = 0
    usestatusarea  = 0
    index = 0
    dragMode = False
    rateList= ['1/24.0', '0.1', '0.5', '1.0', '2.0', '5.0' , '10.0']


    def __init__(self, aNode =  None, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        self.id = 'AnimPanel '+ aNode.getName()
        self.appname = self.id
        optiondefs = (
            ('title',               self.appname,       None),
            ('actor',           aNode,               None),
            ('animList',        [],      None),
            )
        self.defineoptions(kw, optiondefs)

        self.frameHeight = 300
        self.id = 'AnimPanel '+ aNode.getName()
        self.nodeName = aNode.getName()
        # Initialize the superclass
        AppShell.__init__(self)

        # Execute option callbacks
        self.initialiseoptions(AnimPanel)

        self.currTime = 0.0 # Initialize the start time
        self.animName = None

        self.parent.resizable(False,False) ## Disable the ability to resize for this Window.

    def createInterface(self):
        # Handle to the toplevels interior
        interior = self.interior()
        menuBar = self.menuBar

        menuBar.addmenu('Anim', 'Anim Panel Operations')

        # Reset all actor controls
        menuBar.addmenuitem('File', 'command',
                            'Load Animation',
                            label = 'Load Animation',
                            command = self.loadAnimation)
        menuBar.addmenuitem('Anim', 'command',
                            'Set actor controls to t = 0.0',
                            label = 'Jump all to zero',
                            command = self.resetAllToZero)
        menuBar.addmenuitem('Anim', 'command',
                            'Set Actor controls to end time',
                            label = 'Jump all to end time',
                            command = self.resetAllToEnd)
        menuBar.addmenuitem('Anim', 'separator')
        menuBar.addmenuitem('Anim', 'command',
                            'Play Current Animation',
                            label = 'Play',
                            command = self.play)
        menuBar.addmenuitem('Anim', 'command',
                            'Stop Current Animation',
                            label = 'stop',
                            command = self.stop)

        # Create a frame to hold all the actor controls
        actorFrame = Frame(interior)
        name_label = Label(actorFrame, text= self.nodeName,font=('MSSansSerif', 16),
                           relief = SUNKEN, borderwidth=3)
        name_label.place(x=5,y=5,anchor=NW)
        Label(actorFrame, text= "Animation:", font=('MSSansSerif', 12)).place(x=140,y=5,anchor=NW)
        Label(actorFrame, text= "Play Rate:", font=('MSSansSerif', 12)).place(x=140,y=35,anchor=NW)
        self['animList'] = self['actor'].getAnimNames()
        self.AnimEntry = self.createcomponent(
            'AnimationMenu', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, selectioncommand = self.setAnimation,
            scrolledlist_items = self['animList'])
        self.AnimEntry.place(x=240,y=10,anchor=NW)


        self.playRateEntry = self.createcomponent(
            'playRateMenu', (), None,
            Pmw.ComboBox, (actorFrame,),
            labelpos = W, entry_width = 20, selectioncommand = self.setPlayRate,
            scrolledlist_items = self.rateList)
        self.playRateEntry.place(x=240,y=40,anchor=NW)
        self.playRateEntry.selectitem('1.0')

        ### Loop checkbox
        Label(actorFrame, text= "Loop:", font=('MSSansSerif', 12)).place(x=420,y=5,anchor=NW)

        self.loopVar = IntVar()
        self.loopVar.set(0)
        self.loopButton = self.createcomponent(
            'loopButton', (), None,
            Checkbutton, (actorFrame,),
            variable = self.loopVar)
        self.loopButton.place(x=470,y=7,anchor=NW)

        ### Display Frames/Seconds
        Label(actorFrame, text= "Frame/Second:", font=('MSSansSerif', 11)).place(x=5,y=75,anchor=NW)

        self.unitsVar = IntVar()
        self.unitsVar.set(FRAMES)
        self.displayButton = self.createcomponent(
            'displayButton', (), None,
            Checkbutton, (actorFrame,),
            command = self.updateDisplay,
            variable = self.unitsVar)
        self.displayButton.place(x=120,y=77,anchor=NW)

        ## scale control
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

        ## button contorl
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

        ButtomFrame.place(anchor=NW,x=5,y=165)

        self.removeButton = self.createcomponent(
            'Remove Animation', (), None,
            Button, (actorFrame,),
            text = 'Remove This Animation', width = 20,
            command = self.removeAnim)
        self.removeButton.place(anchor=NW,x=5,y=220)

        self.loadButton = self.createcomponent(
            'Load Animation', (), None,
            Button, (actorFrame,),
            text = 'Load New Animation', width = 20,
            command = self.loadAnimation)
        self.loadButton.place(anchor=NW,x=180,y=220)

        # Now pack the actor frame
        actorFrame.pack(expand = 1, fill = BOTH)

    def updateList(self):
        #################################################################
        # updateList(self)
        # This function will update the list of animations that the Actor
        # currently has into the combo box widget.
        #################################################################
        self.ignore('DataH_loadFinish'+self.nodeName)
        del self.loaderWindow
        self['animList'] = self['actor'].getAnimNames()
        animL = self['actor'].getAnimNames()
        self.AnimEntry.setlist(animL)


    def removeAnim(self):
        #################################################################
        # removeAnim(self)
        # This function will stop the animation and get the name of animation
        # which user wish to remove from the panel. Then, it will send out
        # a message to dataHolder to remove the target animation.
        # And in the same time, it will start waiting a return message to
        # make sure that target animation has been removed.
        #################################################################
        name = self.AnimEntry.get()
        if taskMgr.hasTaskNamed(self.id + '_UpdateTask'):
            self.stop()
        self.accept('DataH_removeAnimFinish'+self.nodeName,self.afterRemove)
        messenger.send('AW_removeAnim',[self['actor'],name])
        return

    def afterRemove(self):
        #################################################################
        # afterRemove(self)
        # This function will be called once panel has received the return
        # message from dataHolder. This function will call setList to
        # reset the list of Animations
        #################################################################
        self.ignore('DataH_removeAnimFinish'+self.nodeName)
        self['animList'] = self['actor'].getAnimNames()
        animL = self['actor'].getAnimNames()
        self.AnimEntry.setlist(animL)
        print('-----',animL)
        return

    def loadAnimation(self):
        #################################################################
        # loadAnimation(self)
        # This function will open a dialog window to require user to input
        # the animation he wants to load in for this actor.
        #################################################################
        self.loaderWindow = LoadAnimPanel(aNode=self['actor'])
        self.accept('DataH_loadFinish'+self.nodeName,self.updateList)
        return

    def play(self):
        #################################################################
        # play(self)
        # This function will be called when user click on the "play" button.
        # First, this function will initialize all parameter that the actual
        # play task need to run and add the play task into the taskMgr.
        #################################################################
        self.animName = self.AnimEntry.get()
        if self.animName in self['animList']:
            animName = self.AnimEntry.get()
            self.playButton.config(state=DISABLED)
            self.lastT = globalClock.getFrameTime()
            taskMgr.add(self.playTask, self.id + '_UpdateTask')
            self.stopButton.config(state=NORMAL)
        else:
            print('----Illegal Animaion name!!', self.animName)
        return

    def playTask(self, task):
        #################################################################
        # playTask(self, task)
        # This task will record time by each frame
        # In fact it is just a clock keeper.
        # If the current frame time over the max long of the animation,
        # it will reset the timer.
        # Anyway, this function will call gotoT by each frame.
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
        # This function will remove the play task from taskMgr when user
        # click on the "Stop" button
        #################################################################
        taskMgr.remove(self.id + '_UpdateTask')
        self.playButton.config(state=NORMAL)
        self.stopButton.config(state=DISABLED)
        return

    def setAnimation(self, animation):
        #################################################################
        # setAnimation(self, animation)
        # This function will be called each time when user change
        # the current animation. Most important thing this function do is
        # to recalculate all variables to fit the selected animation
        #################################################################
        self.animName = self.AnimEntry.get()
        playRate = '%0.1f' % self['actor'].getPlayRate(self.animName)
        if playRate not in self.rateList:
            def strCmp(a, b):
                return cmp(eval(a), eval(b))
            self.rateList.append(playRate)
            self.rateList.sort(strCmp)
            self.playRateEntry.reset(self.rateList)
            self.playRateEntry.selectitem(playRate)
        self.currTime = 0.0
        self.frameControl.set(0)
        self.updateDisplay()
        return

    def setPlayRate(self,rate):
        #################################################################
        # setPlayRate(self, rate)
        # This function will be called each time when user changes the current play rate.
        #################################################################
        self.animName = self.AnimEntry.get()
        if self.animName in self['animList']:
            self['actor'].setPlayRate(eval(rate), self.animName)
            self.updateDisplay()
        return

    def updateDisplay(self):
        #################################################################
        # updateDisplay(self)
        # This function will be called whenever something has been changed
        # on the panel. In here we will re-new all widgets on the panel to
        # correct value.
        #################################################################
        self.fps = self['actor'].getFrameRate(self.animName)
        self.duration = self['actor'].getDuration(self.animName)
        self.maxFrame = self['actor'].getNumFrames(self.animName) - 1
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
        # gotoT(self, time)
        # calculate the right parameter which will be send to set Frame
        # Control slider, which is the real place we play the animation.
        #################################################################
        if self.unitsVar.get() == FRAMES:
            self.frameControl.set(time * self.fps)
        else:
            self.frameControl.set(time)
        return

    def goTo(self,frame):
        #################################################################
        # goto(self, frame)
        # Call back function for the frame control slider.
        # This function will set the animation by the value on the slider.
        #
        # This function is the real function we "play" the animation.
        #
        #################################################################
        if self.animName in self['animList']:
            # Convert scale value to float
            frame = string.atof(frame)
            # Now convert t to seconds for offset calculations
            if self.unitsVar.get() == FRAMES:
                frame = frame / self.fps
            if self.dragMode:
                # If user is clicking on the slider and is draging the bar, reset the global timer.
                self.currTime = frame
            self['actor'].pose(self.animName,
                            min(self.maxFrame, int(frame * self.fps)))
        return

    def onRelease(self,frame):
        #################################################################
        # onRelease(self, frame)
        # disable the dragMode when user releases the bar on the slider.
        #################################################################
        self.dragMode = False
        return

    def onPress(self,frame):
        #################################################################
        # onPress(self, frame)
        # enable the dragMode when user press the bar on the slider.
        #################################################################
        self.dragMode = True
        return

    def resetAllToZero(self):
        #################################################################
        # resetAllToZero(self)
        # reset the global timer to zero and also move the slider to zero.
        # This will also reset the actor to the zero frame of current animation
        #################################################################
        self.currTime = 0.0
        self.gotoT(0)
        return

    def resetAllToEnd(self):
        #################################################################
        # resetAllToEnd(self)
        # set the global timer to the end of current animation and
        # also move the slider to the end.
        #################################################################
        self.currTime = self.maxSeconds
        self.gotoT(self.duration)
        return

    def onDestroy(self, event):
        if taskMgr.hasTaskNamed(self.id + '_UpdateTask'):
            taskMgr.remove(self.id + '_UpdateTask')
        self.ignore('DataH_loadFinish')
        messenger.send('AW_close',[self.nodeName])
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

class LoadAnimPanel(AppShell):
    #################################################################
    # LoadAnimPanel(AppShell)
    # This class will open a dialog to ask user to input names and
    # file paths of animations
    #################################################################
    # Override class variables
    appname = 'Load Animation'
    frameWidth  = 575
    frameHeight = 200
    usecommandarea = 0
    usestatusarea  = 0
    index = 0
    ## Anim name : File Path

    def __init__(self, aNode =  None, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        self.id = 'Load Animation '+ aNode.getName()
        self.appname = self.id
        self.animDic = {}
        self.animList = []
        optiondefs = (
            ('title',               self.appname,       None),
            )
        self.defineoptions(kw, optiondefs)

        self.frameHeight = 300
        self.nodeName = aNode.getName()
        self.Actor = aNode
        # Initialize the superclass
        AppShell.__init__(self)

        # Execute option callbacks
        self.initialiseoptions(LoadAnimPanel)

    def createInterface(self):
        self.menuBar.destroy()
        interior = self.interior()
        mainFrame = Frame(interior)
        self.inputZone = Pmw.Group(mainFrame, tag_text='File Setting')
        self.inputZone.pack(fill='both',expand=1)
        settingFrame = self.inputZone.interior()
        Label(settingFrame,text='Anim Name').place(anchor=NW,x=60,y=5)
        Label(settingFrame,text='File Path').place(anchor=NW,x=205,y=5)
        self.AnimName_1 = self.createcomponent(
            'Anim Name List', (), None,
            Pmw.ComboBox, (settingFrame,),label_text='Anim   :',
            labelpos = W, entry_width = 10, selectioncommand = self.selectAnim,
            scrolledlist_items = self.animList)
        self.AnimFile_1 = Pmw.EntryField(settingFrame,value='')
        self.AnimFile_1.component('entry').config(width=20)
        self.AnimName_1.place(anchor=NW,x=10,y=25)
        self.AnimFile_1.place(anchor=NW,x=140,y=25)
        self.Browse_1 = self.createcomponent(
            'File Browser1', (), None,
            Button, (mainFrame,),
            text = 'Browse...',
            command = self.Browse_1)
        self.Browse_1.place(anchor=NW,x=270,y=38)

        self.addIntoButton = self.createcomponent(
            'Load Add', (), None,
            Button, (mainFrame,),
            text = 'Add to Load',
            command = self.addIntoList)
        self.addIntoButton.place(anchor=NW,x=345,y=38)

        att_label = Label(mainFrame, font=('MSSansSerif', 10),
                          text= "Attention! Animations won't be loaded in before you press the 'OK' button below!")
        att_label.place(anchor=NW,x=10,y=80)

        self.button_ok = Button(mainFrame, text="OK", command=self.ok_press,width=10)
        self.button_ok.pack(fill=BOTH,expand=0,side=RIGHT)

        mainFrame.pack(expand = 1, fill = BOTH)



    def onDestroy(self, event):
        messenger.send('AWL_close',[self.nodeName])
        '''
        If you have open any thing, please rewrite here!
        '''
        pass

    def selectAnim(self,name):
        #################################################################
        # selectAnim(self, name)
        # This function will be called if user select an animation on the list.
        #################################################################
        if name in self.animDic:
            self.AnimFile_1.setvalue = self.animDic[name]
        return

    def Browse_1(self):
        #################################################################
        # Browse_1(self)
        # when the browse button pused, this function will be called.
        # Do nothing but open a file dialog for user to set the path to target file
        # Then, set the path back to the entry on the panel.
        #################################################################
        AnimFilename = askopenfilename(
            defaultextension = '.egg',
            filetypes = (('Egg Files', '*.egg'),
                         ('Bam Files', '*.bam'),
                         ('All files', '*')),
            initialdir = '.',
            title = 'File Path for Anim 1',
            parent = self.parent)
        if AnimFilename:
            self.AnimFile_1.setvalue(AnimFilename)
        return

    def addIntoList(self):
        #################################################################
        # addIntoList(self)
        # This function will be called each time when user click on the
        # "Add to Load" button. This function will read the current data
        # on the panel into a dictionary. then reset the list of the animation
        # name list on this panel.(not the one in the animation panel...)
        #
        # This function won't load any animation....
        #
        #################################################################
        name = self.AnimName_1.get()
        self.animDic[name] = Filename.fromOsSpecific(self.AnimFile_1.getvalue()).getFullpath()
        if name in self.animList:
            pass
        else:
            self.animList.append(name)
        self.AnimName_1.setlist(self.animList)
        print(self.animDic)
        return

    def ok_press(self):
        #################################################################
        # ok_press(Self)
        # This functiion will be called when user click on the "OK"
        # button. This function will send a message along with the animation
        # file we wish to load for this actor.
        # Then, it will close the panel itself.
        #################################################################
        messenger.send('AW_AnimationLoad',[self.Actor,self.animDic])
        #print self.animDic
        self.quit()
        return
