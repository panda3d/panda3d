
from PandaModules import *
import Task

class Transitions:
    def __init__(self, loader):
        self.iris = loader.loadModel("phase_3/models/misc/iris")
        self.fade = loader.loadModel("phase_3/models/misc/fade")

        self.iris.setPos(0,0,0)
        self.fade.setScale(2.0, 2.0, 2.6666)

        self.irisTaskName = "irisTask"
        self.fadeTaskName = "fadeTask"
        
    def fadeInLerpDone(self, task):
        # This is a helper function to the fadeIn sequence
        self.fade.reparentTo(hidden)
        return Task.done
            
    def fadeIn(self, t=0.4):
        """
        Play a fade in transition over t seconds.
        Places a polygon on the render2d plane then lerps the color
        from black to transparent. When the color lerp is finished, it
        parents the fade polygon to hidden.
        """
        self.noTransitions()

	self.fade.reparentTo(render2d)

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
            taskMgr.spawnTaskNamed(task, self.fadeTaskName)
        
    def fadeOut(self, t=0.4):
        """
        Play a fade out transition over t seconds.
        Places a polygon on the render2d plane then lerps the color
        from transparent to full black. When the color lerp is finished,
        it leaves the fade polygon covering the render2d plane until you
        fadeIn or call noFade.
        """
        self.noTransitions()

	self.fade.reparentTo(render2d)

        if (t == 0):
            # Fade out immediately with no lerp
            self.fade.setColor(0,0,0,1)
        else:
            # Spawn a lerp of the color from no alpha to full alpha
            self.fade.lerpColor(0,0,0,0,
                                0,0,0,1,
                                t, task=self.fadeTaskName)
        
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

    def irisIn(self, t=0.4):
        """
        Play an iris in transition over t seconds.
        Places a polygon on the render2d plane then lerps the scale
        of the iris polygon up so it looks like we iris in. When the
        scale lerp is finished, it parents the iris polygon to hidden.
        """
	self.noTransitions()

        if (t == 0):
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(render2d)
            self.iris.setScale(0.0015, 0.0015, 0.002)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.1, 0.1, 0.1333,
                                    t, blendType="easeIn"),
                Task.Task(self.irisInLerpDone))
            # Spawn the sequence
            taskMgr.spawnTaskNamed(task, self.irisTaskName)

    def irisOutLerpDone(self, task):
        # This is a helper function to the fadeIn sequence
        self.iris.reparentTo(hidden)
        # Use the fade to cover up the hole that the iris would leave
        self.fadeOut(0)
        return Task.done

    def irisOut(self, t=0.4):
        """
        Play an iris out transition over t seconds.
        Places a polygon on the render2d plane then lerps the scale
        of the iris down so it looks like we iris out. When the scale
        lerp is finished, it leaves the iris polygon covering the
        render2d plane until you irisIn or call noIris.
        """
	self.noTransitions()

        if (t == 0):
            self.iris.reparentTo(hidden)
        else:
            self.iris.reparentTo(render2d)
            self.iris.setScale(0.1, 0.1, 0.13333)
            # Create a sequence that scales the iris up,
            # then parents the fade to hidden
            task = Task.sequence(
                self.iris.lerpScale(0.0015, 0.0015, 0.002,
                                    t, blendType="easeIn"),
                Task.Task(self.irisOutLerpDone))
            # Spawn the sequence
            taskMgr.spawnTaskNamed(task, self.irisTaskName)


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
