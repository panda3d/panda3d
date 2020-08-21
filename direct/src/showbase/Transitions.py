"""This module defines various transition effects that can be used to
graphically transition between two scenes, such as by fading the screen to
a particular color."""

__all__ = ['Transitions']

from panda3d.core import *
from direct.showbase import ShowBaseGlobal
from direct.gui.DirectGui import DirectFrame
from direct.gui import DirectGuiGlobals as DGG
from direct.interval.LerpInterval import LerpColorScaleInterval, LerpColorInterval, LerpScaleInterval, LerpPosInterval
from direct.interval.MetaInterval import Sequence, Parallel
from direct.interval.FunctionInterval import Func

class Transitions:

    # These may be reassigned before the fade or iris transitions are
    # actually invoked to change the models that will be used.
    IrisModelName = "models/misc/iris"
    FadeModelName = "models/misc/fade"

    def __init__(self, loader,
                 model=None,
                 scale=3.0,
                 pos=Vec3(0, 0, 0)):
        self.transitionIval = None
        self.__transitionFuture = None
        self.letterboxIval = None
        self.__letterboxFuture = None
        self.iris = None
        self.fade = None
        self.letterbox = None
        self.fadeModel = model
        self.imagePos = pos
        if model:
            self.alphaOff = Vec4(1, 1, 1, 0)
            self.alphaOn = Vec4(1, 1, 1, 1)
            model.setTransparency(1)
            self.lerpFunc = LerpColorScaleInterval
        else:
            self.alphaOff = Vec4(0, 0, 0, 0)
            self.alphaOn = Vec4(0, 0, 0, 1)
            self.lerpFunc = LerpColorInterval

        self.irisTaskName = "irisTask"
        self.fadeTaskName = "fadeTask"
        self.letterboxTaskName = "letterboxTask"

    def __del__(self):
        if self.fadeModel:
            self.fadeModel.removeNode()
            self.fadeModel = None

    ##################################################
    # Fade
    ##################################################

    # We can set a custom model for the fade before using it for the first time
    def setFadeModel(self, model, scale=1.0):
        self.fadeModel = model
        # We have to change some default parameters for a custom fadeModel
        self.alphaOn = Vec4(1, 1, 1, 1)

        # Reload fade if its already been created
        if self.fade:
            self.fade.destroy()
            self.fade = None
            self.loadFade()

    def loadFade(self):
        if self.fade is None:
            # We create a DirectFrame for the fade polygon, instead of
            # simply loading the polygon model and using it directly,
            # so that it will also obscure mouse events for objects
            # positioned behind it.
            self.fade = DirectFrame(
                parent = ShowBaseGlobal.hidden,
                guiId = 'fade',
                relief = None,
                image = self.fadeModel,
                image_scale = (4, 2, 2),
                state = DGG.NORMAL,
                )
            if not self.fadeModel:
                # No fade model was given, so we make this the fade model.
                self.fade["relief"] = DGG.FLAT
                self.fade["frameSize"] = (-2, 2, -1, 1)
                self.fade["frameColor"] = (0, 0, 0, 1)
                self.fade.setTransparency(TransparencyAttrib.MAlpha)
            self.fade.setBin('unsorted', 0)
            self.fade.setColor(0,0,0,0)

        self.fade.setScale(max(base.a2dRight, base.a2dTop))

    def getFadeInIval(self, t=0.5, finishIval=None, blendType='noBlend'):
        """
        Returns an interval without starting it.  This is particularly useful in
        cutscenes, so when the cutsceneIval is escaped out of we can finish the fade immediately
        """
        #self.noTransitions() masad: this creates a one frame pop, is it necessary?
        self.loadFade()

        transitionIval = Sequence(Func(self.fade.reparentTo, aspect2d, DGG.FADE_SORT_INDEX),
                                  Func(self.fade.showThrough),  # in case aspect2d is hidden for some reason
                                  self.lerpFunc(self.fade, t,
                                                self.alphaOff,
                                                # self.alphaOn,
                                                blendType=blendType
                                                ),
                                  Func(self.fade.detachNode),
                                  name = self.fadeTaskName,
                                  )
        if finishIval:
            transitionIval.append(finishIval)
        return transitionIval

    def getFadeOutIval(self, t=0.5, finishIval=None, blendType='noBlend'):
        """
        Create a sequence that lerps the color out, then
        parents the fade to hidden
        """
        self.noTransitions()
        self.loadFade()

        transitionIval = Sequence(Func(self.fade.reparentTo, aspect2d, DGG.FADE_SORT_INDEX),
                                  Func(self.fade.showThrough),  # in case aspect2d is hidden for some reason
                                  self.lerpFunc(self.fade, t,
                                                self.alphaOn,
                                                # self.alphaOff,
                                                blendType=blendType
                                                ),
                                  name = self.fadeTaskName,
                                  )
        if finishIval:
            transitionIval.append(finishIval)
        return transitionIval

    def fadeIn(self, t=0.5, finishIval=None, blendType='noBlend'):
        """
        Play a fade in transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from black to transparent. When the color lerp is finished, it
        parents the fade polygon to hidden.
        """
        gsg = base.win.getGsg()
        if gsg:
            # If we're about to fade in from black, go ahead and
            # preload all the textures etc.
            base.graphicsEngine.renderFrame()
            render.prepareScene(gsg)
            render2d.prepareScene(gsg)

        if (t == 0):
            # Fade in immediately with no lerp
            #print "transitiosn: fadeIn 0.0"
            self.noTransitions()
            self.loadFade()
            self.fade.detachNode()
            fut = AsyncFuture()
            fut.setResult(None)
            return fut
        else:
            # Create a sequence that lerps the color out, then
            # parents the fade to hidden
            self.transitionIval = self.getFadeInIval(t, finishIval, blendType)
            self.transitionIval.append(Func(self.__finishTransition))
            self.__transitionFuture = AsyncFuture()
            self.transitionIval.start()
            return self.__transitionFuture

    def fadeOut(self, t=0.5, finishIval=None, blendType='noBlend'):
        """
        Play a fade out transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from transparent to full black. When the color lerp is finished,
        it leaves the fade polygon covering the aspect2d plane until you
        fadeIn or call noFade.
        lerp
        """
        if (t == 0):
            # Fade out immediately with no lerp
            self.noTransitions()
            self.loadFade()

            self.fade.reparentTo(aspect2d, DGG.FADE_SORT_INDEX)
            self.fade.setColor(self.alphaOn)
        elif ConfigVariableBool('no-loading-screen', False):
            if finishIval:
                self.transitionIval = finishIval
                self.transitionIval.start()
        else:
            # Create a sequence that lerps the color out, then
            # parents the fade to hidden
            self.transitionIval = self.getFadeOutIval(t, finishIval, blendType)
            self.transitionIval.append(Func(self.__finishTransition))
            self.__transitionFuture = AsyncFuture()
            self.transitionIval.start()
            return self.__transitionFuture

        # Immediately done, so return a dummy future.
        fut = AsyncFuture()
        fut.setResult(None)
        return fut

    def fadeOutActive(self):
        return self.fade and self.fade.getColor()[3] > 0

    def fadeScreen(self, alpha=0.5):
        """
        Put a semitransparent screen over the camera plane
        to darken out the world. Useful for drawing attention to
        a dialog box for instance
        """
        #print "transitiosn: fadeScreen"
        self.noTransitions()
        self.loadFade()

        self.fade.reparentTo(aspect2d, DGG.FADE_SORT_INDEX)
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
        #print "transitiosn: fadeScreenColor"
        self.noTransitions()
        self.loadFade()

        self.fade.reparentTo(aspect2d, DGG.FADE_SORT_INDEX)
        self.fade.setColor(color)

    def noFade(self):
        """
        Removes any current fade tasks and parents the fade polygon away
        """
        #print "transitiosn: noFade"
        if self.transitionIval:
            self.transitionIval.pause()
            self.transitionIval = None
        if self.__transitionFuture:
            self.__transitionFuture.cancel()
            self.__transitionFuture = None
        if self.fade:
            # Make sure to reset the color, since fadeOutActive() is looking at it
            self.fade.setColor(self.alphaOff)
            self.fade.detachNode()

    def setFadeColor(self, r, g, b):
        self.alphaOn.set(r, g, b, 1)
        self.alphaOff.set(r, g, b, 0)


    ##################################################
    # Iris
    ##################################################

    def loadIris(self):
        if self.iris == None:
            self.iris = loader.loadModel(self.IrisModelName)
            self.iris.setPos(0, 0, 0)

    def irisIn(self, t=0.5, finishIval=None, blendType = 'noBlend'):
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
            fut = AsyncFuture()
            fut.setResult(None)
            return fut
        else:
            self.iris.reparentTo(aspect2d, DGG.FADE_SORT_INDEX)

            scale = 0.18 * max(base.a2dRight, base.a2dTop)
            self.transitionIval = Sequence(LerpScaleInterval(self.iris, t,
                                                   scale = scale,
                                                   startScale = 0.01,
                                                   blendType=blendType),
                                 Func(self.iris.detachNode),
                                 Func(self.__finishTransition),
                                 name = self.irisTaskName,
                                 )
            self.__transitionFuture = AsyncFuture()
            if finishIval:
                self.transitionIval.append(finishIval)
            self.transitionIval.start()
            return self.__transitionFuture

    def irisOut(self, t=0.5, finishIval=None, blendType='noBlend'):
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
            fut = AsyncFuture()
            fut.setResult(None)
            return fut
        else:
            self.iris.reparentTo(aspect2d, DGG.FADE_SORT_INDEX)

            scale = 0.18 * max(base.a2dRight, base.a2dTop)
            self.transitionIval = Sequence(LerpScaleInterval(self.iris, t,
                                                   scale = 0.01,
                                                   startScale = scale,
                                                   blendType=blendType),
                                 Func(self.iris.detachNode),
                                 # Use the fade to cover up the hole that the iris would leave
                                 Func(self.fadeOut, 0),
                                 Func(self.__finishTransition),
                                 name = self.irisTaskName,
                                 )
            self.__transitionFuture = AsyncFuture()
            if finishIval:
                self.transitionIval.append(finishIval)
            self.transitionIval.start()
            return self.__transitionFuture

    def noIris(self):
        """
        Removes any current iris tasks and parents the iris polygon away
        """
        if self.transitionIval:
            self.transitionIval.pause()
            self.transitionIval = None
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
        # Letterbox is not really a transition, it is a screen overlay
        # self.noLetterbox()

    def __finishTransition(self):
        if self.__transitionFuture:
            self.__transitionFuture.setResult(None)
            self.__transitionFuture = None

    ##################################################
    # Letterbox
    ##################################################

    def loadLetterbox(self):
        if not self.letterbox:
            # We create a DirectFrame for the fade polygon, instead of
            # simply loading the polygon model and using it directly,
            # so that it will also obscure mouse events for objects
            # positioned behind it.
            self.letterbox = NodePath("letterbox")
            # Allow fade in and out of the bars
            self.letterbox.setTransparency(1)

            # Allow DirectLabels to be parented to the letterbox sensibly
            self.letterbox.setBin('unsorted', 0)

            # Allow a custom look to the letterbox graphic.

            # TODO: This model isn't available everywhere.  We should
            # pass it in as a parameter.
            button = loader.loadModel('models/gui/toplevel_gui',
                                      okMissing = True)

            barImage = None
            if button:
                barImage = button.find('**/generic_button')

            self.letterboxTop = DirectFrame(
                parent = self.letterbox,
                guiId = 'letterboxTop',
                relief = DGG.FLAT,
                state = DGG.NORMAL,
                frameColor = (0, 0, 0, 1),
                borderWidth = (0, 0),
                frameSize = (-1, 1, 0, 0.2),
                pos = (0, 0, 1.0),
                image = barImage,
                image_scale = (2.25,1,.5),
                image_pos = (0,0,.1),
                image_color = (0.3,0.3,0.3,1),
                sortOrder = 0,
                )
            self.letterboxBottom = DirectFrame(
                parent = self.letterbox,
                guiId = 'letterboxBottom',
                relief = DGG.FLAT,
                state = DGG.NORMAL,
                frameColor = (0, 0, 0, 1),
                borderWidth = (0, 0),
                frameSize = (-1, 1, 0, 0.2),
                pos = (0, 0, -1.2),
                image = barImage,
                image_scale = (2.25,1,.5),
                image_pos = (0,0,.1),
                image_color = (0.3,0.3,0.3,1),
                sortOrder = 0,
                )

            # masad: always place these at the bottom of render
            self.letterboxTop.setBin('sorted',0)
            self.letterboxBottom.setBin('sorted',0)
            self.letterbox.reparentTo(render2d, -1)
            self.letterboxOff(0)

    def noLetterbox(self):
        """
        Removes any current letterbox tasks and parents the letterbox polygon away
        """
        if self.letterboxIval:
            self.letterboxIval.pause()
            self.letterboxIval = None
        if self.__letterboxFuture:
            self.__letterboxFuture.cancel()
            self.__letterboxFuture = None
        if self.letterbox:
            self.letterbox.stash()

    def __finishLetterbox(self):
        if self.__letterboxFuture:
            self.__letterboxFuture.setResult(None)
            self.__letterboxFuture = None

    def letterboxOn(self, t=0.25, finishIval=None, blendType='noBlend'):
        """
        Move black bars in over t seconds.
        """
        self.noLetterbox()
        self.loadLetterbox()
        self.letterbox.unstash()
        if (t == 0):
            self.letterboxBottom.setPos(0, 0, -1)
            self.letterboxTop.setPos(0, 0, 0.8)
            fut = AsyncFuture()
            fut.setResult(None)
            return fut
        else:
            self.__letterboxFuture = AsyncFuture()
            self.letterboxIval = Sequence(Parallel(
                LerpPosInterval(self.letterboxBottom,
                                t,
                                pos = Vec3(0, 0, -1),
                                #startPos = Vec3(0, 0, -1.2),
                                blendType=blendType
                                ),
                LerpPosInterval(self.letterboxTop,
                                t,
                                pos = Vec3(0, 0, 0.8),
                                # startPos = Vec3(0, 0, 1),
                                blendType=blendType
                                ),
                ),
                                          Func(self.__finishLetterbox),
                                          name = self.letterboxTaskName,
                                          )
            if finishIval:
                self.letterboxIval.append(finishIval)
            self.letterboxIval.start()
            return self.__letterboxFuture

    def letterboxOff(self, t=0.25, finishIval=None, blendType='noBlend'):
        """
        Move black bars away over t seconds.
        """
        self.noLetterbox()
        self.loadLetterbox()
        self.letterbox.unstash()
        if (t == 0):
            self.letterbox.stash()
            fut = AsyncFuture()
            fut.setResult(None)
            return fut
        else:
            self.__letterboxFuture = AsyncFuture()
            self.letterboxIval = Sequence(Parallel(
                LerpPosInterval(self.letterboxBottom,
                                t,
                                pos = Vec3(0, 0, -1.2),
                                # startPos = Vec3(0, 0, -1),
                                blendType=blendType
                                ),
                LerpPosInterval(self.letterboxTop,
                                t,
                                pos = Vec3(0, 0, 1),
                                # startPos = Vec3(0, 0, 0.8),
                                blendType=blendType
                                ),
                ),
                                          Func(self.letterbox.stash),
                                          Func(self.__finishLetterbox),
                                          Func(messenger.send,'letterboxOff'),
                                          name = self.letterboxTaskName,
                                          )
            if finishIval:
                self.letterboxIval.append(finishIval)
            self.letterboxIval.start()
            return self.__letterboxFuture
