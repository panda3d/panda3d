
    """
    CInterval-extensions module: contains methods to extend functionality
    of the CInterval class
    """

    def setT(self, t):
        t = min(max(t, 0.0), self.getDuration())
        state = self.getState()
        if state == CInterval.SInitial:
            self.privInitialize(t)
        elif state == CInterval.SFinal:
            self.privReverseInitialize(t)
        else:
            self.privStep(t)
        self.privPostEvent()

    def start(self, t0 = 0.0, duration = None, scale = 1.0):
        if self.isPlaying():
            self.finish()
        if duration:  # None or 0 implies full length
            self.setupPlay(t0, t0 + duration, scale)
        else:
            self.setupPlay(t0, -1, scale)
        self.privPostEvent()
        self.__loop = 0
        self.__spawnTask()

    def loop(self, t0 = 0.0, duration = None, scale = 1.0):
        self.start(t0, duration, scale)
        self.__loop = 1
        return

    def pause(self):
        if self.getState() == CInterval.SStarted:
            self.privInterrupt()
        self.privPostEvent()
        self.__removeTask()
        return self.getT()

    def resume(self, t0 = None):
        if not hasattr(self, "_CInterval__loop"):
            self.__loop = 0
        if t0 != None:
            self.setT(t0)
        self.setupResume()
        if not self.isPlaying():
            self.__spawnTask()
        
    def finish(self):
        state = self.getState()
        if state == CInterval.SInitial:
            self.privInstant()
        elif state != CInterval.SFinal:
            self.privFinalize()
        self.privPostEvent()
        self.__removeTask()

    def play(self, *args, **kw):
        self.start(*args, **kw)

    def stop(self):
        self.finish()

    def setFinalT(self):
        self.finish()

    def isPlaying(self):
        return taskMgr.hasTaskNamed(self.getName() + '-play')

    def privPostEvent(self):
        # Call after calling any of the priv* methods to do any required
        # Python finishing steps.
        t = self.getT()
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(t)

    def __spawnTask(self):
        # Spawn task
        import Task
        taskName = self.getName() + '-play'
        task = Task.Task(self.__playTask)
        task.interval = self
        taskMgr.add(task, taskName)

    def __removeTask(self):
        # Kill old task(s), including those from a similarly-named but
        # different interval.
        taskName = self.getName() + '-play'
        oldTasks = taskMgr.getTasksNamed(taskName)
        for task in oldTasks:
            if hasattr(task, "interval"):
                task.interval.privInterrupt()
                taskMgr.remove(task)

    def __playTask(self, task):
        import Task
        loopCount = self.stepPlay()
        self.privPostEvent()
        if loopCount == 0 or self.__loop:
            return Task.cont
        else:
            return Task.done

    def popupControls(self, tl = None):
        """ popupControls()
            Popup control panel for interval.
        """
        import TkGlobal
        import math
        # I moved this here because Toontown does not ship Tk
        from Tkinter import Toplevel, Frame, Button, LEFT, X
        import Pmw
        import EntryScale
        if tl == None:
            tl = Toplevel()
            tl.title('Interval Controls')
        outerFrame = Frame(tl)
        def entryScaleCommand(t,s=self):
            s.pause()
            s.setT(t)
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = math.floor(self.getDuration() * 100) / 100,
            command = entryScaleCommand)
        es.set(self.getT(), fCommand = 0)
        es.pack(expand = 1, fill = X)
        bf = Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.pause()
            s.setT(0.0)
        def toEnd(s=self):
            s.pause()
            s.setT(s.getDuration())
        jumpToStart = Button(bf, text = '<<', command = toStart)
        # Stop/play buttons
        def doPlay(s=self, es=es):
            s.resume(es.get())
                       
        stop = Button(bf, text = 'Stop',
                      command = lambda s=self: s.pause())
        play = Button(
            bf, text = 'Play',
            command = doPlay)
        jumpToEnd = Button(bf, text = '>>', command = toEnd)
        jumpToStart.pack(side = LEFT, expand = 1, fill = X)
        play.pack(side = LEFT, expand = 1, fill = X)
        stop.pack(side = LEFT, expand = 1, fill = X)
        jumpToEnd.pack(side = LEFT, expand = 1, fill = X)
        bf.pack(expand = 1, fill = X)
        outerFrame.pack(expand = 1, fill = X)
        # Add function to update slider during setT calls
        def update(t,es=es):
            es.set(t, fCommand = 0)
        if not hasattr(self, "setTHooks"):
            self.setTHooks = []
        self.setTHooks.append(update)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)
