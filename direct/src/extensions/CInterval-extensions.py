
    """
    CInterval-extensions module: contains methods to extend functionality
    of the CInterval class
    """

    def play(self, t0 = 0.0, duration = None, scale = 1.0):
        """ play(t0, duration)
        """
        self.stop()
        if duration:  # None or 0 implies full length
            self.setupPlay(t0, t0 + duration, scale)
        else:
            self.setupPlay(t0, -1, scale)
        self.__loop = 0
        # Spawn task
        taskMgr.add(self.__playTask, self.getName() + '-play')

    def loop(self, t0 = 0.0, duration = None, scale = 1.0):
        self.play(t0, duration, scale)
        self.__loop = 1
        return

    def stop(self):
        """ stop()
        """
        # Kill task
        taskMgr.remove(self.getName() + '-play')
        return self.getT()

    def setT(self, t, event = ETStep):
        # Overridden from the C++ layer.  We rename the C++ function
        # in FFIRename to make this possible.
        self.__cSetT(t, event)
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(t)
    
    def setFinalT(self):
        # We have to define this at the Python level so we can
        # implicitly call stop().
        self.stop()
        self.finalize()
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(self.getT())

    def isPlaying(self):
        return taskMgr.hasTaskNamed(self.getName() + '-play')

    def __playTask(self, task):
        import Task
        loopCount = self.stepPlay()
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(self.getT())
        if loopCount == 0 or self.__loop:
            return Task.cont
        else:
            return Task.done

    def popupControls(self, tl = None):
        """ popupControls()
            Popup control panel for interval.
        """
        import TkGlobal
        import fpformat
        import string
        # I moved this here because Toontown does not ship Tk
        from Tkinter import Toplevel, Frame, Button, LEFT, X
        import Pmw
        import EntryScale
        if tl == None:
            tl = Toplevel()
            tl.title('Interval Controls')
        outerFrame = Frame(tl)
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = string.atof(fpformat.fix(self.getDuration(), 2)),
            command = lambda t, s = self: s.setT(t))
        # So when you drag scale with mouse its like you started a playback
        def onPress(s=self,es=es):
            # Kill playback task
            s.stop()
            # INIT interval
            s.setT(es.get(), CInterval.ETInitialize)
        es.onPress = onPress
        # To make sure you stop free running intervals
        es.onRelease = lambda s=self: s.stop()
        # To update scale and execute intervals with ETInitialize
        def onReturn(s = self, es = es):
            s.setT(es.get(), CInterval.ETInitialize)
            s.stop()
        es.onReturnRelease = onReturn
        es.pack(expand = 1, fill = X)
        bf = Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.setT(0.0, CInterval.ETInitialize)
            s.stop()
        def toEnd(s=self):
            s.setT(s.getDuration(), CInterval.ETInitialize)
            s.stop()
        jumpToStart = Button(bf, text = '<<', command = toStart)
        # Stop/play buttons
        stop = Button(bf, text = 'Stop',
                      command = lambda s=self: s.stop())
        play = Button(
            bf, text = 'Play',
            command = lambda s=self, es=es: s.play(es.get()))
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

        
        
