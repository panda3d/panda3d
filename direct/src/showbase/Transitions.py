
from pandac.PandaModules import *
from direct.gui.DirectGui import *
from direct.task import Task
from direct.interval.IntervalGlobal import *

class Transitions:

    # These may be reassigned before the fade or iris transitions are
    # actually invoked to change the models that will be used.
    IrisModelName = "models/misc/iris"
    FadeModelName = "models/misc/fade"

    def __init__(self, loader):
        self.ival = None
        self.iris = None
        self.fade = None
        self.letterbox = None
        self.alphaOff = Vec4(0,0,0,0)
        self.alphaOn = Vec4(0,0,0,1)

        self.irisTaskName = "irisTask"
        self.fadeTaskName = "fadeTask"
        self.letterboxTaskName = "letterboxTask"

    ##################################################
    # Fade
    ##################################################
        
    def loadFade(self):
        if self.fade == None:
            # We create a DirectFrame for the fade polygon, instead of
            # simply loading the polygon model and using it directly,
            # so that it will also obscure mouse events for objects
            # positioned behind it.
            fadeModel = loader.loadModel(self.FadeModelName)
            self.fade = DirectFrame(
                parent = hidden,
                guiId = 'fade',
                relief = None,
                image = fadeModel,
                image_scale = 3.0,
                state = NORMAL,
                )                                    
            fadeModel.removeNode()

    def fadeIn(self, t=0.5, finishIval=None):
        """
        Play a fade in transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from black to transparent. When the color lerp is finished, it
        parents the fade polygon to hidden. 
        """
        self.noTransitions()
        self.loadFade()
        if (t == 0):
            # Fade in immediately with no lerp
            self.fade.detachNode()
        else:
            # Create a sequence that lerps the color out, then
            # parents the fade to hidden
            self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)
            self.ival = Sequence(LerpColorInterval(self.fade, t,
                                                   color = self.alphaOff,
                                                   startColor = self.alphaOn),
                                 Func(self.fade.detachNode),
                                 name = self.fadeTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()
            
    def fadeOut(self, t=0.5, finishIval=None):
        """
        Play a fade out transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from transparent to full black. When the color lerp is finished,
        it leaves the fade polygon covering the aspect2d plane until you
        fadeIn or call noFade.
        lerp
        """
        self.noTransitions()
        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)
        if (t == 0):
            # Fade out immediately with no lerp
            self.fade.setColor(self.alphaOn)
        else:
            # Create a sequence that lerps the color out, then
            # parents the fade to hidden
            self.ival = Sequence(LerpColorInterval(self.fade, t,
                                                   color = self.alphaOn,
                                                   startColor = self.alphaOff),
                                 name = self.fadeTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()

    def fadeScreen(self, alpha=0.5):
        """
        Put a semitransparent screen over the camera plane
        to darken out the world. Useful for drawing attention to
        a dialog box for instance
        """
        self.noTransitions()
        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)
        self.fade.setColor(self.alphaOn[0],
                           self.alphaOn[1],
                           self.alphaOn[2],
                           alpha)

    def fadeScreenColor(self, color):
        """
        Put a semitransparent screen over the camera plane
        to darken out the world. Useful for drawing attention to
        a dialog box for instance
        """
        self.noTransitions()
        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)
        self.fade.setColor(color)        

    def noFade(self):
        """
        Removes any current fade tasks and parents the fade polygon away
        """
        if self.ival:
            self.ival.pause()
            self.ival = None
        if self.fade:
            self.fade.detachNode()

    def setFadeColor(self, r, g, b):
        self.alphaOn.set(r,g,b,1)
        self.alphaOff.set(r,g,b,0)


    ##################################################
    # Iris
    ##################################################

    def loadIris(self):
        if self.iris == None:
            self.iris = loader.loadModel(self.IrisModelName)
            self.iris.setPos(0,0,0)

    def irisIn(self, t=0.5, finishIval=None):
        """
        Play an iris in transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the scale
        of the iris polygon up so it looks like we iris in. When the
        scale lerp is finished, it parents the iris polygon to hidden.
        """
        self.noTransitions()
        self.loadIris()
        if (t == 0):
            self.iris.detachNode()
        else:
            self.iris.reparentTo(aspect2d, FADE_SORT_INDEX)

            self.ival = Sequence(LerpScaleInterval(self.iris, t,
                                                   scale = 0.18,
                                                   startScale = 0.01),
                                 Func(self.iris.detachNode),
                                 name = self.irisTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()
            
    def irisOut(self, t=0.5, finishIval=None):
        """
        Play an iris out transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the scale
        of the iris down so it looks like we iris out. When the scale
        lerp is finished, it leaves the iris polygon covering the
        aspect2d plane until you irisIn or call noIris.
        """
        self.noTransitions()
        self.loadIris()
        self.loadFade()  # we need this to cover up the hole.
        if (t == 0):
            self.iris.detachNode()
            self.fadeOut(0)
        else:
            self.iris.reparentTo(aspect2d, FADE_SORT_INDEX)

            self.ival = Sequence(LerpScaleInterval(self.iris, t,
                                                   scale = 0.01,
                                                   startScale = 0.18),
                                 Func(self.iris.detachNode),
                                 # Use the fade to cover up the hole that the iris would leave
                                 Func(self.fadeOut, 0),
                                 name = self.irisTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()

    def noIris(self):
        """
        Removes any current iris tasks and parents the iris polygon away
        """
        if self.ival:
            self.ival.pause()
            self.ival = None
        if self.iris != None:
            self.iris.detachNode()
        # Actually we need to remove the fade too,
        # because the iris effect uses it.
        self.noFade()

    def noTransitions(self):
        """
        This call should immediately remove any and all transitions running
        """
        self.noFade()
        self.noIris()
        self.noLetterbox()

    ##################################################
    # Letterbox
    ##################################################

    def loadLetterbox(self):
        if self.letterbox == None:
            # We create a DirectFrame for the fade polygon, instead of
            # simply loading the polygon model and using it directly,
            # so that it will also obscure mouse events for objects
            # positioned behind it.
            self.letterbox = NodePath("letterbox")
            # Allow fade in and out of the bars
            self.letterbox.setTransparency(1)
            self.letterboxTop = DirectFrame(
                parent = self.letterbox,
                guiId = 'letterboxTop',
                relief = FLAT,
                state = NORMAL,
                frameColor = (0,0,0,1),
                borderWidth = (0,0),
                frameSize = (-1, 1, 0, 0.2),
                pos = (0,0,0.8),
                )
            self.letterboxBottom = DirectFrame(
                parent = self.letterbox,
                guiId = 'letterboxBottom',
                relief = FLAT,
                state = NORMAL,
                frameColor = (0,0,0,1),
                borderWidth = (0,0),
                frameSize = (-1, 1, 0, 0.2),
                pos = (0,0,-1),
                )
                                    
    def noLetterbox(self):
        """
        Removes any current letterbox tasks and parents the letterbox polygon away
        """
        if self.ival:
            self.ival.pause()
            self.ival = None
        if self.letterbox != None:
            self.letterbox.detachNode()

    def letterboxOn(self, t=0.25, finishIval=None):
        """
        Move black bars in over t seconds.
        """
        self.noTransitions()
        self.loadLetterbox()
        if (t == 0):
            self.letterbox.reparentTo(render2d, FADE_SORT_INDEX)
            self.letterboxBottom.setPos(0,0,-1)
            self.letterboxTop.setPos(0,0,0.8)
        else:
            self.letterbox.reparentTo(render2d, FADE_SORT_INDEX)
            self.ival = Sequence(Parallel(LerpPosInterval(self.letterboxBottom, t,
                                                          pos = Vec3(0,0,-1),
                                                          startPos = Vec3(0,0,-1.2)),
                                          LerpPosInterval(self.letterboxTop, t,
                                                          pos = Vec3(0,0,0.8),
                                                          startPos = Vec3(0,0,1)),
                                          LerpColorInterval(self.letterbox, t,
                                                            color = self.alphaOn,
                                                            startColor = self.alphaOff),
                                          ),
                                 name = self.letterboxTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()
            
    def letterboxOff(self, t=0.25, finishIval=None):
        """
        Move black bars away over t seconds.
        """
        self.noTransitions()
        self.loadLetterbox()
        if (t == 0):
            self.letterbox.detachNode()
        else:
            self.letterbox.reparentTo(render2d, FADE_SORT_INDEX)
            self.ival = Sequence(Parallel(LerpPosInterval(self.letterboxBottom, t,
                                                          pos = Vec3(0,0,-1.2),
                                                          startPos = Vec3(0,0,-1)),
                                          LerpPosInterval(self.letterboxTop, t,
                                                          pos = Vec3(0,0,1),
                                                          startPos = Vec3(0,0,0.8)),
                                          LerpColorInterval(self.letterbox, t,
                                                            color = self.alphaOff,
                                                            startColor = self.alphaOn),
                                          ),
                                 name = self.letterboxTaskName,
                                 )
            if finishIval:
                self.ival.append(finishIval)
            self.ival.start()
