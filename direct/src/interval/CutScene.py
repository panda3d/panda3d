from direct.showbase.DirectObject import DirectObject

__all__ = ["WaitFor", "CutScene"]


class WaitFor(DirectObject):
    """The WaitFor class allows for . It doesn't subclass from Interval so can't be used in a sequence or parallel."""

    def __init__(self, event: str | list | tuple, timeout: float = None):
        super().__init__()
        self._is_playing: bool = False
        self.time_left: float = timeout
        self.accepting: bool = False
        if type(event) not in (list, tuple):
            self.accept(event, self.fire)
        else:
            for ev in event:
                self.accept(ev, self.fire)

    def start(self) -> None:
        self._is_playing = True
        self.accepting = True

    def pause(self) -> None:
        self._is_playing = False
        self.accepting = False

    def fire(self) -> None:
        if self.accepting:
            self._is_playing = False
            self.accepting = False

    def is_playing(self):
        return self._is_playing


class CutScene(DirectObject):
    """CutScene class allows for adding a WaitFor class inbetween intervals. Allowing for you to wait for a
    event/input between them."""

    def __init__(self, intervals: list = None, auto_play: bool = False):
        super().__init__()

        self.times: int = None
        self.is_playing: bool = auto_play
        self.intervals: list = [] if intervals is None else intervals
        self.current = None
        self.index: int = 0
        self.wait_for: bool = False
        self.task = None
        if auto_play:
            self.task = self.add_task(self.update)

    def play(self, times: int = 1, interval_index: int = 0) -> None:
        """Begin playing the cutscene. This calls start on the next interval. If times is set to -1 the cutscene will
        repeat until it is paused. interval_index determines which interval the scene will first play."""
        if not self.is_playing:
            if not self.task:
                self.add_task(self.update)
            self.is_playing = True
            self.times = times
            self.current = self.intervals[interval_index]
            self.current.start()

    def loop(self) -> None:
        """Make the cutscene loop."""
        self.play(-1)

    def pause(self) -> None:
        """Pauses the current playing interval."""
        self.is_playing = False
        if self.current:
            self.current.pause()

    def add(self, interval) -> None:
        """Add a interval to be played."""
        self.intervals.append(interval)

    def remove(self, interval) -> None:
        """Removes the given interval from the list of intervals to play."""
        self.intervals.append(interval)

    def update(self, task):
        if self.is_playing:
            if type(self.current) is WaitFor:
                if self.current.time_left is not None:
                    self.current.time_left -= globalClock.get_dt()

                    if self.current.time_left <= 0:
                        self.current.pause()

            if not self.current.is_playing():
                self.index += 1

                if self.index == len(self.intervals):
                    if self.times <= 1:
                        self.times -= 1
                        if self.times == 0:
                            return task.done
                        else:
                            self.index = 0

                self.current = self.intervals[self.index]
                self.current.start()

        return task.cont

    def clean_up(self) -> None:
        """Cleans up all the CutScene object by so it can be removed."""
        self.remove_all_tasks()
        self.ignore_all()
        for i in self.intervals:
            if type(i) is WaitFor:
                i.clean_up()
            else:
                i.finish()
        self.intervals = None


if __name__ == '__main__':
    from math import pi, sin, cos
    from direct.showbase.ShowBase import ShowBase
    from direct.task import Task
    from direct.actor.Actor import Actor
    from direct.interval.IntervalGlobal import Sequence
    from panda3d.core import Point3


    class MyApp(ShowBase):

        def __init__(self):
            ShowBase.__init__(self)

            # Disable the camera trackball controls.

            self.disableMouse()

            # Load the environment model.

            self.scene = self.loader.loadModel("models/environment")

            # Reparent the model to render.

            self.scene.reparentTo(self.render)

            # Apply scale and position transforms on the model.

            self.scene.setScale(0.25, 0.25, 0.25)

            self.scene.setPos(-8, 42, 0)

            # Add the spinCameraTask procedure to the task manager.

            self.taskMgr.add(self.spinCameraTask, "SpinCameraTask")

            # Load and transform the panda actor.

            self.pandaActor = Actor("models/panda-model",

                                    {"walk": "models/panda-walk4"})

            self.pandaActor.setScale(0.005, 0.005, 0.005)

            self.pandaActor.reparentTo(self.render)

            # Loop its animation.

            self.pandaActor.loop("walk")

            # Create the four lerp intervals needed for the panda to

            # walk back and forth.
            self.pandaActor.set_pos(Point3(0, 10, 0))

            posInterval1 = self.pandaActor.posInterval(13,

                                                       Point3(0, -10, 0),

                                                       startPos=Point3(0, 10, 0))

            posInterval2 = self.pandaActor.posInterval(13,

                                                       Point3(0, 10, 0),

                                                       startPos=Point3(0, -10, 0))

            hprInterval1 = self.pandaActor.hprInterval(3,

                                                       Point3(180, 0, 0),

                                                       startHpr=Point3(0, 0, 0))

            hprInterval2 = self.pandaActor.hprInterval(3,

                                                       Point3(0, 0, 0),

                                                       startHpr=Point3(180, 0, 0))

            # Create and play the sequence that coordinates the intervals.

            self.pandaPace = Sequence(posInterval1, hprInterval1,

                                      posInterval2, hprInterval2,

                                      name="pandaPace")

            self.cutscene = CutScene()
            self.cutscene.add(WaitFor("p", timeout=3))
            self.cutscene.add(self.pandaPace)
            self.cutscene.add(WaitFor("p"))
            print(self.cutscene.intervals)
            self.cutscene.loop()

        # Define a procedure to move the camera.

        def spinCameraTask(self, task):
            angleDegrees = task.time * 6.0

            angleRadians = angleDegrees * (pi / 180.0)

            self.camera.setPos(20 * sin(angleRadians), -20 * cos(angleRadians), 3)

            self.camera.setHpr(angleDegrees, 0, 0)

            return Task.cont


    app = MyApp()
    app.run()
