from libpandaexpressModules import *
from DirectNotifyGlobal import *
from PythonUtil import *
from MessengerGlobal import *
import time
import fnmatch
import string
import signal
from bisect import bisect

# MRM: Need to make internal task variables like time, name, index
# more unique (less likely to have name clashes)

exit = -1
done = 0
cont = 1

# Task needs this because it might run before __builtin__.globalClock
# can be set.
globalClock = ClockObject.getGlobalClock()

def print_exc_plus():
    """
    Print the usual traceback information, followed by a listing of all the
    local variables in each frame.
    """
    import sys
    import traceback

    tb = sys.exc_info()[2]
    while 1:
        if not tb.tb_next:
            break
        tb = tb.tb_next
    stack = []
    f = tb.tb_frame
    while f:
        stack.append(f)
        f = f.f_back
    stack.reverse()
    traceback.print_exc()
    print "Locals by frame, innermost last"
    for frame in stack:
        print
        print "Frame %s in %s at line %s" % (frame.f_code.co_name,
                                             frame.f_code.co_filename,
                                             frame.f_lineno)
        for key, value in frame.f_locals.items():
            print "\t%20s = " % key,
            #We have to be careful not to cause a new error in our error
            #printer! Calling str() on an unknown object could cause an
            #error we don't want.
            try:                   
                print value
            except:
                print "<ERROR WHILE PRINTING VALUE>"

class Task:
    count = 0
    def __init__(self, callback, priority = 0):
        # Unique ID for each task
        self.id = Task.count
        Task.count += 1
        self.__call__ = callback
        self.__priority = priority
        self.uponDeath = None
        self.dt = 0.0
        self.maxDt = 0.0
        self.avgDt = 0.0
        self.runningTotal = 0.0
        self.pstats = None
        self.__removed = 0

    def remove(self):
        self.__removed = 1

    def isRemoved(self):
        return self.__removed

    def getPriority(self):
        return self.__priority

    def setPriority(self, pri):
        self.__priority = pri

    def setStartTimeFrame(self, startTime, startFrame):
        self.starttime = startTime
        self.startframe = startFrame

    def setCurrentTimeFrame(self, currentTime, currentFrame):
        # Calculate and store this task's time (relative to when it started)
        self.time = currentTime - self.starttime
        self.frame = currentFrame - self.startframe

    def setupPStats(self, name):
        if __debug__:
            import PStatCollector
            self.pstats = PStatCollector.PStatCollector("App:Show code:" + name)

    def finishTask(self, verbose):
        if self.uponDeath:
            self.uponDeath(self)
        if verbose:
            # We regret to announce...
            messenger.send('TaskManager-removeTask', sentArgs = [self, self.name])

    def __repr__(self):
        if hasattr(self, 'name'):
            return ('Task id: %s, name %s' % (self.id, self.name))
        else:
            return ('Task id: %s, no name' % (self.id))


def pause(delayTime):
    def func(self):
        if (self.time < self.delayTime):
            return cont
        else:
            return done
    task = Task(func)
    task.name = 'pause'
    task.delayTime = delayTime
    return task

def sequence(*taskList):
    return make_sequence(taskList)


def make_sequence(taskList):
    def func(self):
        frameFinished = 0
        taskDoneStatus = -1
        while (not frameFinished):
            task = self.taskList[self.index]
            # If this is a new task, set its start time and frame
            if (self.index > self.prevIndex):
                task.setStartTimeFrame(self.time, self.frame)
            self.prevIndex = self.index
            # Calculate this task's time since it started
            task.setCurrentTimeFrame(self.time, self.frame)
            # Execute the current task
            ret = task(task)
            # Check the return value from the task
            # If this current task wants to continue,
            # come back to it next frame
            if (ret == cont):
                taskDoneStatus = cont
                frameFinished = 1
            # If this task is done, increment the index so that next frame
            # we will start executing the next task on the list
            elif (ret == done):
                self.index = self.index + 1
                taskDoneStatus = cont
                frameFinished = 0
            # If this task wants to exit, the sequence exits
            elif (ret == exit):
                taskDoneStatus = exit
                frameFinished = 1

            # If we got to the end of the list, this sequence is done
            if (self.index >= len(self.taskList)):
                assert(TaskManager.notify.debug('sequence done: ' + self.name))
                frameFinished = 1
                taskDoneStatus = done
                
        return taskDoneStatus 

    task = Task(func)
    task.name = 'sequence'
    task.taskList = taskList
    task.prevIndex = -1
    task.index = 0
    return task

def resetSequence(task):
    # Should this automatically be done as part of spawnTaskNamed?
    # Or should one have to create a new task instance every time
    # one wishes to spawn a task (currently sequences and can
    # only be fired off once
    task.index = 0
    task.prevIndex = -1

def loop(*taskList):
    return make_loop(taskList)

def make_loop(taskList):
    def func(self):
        frameFinished = 0
        taskDoneStatus = -1
        while (not frameFinished):
            task = self.taskList[self.index]
            # If this is a new task, set its start time and frame
            if (self.index > self.prevIndex):
                task.setStartTimeFrame(self.time, self.frame)
            self.prevIndex = self.index
            # Calculate this task's time since it started
            task.setCurrentTimeFrame(self.time, self.frame)
            # Execute the current task
            ret = task(task)
            # Check the return value from the task
            # If this current task wants to continue,
            # come back to it next frame
            if (ret == cont):
                taskDoneStatus = cont
                frameFinished = 1
            # If this task is done, increment the index so that next frame
            # we will start executing the next task on the list
            # TODO: we should go to the next frame now
            elif (ret == done):
                self.index = self.index + 1
                taskDoneStatus = cont
                frameFinished = 0
            # If this task wants to exit, the sequence exits
            elif (ret == exit):
                taskDoneStatus = exit
                frameFinished = 1
            # If we got to the end of the list, wrap back around
            if (self.index >= len(self.taskList)):
                self.prevIndex = -1
                self.index = 0
                frameFinished = 1
        return taskDoneStatus
    task = Task(func)
    task.name = 'loop'
    task.taskList = taskList
    task.prevIndex = -1
    task.index = 0
    return task


class TaskPriorityList(list):
    def __init__(self, priority):
        self.__priority = priority
        self.__emptyIndex = 0
    def getPriority(self):
        return self.__priority
    def getEmptyIndex(self):
        return self.__emptyIndex
    def setEmptyIndex(self, index):
        self.__emptyIndex = index
    def add(self, task):
        if (self.__emptyIndex >= len(self)):
            self.append(task)
            self.__emptyIndex += 1
        else:
            self[self.__emptyIndex] = task
            self.__emptyIndex += 1
    def remove(self, i):
        assert(i <= len(self))
        if (len(self) == 1) and (i == 1):
            self[i] = None
            self.__emptyIndex = 0
        else:
            # Swap the last element for this one
            lastElement = self[self.__emptyIndex-1]
            self[i] = lastElement
            self[self.__emptyIndex-1] = None
            self.__emptyIndex -= 1


class DoLaterList(list):
    """
    This is a list that maintains sorted order of wakeTimes on tasks
    """
    def __init__(self):
        list.__init__(self)

    def add(self, task):
        """
        Add task, keeping the list sorted.
        This does a binary search for the index to insert into.
        Returns the index at which task was inserted.
        """
        lo = 0
        hi = len(self)
        while lo < hi:
            mid = (lo+hi)//2
            if task.wakeTime < self[mid].wakeTime:
                hi = mid
            else:
                lo = mid+1
        list.insert(self, lo, task)
        return lo

class TaskManager:

    notify = None

    def __init__(self):
        self.running = 0
        self.stepping = 0
        self.taskList = []
        # Dictionary of priority to newTaskLists
        self.pendingTaskDict = {}
        self.doLaterList = DoLaterList()
        self.currentTime, self.currentFrame = self.__getTimeFrame()
        if (TaskManager.notify == None):
            TaskManager.notify = directNotify.newCategory("TaskManager")
        self.taskTimerVerbose = 0
        self.extendedExceptions = 0
        self.fKeyboardInterrupt = 0
        self.interruptCount = 0
        self.pStatsTasks = 0
        self.resumeFunc = None
        self.fVerbose = 0
        self.nameDict = {}
        self.add(self.__doLaterProcessor, "doLaterProcessor")
        # Dictionary of task name to list of tasks with that name

    def stepping(self, value):
        self.stepping = value

    def setVerbose(self, value):
        self.fVerbose = value
        messenger.send('TaskManager-setVerbose', sentArgs = [value])

    def keyboardInterruptHandler(self, signalNumber, stackFrame):
        self.fKeyboardInterrupt = 1
        self.interruptCount += 1
        if self.interruptCount == 2:
            # The user must really want to interrupt this process
            # Next time around use the default interrupt handler
            signal.signal(signal.SIGINT, signal.default_int_handler)

    def hasTaskNamed(self, taskName):
        # TODO: check pending task list
        # Get the tasks with this name
        tasks = self.nameDict.get(taskName)
        # If we found some, see if any of them are still active (not removed)
        if tasks:
            for task in tasks:
                if not task.isRemoved():
                    return 1
        # Didnt find any, return 0
        return 0

    def getTasksNamed(self, taskName):
        # TODO: check pending tasks
        # Get the tasks with this name
        tasks = self.nameDict.get(taskName, [])
        # Filter out the tasks that have been removed
        if tasks:
            tasks = filter(lambda task: not task.isRemoved(), tasks)
        return tasks

    def __doLaterProcessor(self, task):
        # Make a temp list of all the dolaters that expired this time
        # through so we can remove them after we are done with the
        # for loop. Removing them during the for loop is a bad idea
        while self.doLaterList:
            # TODO: because this processor breaks out early, some tasks
            # which have been flagged for removal may stay on the end of
            # the doLaterList longer than expected. One brute force fix
            # would be to cycle through all tasks removing the ones that
            # are flagged each frame.
            dl = self.doLaterList[0]
            if dl.isRemoved():
                del self.doLaterList[0]
                continue
            # If the time now is less than the start of the doLater + delay
            # then we are not ready yet, continue to next one
            elif task.time < dl.wakeTime:
                # Since the list is sorted, the first one we get to, that
                # is not ready to go, we can return
                break
            else:
                assert(TaskManager.notify.debug('__doLaterProcessor: spawning %s' % (dl)))
                del self.doLaterList[0]
                dl.setStartTimeFrame(self.currentTime, self.currentFrame)
                self.__addPendingTask(dl)
                continue
        return cont

    def __spawnDoLater(self, task):
        assert(TaskManager.notify.debug('spawning doLater: %s' % (task)))
        # Add this task to the nameDict
        nameList = self.nameDict.setdefault(task.name, [])
        nameList.append(task)
        # be sure to ask the globalClock for the current frame time
        # rather than use a cached value; globalClock's frame time may
        # have been synced since the start of this frame
        currentTime = globalClock.getFrameTime()
        task.setStartTimeFrame(currentTime, self.currentFrame)
        # Cache the time we should wake up for easier sorting
        task.wakeTime = task.starttime + task.delayTime
        index = self.doLaterList.add(task)
        if self.fVerbose:
            # Alert the world, a new task is born!
            messenger.send('TaskManager-spawnDoLater',
                           sentArgs = [task, task.name, index])
        return task

    def doLater(self, delayTime, task, taskName):
        assert(TaskManager.notify.debug('doLater: %s' % (taskName)))
        task.delayTime = delayTime
        task.name = taskName
        return self.__spawnDoLater(task)

    def doMethodLater(self, delayTime, func, taskName):
        task = Task(func)
        return self.doLater(delayTime, task, taskName)

    def add(self, funcOrTask, name, priority = 0):
        """
        Add a new task to the taskMgr.
        You can add a Task object or a method that takes one argument.
        """
        assert(TaskManager.notify.debug('add: %s' % (name)))
        if isinstance(funcOrTask, Task):
            funcOrTask.setPriority(priority)
            return self.__spawnTaskNamed(funcOrTask, name)
        elif callable(funcOrTask):
            return self.__spawnMethodNamed(funcOrTask, name, priority)
        else:
            self.notify.error('add: Tried to add a task that was not a Task or a func')

    def __spawnMethodNamed(self, func, name, priority=0):
        task = Task(func, priority)
        return self.__spawnTaskNamed(task, name)

    def __spawnTaskNamed(self, task, name):
        assert(TaskManager.notify.debug('__spawnTaskNamed: %s' % (name)))
        # Init params
        task.name = name
        # be sure to ask the globalClock for the current frame time
        # rather than use a cached value; globalClock's frame time may
        # have been synced since the start of this frame
        currentTime = globalClock.getFrameTime()
        task.setStartTimeFrame(currentTime, self.currentFrame)
        nameList = self.nameDict.setdefault(name, [])
        nameList.append(task)
        # Put it on the list for the end of this frame
        self.__addPendingTask(task)
        return task

    def __addPendingTask(self, task):
        assert(TaskManager.notify.debug('__addPendingTask: %s' % (task.name)))
        pri = task.getPriority()
        if self.pendingTaskDict.has_key(pri):
            taskPriList = self.pendingTaskDict[pri]
        else:
            taskPriList = TaskPriorityList(pri)
            self.pendingTaskDict[pri] = taskPriList
        taskPriList.add(task)

    def __addNewTask(self, task):
        # The taskList is really an ordered list of TaskPriorityLists
        # search back from the end of the list until we find a
        # taskList with a lower priority, or we hit the start of the list
        taskPriority = task.getPriority()
        index = len(self.taskList) - 1
        while (1):
            if (index < 0):
                newList = TaskPriorityList(taskPriority)
                newList.add(task)
                # Add the new list to the beginning of the taskList
                self.taskList.insert(0, newList)
                break
            taskListPriority = self.taskList[index].getPriority()
            if (taskListPriority == taskPriority):
                self.taskList[index].add(task)
                break
            elif (taskListPriority > taskPriority):
                index = index - 1
            elif (taskListPriority < taskPriority):
                # Time to insert
                newList = TaskPriorityList(taskPriority)
                newList.add(task)
                # Insert this new priority level
                # If we are already at the end, just append it
                if (index == len(self.taskList)-1):
                    self.taskList.append(newList)
                else:
                    # Otherwise insert it
                    self.taskList.insert(index+1, newList)
                break

        if __debug__:
            if self.pStatsTasks and task.name != "igloop":
                # Get the PStats name for the task.  By convention,
                # this is everything until the first hyphen; the part
                # of the task name following the hyphen is generally
                # used to differentiate particular tasks that do the
                # same thing to different objects.
                name = task.name
                hyphen = name.find('-')
                if hyphen >= 0:
                    name = name[0:hyphen]
                task.setupPStats(name)
        if self.fVerbose:
            # Alert the world, a new task is born!
            messenger.send('TaskManager-spawnTask', sentArgs = [task, task.name, index])
        return task

    def remove(self, taskOrName):
        if type(taskOrName) == type(''):
            return self.__removeTasksNamed(taskOrName)
        elif isinstance(taskOrName, Task):
            return self.__removeTasksEqual(taskOrName)
        else:
            self.notify.error('remove takes a string or a Task')

    def removeTasksMatching(self, taskPattern):
        """removeTasksMatching(self, string taskPattern)

        Removes tasks whose names match the pattern, which can include
        standard shell globbing characters like *, ?, and [].

        """
        assert(TaskManager.notify.debug('removing tasks matching: ' + taskPattern))
        num = 0
        keyList = filter(lambda key: fnmatch.fnmatchcase(key, taskPattern), self.nameDict.keys())
        for key in keyList:
            num += self.__removeTasksNamed(key)
        return num

    def __removeTasksEqual(self, task):
        # Remove this task from the nameDict (should be a short list)
        if self.__removeTaskFromNameDict(task):
            assert(TaskManager.notify.debug('__removeTasksEqual: removing task: %s' % (task)))
            # Flag the task for removal from the real list
            task.remove()
            # Cleanup stuff
            task.finishTask(self.fVerbose)
            return 1
        else:
            return 0

    def __removeTasksNamed(self, taskName):
        if not self.nameDict.has_key(taskName):
            return 0
        assert(TaskManager.notify.debug('__removeTasksNamed: removing tasks named: %s' % (taskName)))
        for task in self.nameDict[taskName]:
            # Flag for removal
            task.remove()
            # Cleanup stuff
            task.finishTask(self.fVerbose)
        # Record the number of tasks removed
        num = len(self.nameDict[taskName])
        # Blow away the nameDict entry completely
        del self.nameDict[taskName]
        return num

    def __removeTaskFromNameDict(self, task):
        taskName = task.name
        # If this is the only task with that name, remove the dict entry
        tasksWithName = self.nameDict.get(taskName)
        if tasksWithName:
            if task in tasksWithName:
                tasksWithName.remove(task)
                if len(tasksWithName) == 0:
                    del self.nameDict[taskName]
                return 1
        return 0

    def __executeTask(self, task):
        task.setCurrentTimeFrame(self.currentTime, self.currentFrame)
        if not self.taskTimerVerbose:
            # don't record timing info
            ret = task(task)
        else:
            # Run the task and check the return value
            if task.pstats:
                task.pstats.start()
            startTime = globalClock.getRealTime()
            ret = task(task)
            endTime = globalClock.getRealTime()
            if task.pstats:
                task.pstats.stop()

            # Record the dt
            dt = endTime - startTime
            task.dt = dt

            # See if this is the new max
            if dt > task.maxDt:
                task.maxDt = dt

            # Record the running total of all dts so we can compute an average
            task.runningTotal = task.runningTotal + dt
            if (task.frame > 0):
                task.avgDt = (task.runningTotal / task.frame)
            else:
                task.avgDt = 0
        return ret

    def __stepThroughList(self, taskPriList):
        # Traverse the taskPriList with an iterator
        i = 0
        while (i < len(taskPriList)):
            task = taskPriList[i]
            # See if we are at the end of the real tasks
            if task is None:
                break
            # See if this task has been removed in show code
            if task.isRemoved():
                assert(TaskManager.notify.debug('__stepThroughList: task is flagged for removal %s' % (task)))
                task.finishTask(self.fVerbose)
                taskPriList.remove(i)
                # Do not increment the iterator
                continue
            # Now actually execute the task
            ret = self.__executeTask(task)
            # See if the task is done
            if (ret == cont):
                # Leave it for next frame, its not done yet
                pass
            elif ((ret == done) or (ret == exit)):
                assert(TaskManager.notify.debug('__stepThroughList: task is finished %s' % (task)))
                # Remove the task
                if not task.isRemoved():
                    assert(TaskManager.notify.debug('__stepThroughList: task not removed %s' % (task)))
                    task.remove()
                    task.finishTask(self.fVerbose)
                    self.__removeTaskFromNameDict(task)
                else:
                    assert(TaskManager.notify.debug('__stepThroughList: task already removed %s' % (task)))
                    self.__removeTaskFromNameDict(task)
                taskPriList.remove(i)
                # Do not increment the iterator
                continue
            else:
                raise StandardError, "Task named %s did not return cont, exit, or done" % task.name
            # Move to the next element
            i += 1
    
    def step(self):
        assert(TaskManager.notify.debug('step: begin'))
        self.currentTime, self.currentFrame = self.__getTimeFrame()
        # Replace keyboard interrupt handler during task list processing
        # so we catch the keyboard interrupt but don't handle it until
        # after task list processing is complete.
        self.fKeyboardInterrupt = 0
        self.interruptCount = 0
        signal.signal(signal.SIGINT, self.keyboardInterruptHandler)

        # Traverse the task list in order because it is in priority order
        for taskPriList in self.taskList:
            pri = taskPriList.getPriority()
            assert(TaskManager.notify.debug('step: running through taskList at pri: %s' % (pri)))
            self.__stepThroughList(taskPriList)

            # Now see if that generated any pending tasks for this taskPriList
            pendingTasks = self.pendingTaskDict.get(pri, [])
            while pendingTasks:
                assert(TaskManager.notify.debug('step: running through pending tasks at pri: %s' % (pri)))
                # Remove them from the pendingTaskDict
                del self.pendingTaskDict[pri]
                # Execute them
                self.__stepThroughList(pendingTasks)
                # Add these to the real taskList
                for task in pendingTasks:
                    if (task and not task.isRemoved()):
                        assert(TaskManager.notify.debug('step: moving %s from pending to taskList' % (task.name)))
                        self.__addNewTask(task)
                # See if we generated any more for this pri level
                pendingTasks = self.pendingTaskDict.get(pri, [])

        # Now that we are all done, add any left over pendingTasks generated in
        # priority levels lower than where we were when we iterated
        for taskList in self.pendingTaskDict.values():
            for task in taskList:
                if (task and not task.isRemoved()):
                    assert(TaskManager.notify.debug('step: moving %s from pending to taskList' % (task.name)))
                    self.__addNewTask(task)
        self.pendingTaskDict.clear()
        
        # Restore default interrupt handler
        signal.signal(signal.SIGINT, signal.default_int_handler)
        if self.fKeyboardInterrupt:
            raise KeyboardInterrupt
        return

    def run(self):
        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        t = globalClock.getFrameTime()
        timeDelta = t - globalClock.getRealTime()
        globalClock.setRealTime(t)

        messenger.send("resetClock", [timeDelta])

        if self.resumeFunc != None:
            self.resumeFunc()
        
        if self.stepping:
            self.step()
        else:
            self.running = 1
            while self.running:
                try:
                    self.step()
                except KeyboardInterrupt:
                    self.stop()
                except:
                    if self.extendedExceptions:
                        self.stop()
                        print_exc_plus()
                    else:
                        raise

    def stop(self):
        # Set a flag so we will stop before beginning next frame
        self.running = 0

    def replaceMethod(self, oldMethod, newFunction):
        import new
        for taskPriList in self.taskList:
            for task in taskPriList:
                if task is None:
                    break
                method = task.__call__
                if (type(method) == types.MethodType):
                    function = method.im_func
                else:
                    function = method
                #print ('function: ' + `function` + '\n' +
                #       'method: ' + `method` + '\n' +
                #       'oldMethod: ' + `oldMethod` + '\n' +
                #       'newFunction: ' + `newFunction` + '\n')
                if (function == oldMethod):
                    newMethod = new.instancemethod(newFunction,
                                                   method.im_self,
                                                   method.im_class)
                    task.__call__ = newMethod
                    # Found it return true
                    return 1
        return 0

    def __repr__(self):
        taskNameWidth = 32
        dtWidth = 10
        priorityWidth = 10
        totalDt = 0
        totalAvgDt = 0
        str = ('taskList'.ljust(taskNameWidth)
               + 'dt(ms)'.rjust(dtWidth)
               + 'avg'.rjust(dtWidth)
               + 'max'.rjust(dtWidth)
               + 'priority'.rjust(priorityWidth)
               + '\n')
        str = str + '---------------------------------------------------------------\n'
        for taskPriList in self.taskList:
            priority = `taskPriList.getPriority()`
            for task in taskPriList:
                if task is None:
                    break
                totalDt = totalDt + task.dt
                totalAvgDt = totalAvgDt + task.avgDt
                if task.isRemoved():
                    taskName = '(R)' + task.name
                else:
                    taskName = task.name
                if (self.taskTimerVerbose):
                    import fpformat
                    str = str + (taskName.ljust(taskNameWidth)
                                 + fpformat.fix(task.dt*1000, 2).rjust(dtWidth)
                                 + fpformat.fix(task.avgDt*1000, 2).rjust(dtWidth)
                                 + fpformat.fix(task.maxDt*1000, 2).rjust(dtWidth)
                                 + priority.rjust(priorityWidth)
                                 + '\n')
                else:
                    str = str + (task.name.ljust(taskNameWidth)
                                 + '----'.rjust(dtWidth)
                                 + '----'.rjust(dtWidth)
                                 + '----'.rjust(dtWidth)
                                 + priority.rjust(priorityWidth)
                                 + '\n')
        str = str + '---------------------------------------------------------------\n'
        str = str + ' pendingTasks\n'
        str = str + '---------------------------------------------------------------\n'
        for pri, taskList in self.pendingTaskDict.items():
            for task in taskList:
                remainingTime = ((task.starttime) - self.currentTime)
                if task.isRemoved():
                    taskName = '(PR)' + task.name
                else:
                    taskName = '(P)' + task.name
                if (self.taskTimerVerbose):
                    import fpformat
                    str = str + ('  ' + taskName.ljust(taskNameWidth-2)
                                 +  fpformat.fix(pri, 2).rjust(dtWidth)
                                 + '\n')
                else:
                    str = str + ('  ' + taskName.ljust(taskNameWidth-2)
                                 +  '----'.rjust(dtWidth)
                                 + '\n')
        str = str + '---------------------------------------------------------------\n'
        str = str + ' doLaterList\n'
        str = str + '---------------------------------------------------------------\n'
        for task in self.doLaterList:
            remainingTime = ((task.wakeTime) - self.currentTime)
            if task.isRemoved():
                taskName = '(R)' + task.name
            else:
                taskName = task.name
            if (self.taskTimerVerbose):
                import fpformat
                str = str + ('  ' + taskName.ljust(taskNameWidth-2)
                             +  fpformat.fix(remainingTime, 2).rjust(dtWidth)
                             + '\n')
            else:
                str = str + ('  ' + taskName.ljust(taskNameWidth-2)
                             +  '----'.rjust(dtWidth)
                             + '\n')
        str = str + '---------------------------------------------------------------\n'
        if (self.taskTimerVerbose):
            import fpformat
            str = str + ('total'.ljust(taskNameWidth)
                         + fpformat.fix(totalDt*1000, 2).rjust(dtWidth)
                         + fpformat.fix(totalAvgDt*1000, 2).rjust(dtWidth)
                         + '\n')
        else:
            str = str + ('total'.ljust(taskNameWidth)
                         + '----'.rjust(dtWidth)
                         + '----'.rjust(dtWidth)
                         + '\n')
        return str

    def resetStats(self):
        # WARNING: this screws up your do-later timings
        for task in self.taskList:
            task.dt = 0
            task.avgDt = 0
            task.maxDt = 0
            task.runningTotal = 0
            task.setStartTimeFrame(self.currentTime, self.currentFrame)

    def popupControls(self):
        import TaskManagerPanel
        return TaskManagerPanel.TaskManagerPanel(self)

    def __getTimeFrame(self):
        # WARNING: If you are testing tasks without an igloop,
        # you must manually tick the clock        
        # Ask for the time last frame
        return globalClock.getFrameTime(), globalClock.getFrameCount()


"""

import Task

def goo(task):
    print 'goo'
    return Task.done

def bar(task):
    print 'bar'
    taskMgr.add(goo, 'goo')
    return Task.done

def foo(task):
    print 'foo'
    taskMgr.add(bar, 'bar')
    return Task.done

taskMgr.add(foo, 'foo')
"""
