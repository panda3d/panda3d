
from pandac.PandaModules import *
from direct.gui.DirectGui import *
from direct.task import Task

class Transitions:

    # These may be reassigned before the fade or iris transitions are
    # actually invoked to change the models that will be used.
    IrisModelName = "models/misc/iris"
    FadeModelName = "models/misc/fade"

    def __init__(self, loader):
        self.iris = None
        self.fade = None

        self.irisTaskName = "irisTask"
        self.fadeTaskName = "fadeTask"
        
    def __fadeInLerpDone(self, task):
        # This is a helper function to the fadeIn sequence
        self.fade.reparentTo(hidden)
        return Task.done
            
    def fadeIn(self, t=0.5, block=0):
        """
        Play a fade in transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from black to transparent. When the color lerp is finished, it
        parents the fade polygon to hidden. If block is set, return the
        sequence instead of spawning it.
        """
        self.noTransitions()

        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)

        if (t == 0):
            # Fade in immediately with no lerp
            self.fade.reparentTo(hidden)
        else:
            # Create a sequence that lerps the color out, then
            # parents the fade to hidden
            task = Task.sequence(
                self.fade.lerpColor(0,0,0,1,
                                    0,0,0,0,
                                    t),
                Task.Task(self.__fadeInLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.add(task, self.fadeTaskName)
            else:
                return task
            
    def fadeInTask(self, task, time=0.5):
        """
        As a sequence: Fade in, execute the given task
        """
        seq = Task.sequence(self.fadeIn(time, block=1), task)
        taskMgr.add(seq, 'fadeInTaskSeq')

    def fadeOut(self, t=0.5, block=0):
        """
        Play a fade out transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the color
        from transparent to full black. When the color lerp is finished,
        it leaves the fade polygon covering the aspect2d plane until you
        fadeIn or call noFade. If block is set to 1, performs blocking
        lerp
        """
        self.noTransitions()

        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)

        if (t == 0):
            # Fade out immediately with no lerp
            self.fade.setColor(0,0,0,1)
        else:
            # Spawn a lerp of the color from no alpha to full alpha
            if not block:
                self.fade.lerpColor(0,0,0,0,
                                    0,0,0,1,
                                    t, task=self.fadeTaskName)
            else:
                return self.fade.lerpColor(0,0,0,0,
                                           0,0,0,1,
                                           t)

    def fadeScreen(self, alpha=0.5):
        """
        Put a semitransparent screen over the camera plane
        to darken out the world. Useful for drawing attention to
        a dialog box for instance
        """
        self.noTransitions()
        self.loadFade()
        self.fade.reparentTo(aspect2d, FADE_SORT_INDEX)
        self.fade.setColor(0,0,0,alpha)

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

    def fadeOutTask(self, task, time=0.3, noFade=1):
        """
        As a sequence: Fade out, execute the given task, then do a noFade
        if requested
        """
        if noFade:
            def noFadeTask(task):
                task.noFade()
                return Task.done
            nft = Task.Task(noFadeTask)
            nft.noFade = self.noFade
            seq = Task.sequence(self.fadeOut(time, block=1), task, nft)
        else:
            seq = Task.sequence(self.fadeOut(time, block=1), task)
            
        # do it
        taskMgr.add(seq, 'fadeOutTaskSeq')

    def noFade(self):
        """
        Removes any current fade tasks and parents the fade polygon away
        """
        taskMgr.remove(self.fadeTaskName)
        if self.fade != None:
            self.fade.reparentTo(hidden)
        
    def __irisInLerpDone(self, task):
        # This is a helper function to the fadeIn sequence
        self.iris.reparentTo(hidden)
        return Task.done

    def irisIn(self, t=0.5, block=0):
        """
        Play an iris in transition over t seconds.
        Places a polygon on the aspect2d plane then lerps the scale
        of the iris polygon up so it looks like we iris in. When the
        scale lerp is finished, it parents the iris polygon to hidden.
        If block is true, does not execute lerp, but returns the sequence.
        """
        self.noTransitions()

        self.loadIris()
        if (t == 0):
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(aspect2d, FADE_SORT_INDEX)
            self.iris.setScale(0.015)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.18, 0.18, 0.18,
                                    t, blendType="noBlend"),
                Task.Task(self.__irisInLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.add(task, self.irisTaskName)
            else:
                return task
            
    def irisInTask(self, task, time=0.5):
        """
        As a sequence: iris in, execute the given task
        """
        seq = Task.sequence(self.irisIn(time, block=1), task)
        taskMgr.add(seq, 'irisInTaskSeq')

    def irisOutLerpDone(self, task):
        # This is a helper function to the fadeIn sequence
        self.iris.reparentTo(hidden)
        # Use the fade to cover up the hole that the iris would leave
        self.fadeOut(0)
        return Task.done

    def irisOut(self, t=0.5, block=0):
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
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(aspect2d, FADE_SORT_INDEX)
            self.iris.setScale(0.18)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.015, 0.015, 0.015,
                                    t, blendType="noBlend"),
                Task.Task(self.irisOutLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.add(task, self.irisTaskName)
            else:
                return task

    def irisOutTask(self, task, time=0.5, noIris=1):
        """
        As a sequence: iris out, execute the given task, then do a noIris
        if requested
        """
        if noIris:
            def noIrisTask(task):
                task.noIris()
                return Task.done
            nit = Task.Task(noIrisTask)
            nit.noIris = self.noIris
            seq = Task.sequence(self.irisOut(time, block=1), task, nit)
        else:
            seq = Task.sequence(self.irisOut(time, block=1), task)
            
        # do it
        taskMgr.add(seq, 'irisOutTaskSeq')

    def noIris(self):
        """
        Removes any current iris tasks and parents the iris polygon away
        """
        taskMgr.remove(self.irisTaskName)
        if self.iris != None:
            self.iris.reparentTo(hidden)

        # Actually we need to remove the fade too,
        # because the iris effect uses it.
        self.noFade()
    

    def noTransitions(self):
        """
        This call should immediately remove any and all transitions running
        """
        self.noFade()
        self.noIris()

    def loadIris(self):
        if self.iris == None:
            self.iris = loader.loadModel(self.IrisModelName)
            self.iris.setPos(0,0,0)

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

