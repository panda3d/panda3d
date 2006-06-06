from direct.task.Task import Task

class DelayedCallback:
    """ waits N frames and then calls a callback """
    def __init__(self, frames, callback, cancelFunc=None):
        # checkFunc is optional; called every frame, if returns True, FrameDelay is cancelled
        # and callback is not called
        self._frames = frames
        self._callback = callback
        self._cancelFunc = cancelFunc
        self._taskName = uniqueName(self.__class__.__name__)
        self._startTask()
    def destroy(self):
        self._stopTask()
    def finish(self):
        self._callback()
        self.destroy()
    def _startTask(self):
        taskMgr.add(self._frameTask, self._taskName)
        self._counter = 0
    def _stopTask(self):
        taskMgr.remove(self._taskName)
    def _frameTask(self, task=None):
        if self._cancelFunc():
            self.destroy()
            return Task.done
        self._counter += 1
        if self._counter >= self._frames:
            self.finish()
            return Task.done
        return Task.cont
