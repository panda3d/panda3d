
from PandaModules import *
import Task

class Transitions:
    def __init__(self, loader):
        self.iris = loader.loadModel("phase_3/models/misc/iris")
        self.iris.setBin("fixed", 1000)
        self.fade = loader.loadModel("phase_3/models/misc/fade")
        self.fade.setBin("fixed", 1000)

        self.iris.setPos(0,0,0)
        self.fade.setScale(3)

        self.irisTaskName = "irisTask"
        self.fadeTaskName = "fadeTask"
        
    def fadeInLerpDone(self, task):
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

	self.fade.reparentTo(aspect2d)

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
                Task.Task(self.fadeInLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.spawnTaskNamed(task, self.fadeTaskName)
            else:
                return task
            
    def fadeInTask(self, task, time=0.5):
        """
        As a sequence: Fade in, execute the given task
        """
        seq = Task.sequence(self.fadeIn(time, block=1), task)
        taskMgr.spawnTaskNamed(seq, 'fadeInTaskSeq')

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

	self.fade.reparentTo(aspect2d)

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
        self.fade.reparentTo(aspect2d)
        self.fade.setColor(0,0,0,alpha)

    def fadeOutTask(self, task, time=0.5, noFade=1):
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
        taskMgr.spawnTaskNamed(seq, 'fadeOutTaskSeq')

    def noFade(self):
        """
        Removes any current fade tasks and parents the fade polygon away
        """
	taskMgr.removeTasksNamed(self.fadeTaskName)
	self.fade.reparentTo(hidden)
        
    def irisInLerpDone(self, task):
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

        if (t == 0):
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(aspect2d)
            self.iris.setScale(0.0015)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.18, 0.18, 0.18,
                                    t, blendType="noBlend"),
                Task.Task(self.irisInLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.spawnTaskNamed(task, self.irisTaskName)
            else:
                return task
            
    def irisInTask(self, task, time=0.5):
        """
        As a sequence: iris in, execute the given task
        """
        seq = Task.sequence(self.irisIn(time, block=1), task)
        taskMgr.spawnTaskNamed(seq, 'irisInTaskSeq')

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
        if (t == 0):
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(aspect2d)
            self.iris.setScale(0.18)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.0015, 0.0015, 0.0015,
                                    t, blendType="noBlend"),
                Task.Task(self.irisOutLerpDone))
            # Spawn the sequence
            if not block:
                taskMgr.spawnTaskNamed(task, self.irisTaskName)
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
        taskMgr.spawnTaskNamed(seq, 'irisOutTaskSeq')

    def noIris(self):
        """
        Removes any current iris tasks and parents the iris polygon away
        """
	taskMgr.removeTasksNamed(self.irisTaskName)
	self.iris.reparentTo(hidden)
        # Actually we need to remove the fade to, because the iris effect uses it
        self.noFade()
    

    def noTransitions(self):
        """
        This call should immediately remove any and all transitions running
        """
	self.noFade()
	self.noIris()
