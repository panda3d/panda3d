
    """
    CInterval-extensions module: contains methods to extend functionality
    of the CInterval class
    """

    from direct.directnotify.DirectNotifyGlobal import directNotify
    notify = directNotify.newCategory("Interval")

    def setT(self, t):
        # Overridden from the C++ function to call privPostEvent
        # afterward.  We do this by renaming the C++ function in
        # FFIRename.
        self._priv__cSetT(t)
        self.privPostEvent()

    def play(self, t0 = 0.0, duration = None, scale = 1.0):
        self.notify.error("using deprecated CInterval.play() interface")
        if duration:  # None or 0 implies full length
            self.start(t0, t0 + duration, scale)
        else:
            self.start(t0, -1, scale)

    def stop(self):
        self.notify.error("using deprecated CInterval.stop() interface")
        self.finish()

    def setFinalT(self):
        self.notify.error("using deprecated CInterval.setFinalT() interface")
        self.finish()

    def privPostEvent(self):
        # Call after calling any of the priv* methods to do any required
        # Python finishing steps.
        t = self.getT()
        if hasattr(self, "setTHooks"):
            for func in self.setTHooks:
                func(t)

    def popupControls(self, tl = None):
        """
        Popup control panel for interval.
        """
        from direct.showbase.TkGlobal import Toplevel, Frame, Button, LEFT, X, Pmw
        import math
        from direct.tkwidgets import EntryScale
        if tl == None:
            tl = Toplevel()
            tl.title('Interval Controls')
        outerFrame = Frame(tl)
        def entryScaleCommand(t, s=self):
            s.setT(t)
            s.pause()
        self.es = es = EntryScale.EntryScale(
            outerFrame, text = self.getName(),
            min = 0, max = math.floor(self.getDuration() * 100) / 100,
            command = entryScaleCommand)
        es.set(self.getT(), fCommand = 0)
        es.pack(expand = 1, fill = X)
        bf = Frame(outerFrame)
        # Jump to start and end
        def toStart(s=self, es=es):
            s.clearToInitial()
            s.setT(0)
            es.set(0, fCommand = 0)
        def toEnd(s=self):
            s.setT(s.getDuration())
            s.pause()
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
        def update(t, es=es):
            es.set(t, fCommand = 0)
        if not hasattr(self, "setTHooks"):
            self.setTHooks = []
        self.setTHooks.append(update)
        self.setWantsTCallback(1)
        # Clear out function on destroy
        def onDestroy(e, s=self, u=update):
            if u in s.setTHooks:
                s.setTHooks.remove(u)
        tl.bind('<Destroy>', onDestroy)
