
    """
    HTTPChannel-extensions module: contains methods to extend functionality
    of the HTTPChannel class
    """

    def spawnTask(self, name = None, callback = None, extraArgs = []):
        """Spawns a task to service the download recently requested
        via beginGetDocument(), etc., and/or downloadToFile() or
        downloadToRam().  If a callback is specified, that function is
        called when the download is complete, passing in the extraArgs
        given.

        Returns the newly-spawned task.
        """
        if not name:
            name = self.getUrl().cStr()
        from direct.task import Task
        task = Task.Task(self.doTask)
        task.callback = callback
        task.callbackArgs = extraArgs
        return taskMgr.add(task, name)    
        
    def doTask(self, task):
        from direct.task import Task
        if self.run():
            return Task.cont
        if task.callback:
            task.callback(*task.callbackArgs)
        return Task.done
    
