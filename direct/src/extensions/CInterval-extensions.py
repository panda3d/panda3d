
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
    
    def setFinalT(self):
        # We have to define this at the Python level so we can
        # implicitly call stop().
        self.stop()
        self.finalize()

    def isPlaying(self):
        return taskMgr.hasTaskNamed(self.getName() + '-play')

    def __playTask(self, task):
        import Task
        loopCount = self.stepPlay()
        if loopCount == 0 or self.__loop:
            return Task.cont
        else:
            return Task.done
