"""Undocumented Module"""

__all__ = ['Task', 'TaskSortList', 'TaskManager',
           'exit', 'cont', 'done', 'again',
           'sequence', 'loop', 'pause']

# This module may not import pandac.PandaModules, since it is imported
# by the Toontown Launcher before the complete PandaModules have been
# downloaded.  Instead, it imports only libpandaexpressModules, the
# subset of PandaModules that we know is available immediately.
# Methods that require more advanced C++ methods may import the
# appropriate files within their own scope.

# Actually, the above is no longer true.  The Task module is no longer
# imported before PandaModules is available.  Instead, we make use of
# the much more limited MiniTask.py for initial startup.

from pandac.libpandaexpressModules import *

from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.PythonUtil import *
from direct.showbase.MessengerGlobal import *
from direct.showbase import ExceptionVarDump
import time
import fnmatch
import string
import signal
import random
try:
    Dtool_PreloadDLL("libp3heapq")
    from libp3heapq import heappush, heappop, heapify
except:
    Dtool_PreloadDLL("libheapq")
    from libheapq import heappush, heappop, heapify
import types

if __debug__:
    # For pstats
    from pandac.PandaModules import PStatCollector

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

    # This enum is a copy of the one at the top-level.
    exit = -1
    done = 0
    cont = 1
    again = 2

    count = 0
    def __init__(self, callback, sort = 0):
        try:
            config
        except:
            pass
        else:
            if config.GetBool('record-task-creation-stack', 0):
                self.debugInitTraceback = StackTrace("Task "+str(callback), 1, 10)
        # Unique ID for each task
        self.id = Task.count
        Task.count += 1

        #set to have the task managed
        self.owner = None

        self.__call__ = callback
        self._sort = sort
        self._removed = 0
        self.dt = 0.0
        if TaskManager.taskTimerVerbose:
            self.avgDt = 0.0
            self.maxDt = 0.0
            self.runningTotal = 0.0
            self.pstats = None
            self.pstatCollector = None
        self.extraArgs = []
        # Used for doLaters
        self.wakeTime = 0.0
        # for repeating doLaters
        self.delayTime = 0.0
        self.time = 0.0

#     # Used for putting into the doLaterList
#     # the heapq calls __cmp__ via the rich compare function
#     def __cmp__(self, other):
#         if isinstance(other, Task):
#             if self.wakeTime < other.wakeTime:
#                 return -1
#             elif self.wakeTime > other.wakeTime:
#                 return 1
#             # If the wakeTimes happen to be the same, just
#             # sort them based on id
#             else:
#                 return cmp(id(self), id(other))
#         # This is important for people doing a (task != None) and such.
#         else:
#             return cmp(id(self), id(other))

#     # According to the Python manual (3.3.1), if you define a cmp operator
#     # you should also define a hash operator or your objects will not be
#     # usable in dictionaries. Since no two task objects are unique, we can
#     # just return the unique id.
#     def __hash__(self):
#         return self.id

    def remove(self):
        if not self._removed:
            if(self.owner):
                self.owner._clearTask(self)
            self._removed = 1
            # Remove any refs to real objects
            # In case we hang around the doLaterList for a while
            del self.__call__
            del self.extraArgs
            if TaskManager.taskTimerVerbose and self.pstatCollector:
                self.pstatCollector.subLevelNow(1)

    def isRemoved(self):
        return self._removed

    def isAlive(self):
        """ Returns true if the task is alive; that is, it has never
        been removed and it has not finished.  This method may
        inaccurately return true before the task has been initially
        added to the task manager. """
        
        return not self._removed

    def getSort(self):
        return self._sort

    def setSort(self, pri):
        self._sort = pri

    def getPriority(self):
        return 0

    def setPriority(self, pri):
        TaskManager.notify.error("deprecated task.setPriority() called; use setSort() instead")
        pass

    def getName(self):
        return self.name

    def setName(self, name):
        self.name = name

    def getDelay(self):
        return self.delayTime

    def setDelay(self, delay):
        self.delayTime = delay

    def getUponDeath(self):
        return self.uponDeath

    def setUponDeath(self, uponDeath):
        self.uponDeath = uponDeath

    def recalcWakeTime(self):
        """If the task is currently sleeping, this resets its wake
        time to the current time + self.delayTime.  It is as if the
        task had suddenly returned task.again.  The task will sleep
        for its current delay seconds before running again.  This
        method may therefore be used to make the task wake up sooner
        or later than it would have otherwise.

        If the task is not already sleeping, this method has no
        effect."""

        # This is a little bit hacky--we're poking around in global
        # space more than we should--but this code will be going away
        # soon in favor of the new task manager anyway.
        
        now = globalClock.getFrameTime()
        self.wakeTime = now + self.delayTime

        # Very important that we re-sort the priority queue after
        # monkeying with the wake times.
        heapify(taskMgr._TaskManager__doLaterList)
        

    def setStartTimeFrame(self, startTime, startFrame):
        self.starttime = startTime
        self.startframe = startFrame

    def setCurrentTimeFrame(self, currentTime, currentFrame):
        # Calculate and store this task's time (relative to when it started)
        self.time = currentTime - self.starttime
        self.frame = currentFrame - self.startframe

    def getNamePrefix(self):
        # get a version of the task name, omitting a hyphen or
        # underscore followed by a string of digits at the end of the
        # name.
        name = self.name
        trimmed = len(name)
        p = trimmed
        while True:
            while p > 0 and name[p - 1] in string.digits:
                p -= 1
            if p > 0 and name[p - 1] in '-_':
                p -= 1
                trimmed = p
            else:
                p = trimmed
                break

        return name[:trimmed]

    def setupPStats(self):
        if __debug__ and TaskManager.taskTimerVerbose and not self.pstats:
            # Get the PStats name for the task.  By convention,
            # this is everything until the first hyphen; the part
            # of the task name following the hyphen is generally
            # used to differentiate particular tasks that do the
            # same thing to different objects.
            name = self.name
            hyphen = name.find('-')
            if hyphen >= 0:
                name = name[0:hyphen]
            self.pstats = PStatCollector("App:Show code:" + name)
            if self.wakeTime or self.delayTime:
                self.pstatCollector = PStatCollector("Tasks:doLaters:" + name)
            else:
                self.pstatCollector = PStatCollector("Tasks:" + name)
            self.pstatCollector.addLevelNow(1)

    def finishTask(self):
        if hasattr(self, "uponDeath"):
            self.uponDeath(self)
            del self.uponDeath
        # We regret to announce...
        messenger.send('TaskManager-removeTask', sentArgs = [self])

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
Task.pause = staticmethod(pause)

def sequence(*taskList):
    return make_sequence(taskList)
Task.sequence = staticmethod(sequence)


def make_sequence(taskList):
    def func(self):
        frameFinished = 0
        taskDoneStatus = -1
        while not frameFinished:
            task = self.taskList[self.index]
            # If this is a new task, set its start time and frame
            if self.index > self.prevIndex:
                task.setStartTimeFrame(self.time, self.frame)
            self.prevIndex = self.index
            # Calculate this task's time since it started
            task.setCurrentTimeFrame(self.time, self.frame)
            # Execute the current task
            ret = task(task)
            # Check the return value from the task
            if ret == cont:
                # If this current task wants to continue,
                # come back to it next frame
                taskDoneStatus = cont
                frameFinished = 1
            elif ret == done:
                # If this task is done, increment the index so that next frame
                # we will start executing the next task on the list
                self.index = self.index + 1
                taskDoneStatus = cont
                frameFinished = 0
            elif ret == exit:
                # If this task wants to exit, the sequence exits
                taskDoneStatus = exit
                frameFinished = 1

            # If we got to the end of the list, this sequence is done
            if self.index >= len(self.taskList):
                # TaskManager.notify.debug('sequence done: ' + self.name)
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
Task.loop = staticmethod(loop)

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
            if (ret == cont):
                # If this current task wants to continue,
                # come back to it next frame
                taskDoneStatus = cont
                frameFinished = 1
            elif (ret == done):
                # If this task is done, increment the index so that next frame
                # we will start executing the next task on the list
                # TODO: we should go to the next frame now
                self.index = self.index + 1
                taskDoneStatus = cont
                frameFinished = 0
            elif (ret == exit):
                # If this task wants to exit, the sequence exits
                taskDoneStatus = exit
                frameFinished = 1
            if (self.index >= len(self.taskList)):
                # If we got to the end of the list, wrap back around
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


class TaskSortList(list):
    def __init__(self, sort):
        self._sort = sort
        self.__emptyIndex = 0
    def getSort(self):
        return self._sort
    def add(self, task):
        if (self.__emptyIndex >= len(self)):
            self.append(task)
            self.__emptyIndex += 1
        else:
            self[self.__emptyIndex] = task
            self.__emptyIndex += 1
    def remove(self, i):
        assert i <= len(self)
        if (len(self) == 1) and (i == 1):
            self[i] = None
            self.__emptyIndex = 0
        else:
            # Swap the last element for this one
            lastElement = self[self.__emptyIndex-1]
            self[i] = lastElement
            self[self.__emptyIndex-1] = None
            self.__emptyIndex -= 1

class TaskManager:

    # These class vars are generally overwritten by Config variables which
    # are read in at the start of a show (ShowBase.py or AIStart.py)

    notify = None
    # TODO: there is a bit of a bug when you default this to 0. The first
    # task we make, the doLaterProcessor, needs to have this set to 1 or
    # else we get an error.
    taskTimerVerbose = 1
    extendedExceptions = 0
    pStatsTasks = 0

    doLaterCleanupCounter = 2000

    OsdPrefix = 'task.'

    # multiple of average frame duration
    DefTaskDurationWarningThreshold = 40.

    _DidTests = False

    def __init__(self):
        self._destroyed = False
        self.running = 0
        self.stepping = 0
        self.taskList = []
        # Dictionary of sort to newTaskLists
        self.pendingTaskDict = {}
        # List of tasks scheduled to execute in the future
        self.__doLaterList = []

        self._frameProfileQueue = Queue()
        self.MaxEpockSpeed = 1.0/30.0;   

        # this will be set when it's safe to import StateVar
        self._profileFrames = None
        self._frameProfiler = None
        self._profileTasks = None
        self._taskProfiler = None
        self._taskProfileInfo = ScratchPad(
            taskId = None,
            profiled = False,
            session = None,
            )

        # We copy this value in from __builtins__ when it gets set.
        # But since the TaskManager might have to run before it gets
        # set--before it can even be available--we also have to have
        # special-case code that handles the possibility that we don't
        # have a globalClock yet.
        self.globalClock = None

        # To help cope with the possibly-missing globalClock, we get a
        # handle to Panda's low-level TrueClock object for measuring
        # small intervals.
        self.trueClock = TrueClock.getGlobalPtr()

        # We don't have a base yet, but we can query the config
        # variables directly.
        self.warnTaskDuration = ConfigVariableBool('want-task-duration-warnings', 1).getValue()
        self.taskDurationWarningThreshold = ConfigVariableDouble(
            'task-duration-warning-threshold',
            TaskManager.DefTaskDurationWarningThreshold).getValue()

        self.currentTime, self.currentFrame = self.__getTimeFrame()
        if (TaskManager.notify == None):
            TaskManager.notify = directNotify.newCategory("TaskManager")
        self.fKeyboardInterrupt = 0
        self.interruptCount = 0
        self.resumeFunc = None
        # Dictionary of task name to list of tasks with that name
        self.nameDict = {}

        # A default task.
        self._doLaterTask = self.add(self.__doLaterProcessor, "doLaterProcessor", -10)

    def finalInit(self):
        # This function should be called once during startup, after
        # most things are imported.
        pass

    def destroy(self):
        if self._destroyed:
            return

        self._frameProfileQueue.clear()
        if self._doLaterTask:
            self._doLaterTask.remove()
        if self._taskProfiler:
            self._taskProfiler.destroy()
        del self.nameDict
        del self.trueClock
        del self.globalClock
        del self.__doLaterList
        del self.pendingTaskDict
        del self.taskList
        self._destroyed = True

    def setStepping(self, value):
        self.stepping = value

    def getTaskDurationWarningThreshold(self):
        return self.taskDurationWarningThreshold

    def setTaskDurationWarningThreshold(self, threshold):
        self.taskDurationWarningThreshold = threshold

    def invokeDefaultHandler(self, signalNumber, stackFrame):
        print '*** allowing mid-frame keyboard interrupt.'
        # Restore default interrupt handler
        signal.signal(signal.SIGINT, signal.default_int_handler)
        # and invoke it
        raise KeyboardInterrupt

    def keyboardInterruptHandler(self, signalNumber, stackFrame):
        self.fKeyboardInterrupt = 1
        self.interruptCount += 1
        if self.interruptCount == 1:
            print '* interrupt by keyboard'
        elif self.interruptCount == 2:
            print '** waiting for end of frame before interrupting...'
            # The user must really want to interrupt this process
            # Next time around invoke the default handler
            signal.signal(signal.SIGINT, self.invokeDefaultHandler)

    def setupTaskChain(self, *args, **kw):
        # This is a no-op in the original task implementation; task
        # chains are not supported here.
        pass

    def hasTaskNamed(self, taskName):
        # TODO: check pending task list
        # Get the tasks with this name
        # If we found some, see if any of them are still active (not removed)
        for task in self.nameDict.get(taskName, []):
            if not task._removed:
                return 1
        # Didnt find any, return 0
        return 0

    def getTasksNamed(self, taskName):
        # TODO: check pending tasks
        # Get the tasks with this name
        return [task for task in self.nameDict.get(taskName, []) #grab all tasks with name
                   if not task._removed] #filter removed tasks

    def getTasksMatching(self, taskPattern):
        """getTasksMatching(self, string taskPattern)

        Returns tasks whose names match the pattern, which can include
        standard shell globbing characters like *, ?, and [].
        """
        keyList = filter(
            lambda key: fnmatch.fnmatchcase(key, taskPattern),
            self.nameDict.keys())

        result = []
        for key in keyList:
            for task in self.nameDict.get(key, []):
                if not task._removed:
                    result.append(task)
        return result

    def __doLaterFilter(self):
        # Filter out all the tasks that have been removed like a mark and
        # sweep garbage collector. Returns the number of tasks that have
        # been removed Warning: this creates an entirely new doLaterList.
        oldLen = len(self.__doLaterList)
        # grab all the tasks being removed so we can remove them from the nameDict
        # TODO: would be more efficient to remove from nameDict in task.remove()
        removedTasks = [task for task in self.__doLaterList
                        if task._removed]
        self.__doLaterList = [task for task in self.__doLaterList #grab all tasks with name
                              if not task._removed] #filter removed tasks
        for task in removedTasks:
            self.__removeTaskFromNameDict(task)
        # Re heapify to maintain ordering after filter
        heapify(self.__doLaterList)
        newLen = len(self.__doLaterList)
        return oldLen - newLen
        
    def __getNextDoLaterTime(self):
        if self.__doLaterList:                        
            dl = self.__doLaterList[0]
            return dl.wakeTime
        return -1;
                       

    def __doLaterProcessor(self, task):
        # Removing the tasks during the for loop is a bad idea
        # Instead we just flag them as removed
        # Later, somebody else cleans them out
        currentTime = self.__getTime()
        while self.__doLaterList:
            # Check the first one on the list to see if it is ready
            dl = self.__doLaterList[0]
            if dl._removed:
                # Get rid of this task forever
                heappop(self.__doLaterList)
                continue
            # If the time now is less than the start of the doLater + delay
            # then we are not ready yet, continue to next one
            elif currentTime < dl.wakeTime:
                # Since the list is sorted, the first one we get to, that
                # is not ready to go, we can return
                break
            else:
                # Take it off the doLaterList, set its time, and make
                # it pending
                heappop(self.__doLaterList)
                dl.setStartTimeFrame(self.currentTime, self.currentFrame)
                self.__addPendingTask(dl)
                continue
            
        # Every nth pass, let's clean out the list of removed tasks
        # This is basically a mark and sweep garbage collection of doLaters
        if ((task.frame % self.doLaterCleanupCounter) == 0):
            numRemoved = self.__doLaterFilter()
            # TaskManager.notify.debug("filtered %s removed doLaters" % numRemoved)
        return cont

    def doMethodLater(self, delayTime, funcOrTask, name, extraArgs = None,
                      sort = None, priority = None, taskChain = None,
                      uponDeath = None, appendTask = False, owner = None):
        if delayTime < 0:
            assert self.notify.warning('doMethodLater: added task: %s with negative delay: %s' % (name, delayTime))
        if isinstance(funcOrTask, Task):
            task = funcOrTask
        elif callable(funcOrTask):
            task = Task(funcOrTask, sort)
        else:
            self.notify.error('doMethodLater: Tried to add a task that was not a Task or a func')
        assert isinstance(name, str), 'Name must be a string type'

        # For historical reasons, if priority is specified but not
        # sort, it really means sort.
        if priority is not None and sort is None:
            sort = priority

        task.setSort(sort or 0)
        task.name = name
        task.owner = owner
        if extraArgs == None:
            extraArgs = []
            appendTask = True

        # if told to, append the task object to the extra args list so the
        # method called will be able to access any properties on the task
        if appendTask:
            extraArgs.append(task)
          
        task.extraArgs = extraArgs
        if uponDeath:
            task.uponDeath = uponDeath

        # TaskManager.notify.debug('spawning doLater: %s' % (task))
        # Add this task to the nameDict
        nameList = self.nameDict.get(name)
        if nameList:
            nameList.append(task)
        else:
            self.nameDict[name] = [task]
        currentTime = self.__getTime()
        # Cache the time we should wake up for easier sorting
        task.delayTime = delayTime
        task.wakeTime = currentTime + delayTime
        # Push this onto the doLaterList. The heap maintains the sorting.
        heappush(self.__doLaterList, task)

        # Alert the world, a new task is born!
        #messenger.send('TaskManager-spawnDoLater', sentArgs = [task])

        if task.owner:
            task.owner._addTask(task)
        return task

    def add(self, funcOrTask, name, sort = None, extraArgs = None,
            priority = None, uponDeath = None, appendTask = False,
            taskChain = None, owner = None):
        
        """
        Add a new task to the taskMgr.
        You can add a Task object or a method that takes one argument.
        """

        # For historical reasons, if priority is specified but not
        # sort, it really means sort.
        if priority is not None and sort is None:
            sort = priority

        # TaskManager.notify.debug('add: %s' % (name))
        if isinstance(funcOrTask, Task):
            task = funcOrTask
        elif callable(funcOrTask):
            task = Task(funcOrTask, sort)
        else:
            self.notify.error(
                'add: Tried to add a task that was not a Task or a func')
        assert isinstance(name, str), 'Name must be a string type'
        task.setSort(sort or 0)
        task.name = name
        task.owner = owner
        if extraArgs == None:
            extraArgs = []
            appendTask = True

        # if told to, append the task object to the extra args list so the
        # method called will be able to access any properties on the task
        if appendTask:
            extraArgs.append(task)

        task.extraArgs = extraArgs
        if uponDeath:
            task.uponDeath = uponDeath
        currentTime = self.__getTime()
        task.setStartTimeFrame(currentTime, self.currentFrame)
        nameList = self.nameDict.get(name)
        if nameList:
            nameList.append(task)
        else:
            self.nameDict[name] = [task]
        # Put it on the list for the end of this frame
        self.__addPendingTask(task)
        if task.owner:
            task.owner._addTask(task)
        return task

    def __addPendingTask(self, task):
        # TaskManager.notify.debug('__addPendingTask: %s' % (task.name))
        pri = task._sort
        taskPriList = self.pendingTaskDict.get(pri)
        if not taskPriList:
            taskPriList = TaskSortList(pri)
            self.pendingTaskDict[pri] = taskPriList
        taskPriList.add(task)

    def __addNewTask(self, task):
        # The taskList is really an ordered list of TaskSortLists
        # search back from the end of the list until we find a
        # taskList with a lower sort, or we hit the start of the list
        taskSort = task._sort
        index = len(self.taskList) - 1
        while (1):
            if (index < 0):
                newList = TaskSortList(taskSort)
                newList.add(task)
                # Add the new list to the beginning of the taskList
                self.taskList.insert(0, newList)
                break
            taskListSort = self.taskList[index]._sort
            if (taskListSort == taskSort):
                self.taskList[index].add(task)
                break
            elif (taskListSort > taskSort):
                index = index - 1
            elif (taskListSort < taskSort):
                # Time to insert
                newList = TaskSortList(taskSort)
                newList.add(task)
                # Insert this new sort level
                # If we are already at the end, just append it
                if (index == len(self.taskList)-1):
                    self.taskList.append(newList)
                else:
                    # Otherwise insert it
                    self.taskList.insert(index+1, newList)
                break

        if __debug__:
            if self.pStatsTasks and task.name != "igLoop":                
                task.setupPStats()

        # Alert the world, a new task is born!
        messenger.send('TaskManager-spawnTask', sentArgs = [task])
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
        # TaskManager.notify.debug('removing tasks matching: ' + taskPattern)
        num = 0
        keyList = filter(
            lambda key: fnmatch.fnmatchcase(key, taskPattern),
            self.nameDict.keys())
        for key in keyList:
            num += self.__removeTasksNamed(key)
        return num

    def __removeTasksEqual(self, task):
        # Remove this task from the nameDict (should be a short list)
        if self.__removeTaskFromNameDict(task):
            # TaskManager.notify.debug(
            #    '__removeTasksEqual: removing task: %s' % (task))
            # Flag the task for removal from the real list
            task.remove()
            task.finishTask()
            return 1
        else:
            return 0

    def __removeTasksNamed(self, taskName):
        tasks = self.nameDict.get(taskName)
        if not tasks:
            return 0
        # TaskManager.notify.debug(
        #    '__removeTasksNamed: removing tasks named: %s' % (taskName))
        for task in tasks:
            # Flag for removal
            task.remove()
            task.finishTask()
        # Record the number of tasks removed
        num = len(tasks)
        # Blow away the nameDict entry completely
        del self.nameDict[taskName]
        return num

    def __removeTaskFromNameDict(self, task):
        taskName = task.name
        # If this is the only task with that name, remove the dict entry
        tasksWithName = self.nameDict.get(taskName)
        if tasksWithName:
            if task in tasksWithName:
                # If this is the last element, just remove the entry
                # from the dictionary
                if len(tasksWithName) == 1:
                    del self.nameDict[taskName]
                else:
                    tasksWithName.remove(task)
                return 1
        return 0

    def __executeTask(self, task):
        task.setCurrentTimeFrame(self.currentTime, self.currentFrame)
        
        # cache reference to profile info here, self._taskProfileInfo might get swapped out
        # by the task when it runs
        profileInfo = self._taskProfileInfo
        doProfile = (task.id == profileInfo.taskId)
        # don't profile the same task twice in a row
        doProfile = doProfile and (not profileInfo.profiled)

        # Defer this import until we need it: some Python
        # distributions don't provide the profile and pstats modules.
        from direct.showbase.ProfileSession import ProfileSession
        
        if not self.taskTimerVerbose:
            startTime = self.trueClock.getShortTime()
            
            # don't record timing info
            if doProfile:
                profileSession = ProfileSession('TASK_PROFILE:%s' % task.name,
                                                Functor(task, *task.extraArgs))
                ret = profileSession.run()
                # set these values *after* profiling in case we're profiling the TaskProfiler
                profileInfo.session = profileSession
                profileInfo.profiled = True
            else:
                ret = task(*task.extraArgs)
            endTime = self.trueClock.getShortTime()
            
            # Record the dt
            dt = endTime - startTime
            if doProfile:
                # if we profiled, don't pollute the task's recorded duration
                dt = task.avgDt
            task.dt = dt

        else:
            # Run the task and check the return value
            if task.pstats:
                task.pstats.start()
            startTime = self.trueClock.getShortTime()
            if doProfile:
                profileSession = ProfileSession('profiled-task-%s' % task.name,
                                                Functor(task, *task.extraArgs))
                ret = profileSession.run()
                # set these values *after* profiling in case we're profiling the TaskProfiler
                profileInfo.session = profileSession
                profileInfo.profiled = True
            else:
                ret = task(*task.extraArgs)
            endTime = self.trueClock.getShortTime()
            if task.pstats:
                task.pstats.stop()

            # Record the dt
            dt = endTime - startTime
            if doProfile:
                # if we profiled, don't pollute the task's recorded duration
                dt = task.avgDt
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

        # warn if the task took too long
        if self.warnTaskDuration and self.globalClock:
            avgFrameRate = self.globalClock.getAverageFrameRate()
            if avgFrameRate > .00001:
                avgFrameDur = (1. / avgFrameRate)
                if dt >= (self.taskDurationWarningThreshold * avgFrameDur):
                    assert TaskManager.notify.warning('frame %s: task %s ran for %.2f seconds, avg frame duration=%.2f seconds' % (
                        globalClock.getFrameCount(), task.name, dt, avgFrameDur))
            
        return ret

    def __repeatDoMethod(self, task):
        """
        Called when a task execute function returns Task.again because
        it wants the task to execute again after the same or a modified
        delay (set 'delayTime' on the task object to change the delay)
        """
        if (not task._removed):
            # be sure to ask the globalClock for the current frame time
            # rather than use a cached value; globalClock's frame time may
            # have been synced since the start of this frame
            currentTime = self.__getTime()
            # Cache the time we should wake up for easier sorting
            task.wakeTime = currentTime + task.delayTime
            # Push this onto the doLaterList. The heap maintains the sorting.
            heappush(self.__doLaterList, task)

            # Alert the world, a new task is born!
            #messenger.send('TaskManager-againDoLater', sentArgs = [task])

    def __stepThroughList(self, taskPriList):
        # Traverse the taskPriList with an iterator
        i = 0
        while (i < len(taskPriList)):
            task = taskPriList[i]
            # See if we are at the end of the real tasks
            if task is None:
                break
            # See if this task has been removed in show code
            if task._removed:
                # assert TaskManager.notify.debug(
                #    '__stepThroughList: task is flagged for removal %s' % (task))
                # If it was removed in show code, it will need finishTask run
                # If it was removed by the taskMgr, it will not, but that is ok
                # because finishTask is safe to call twice
                task.finishTask()
                taskPriList.remove(i)
                self.__removeTaskFromNameDict(task)
                # Do not increment the iterator
                continue
            # Now actually execute the task
            ret = self.__executeTask(task)
            # See if the task is done
            if (ret == cont):
                # Leave it for next frame, its not done yet
                pass
            elif (ret == again):
                # repeat doLater again after a delay
                self.__repeatDoMethod(task)
                taskPriList.remove(i)
                continue
            elif ((ret == done) or (ret == exit) or (ret == None)):
                # assert TaskManager.notify.debug(
                #    '__stepThroughList: task is finished %s' % (task))
                # Remove the task
                if not task._removed:
                    # assert TaskManager.notify.debug(
                    #    '__stepThroughList: task not removed %s' % (task))
                    task.remove()
                    # Note: Should not need to remove from doLaterList here
                    # because this task is not in the doLaterList
                    task.finishTask()
                    self.__removeTaskFromNameDict(task)
                else:
                    # assert TaskManager.notify.debug(
                    #    '__stepThroughList: task already removed %s' % (task))
                    self.__removeTaskFromNameDict(task)
                taskPriList.remove(i)
                # Do not increment the iterator
                continue
            else:
                raise StandardError, \
                    "Task named %s did not return cont, exit, done, or None" % \
                    (task.name,)
            # Move to the next element
            i += 1

    def __addPendingTasksToTaskList(self):
        # Now that we are all done, add any left over pendingTasks
        # generated in sort levels lower or higher than where
        # we were when we iterated
        for taskList in self.pendingTaskDict.values():
            for task in taskList:
                if (task and not task._removed):
                    # assert TaskManager.notify.debug(
                    #    'step: moving %s from pending to taskList' % (task.name))
                    self.__addNewTask(task)
        self.pendingTaskDict.clear()

    def getProfileSession(self, name=None):
        # call to get a profile session that you can modify before passing to profileFrames()
        if name is None:
            name = 'taskMgrFrameProfile'

        # Defer this import until we need it: some Python
        # distributions don't provide the profile and pstats modules.
        from direct.showbase.ProfileSession import ProfileSession
        return ProfileSession(name)

    def profileFrames(self, num=None, session=None, callback=None):
        if num is None:
            num = 1
        if session is None:
            session = self.getProfileSession()
        # make sure the profile session doesn't get destroyed before we're done with it
        session.acquire()
        self._frameProfileQueue.push((num, session, callback))
    

    # in the event we want to do frame time managment.. this is the function to 
    #  replace or overload..        
    def  doYield(self , frameStartTime, nextScheuledTaksTime):
          None
          
    def  doYieldExample(self , frameStartTime, nextScheuledTaksTime):
        minFinTime = frameStartTime + self.MaxEpockSpeed
        if nextScheuledTaksTime > 0 and nextScheuledTaksTime < minFinTime:
            print ' Adjusting Time'
            minFinTime = nextScheuledTaksTime;
        delta = minFinTime - self.globalClock.getRealTime();
        while(delta > 0.002):
            print ' sleep %s'% (delta)
            time.sleep(delta)           
            delta = minFinTime - self.globalClock.getRealTime();
    

    def _doProfiledFrames(self, numFrames):
        for i in xrange(numFrames):
            result = self.step()
        return result

    def getProfileFrames(self):
        return self._profileFrames.get()

    def getProfileFramesSV(self):
        return self._profileFrames

    def setProfileFrames(self, profileFrames):
        self._profileFrames.set(profileFrames)
        if (not self._frameProfiler) and profileFrames:
            # import here due to import dependencies
            from direct.task.FrameProfiler import FrameProfiler
            self._frameProfiler = FrameProfiler()

    def getProfileTasks(self):
        return self._profileTasks.get()

    def getProfileTasksSV(self):
        return self._profileTasks

    def setProfileTasks(self, profileTasks):
        self._profileTasks.set(profileTasks)
        if (not self._taskProfiler) and profileTasks:
            # import here due to import dependencies
            from direct.task.TaskProfiler import TaskProfiler
            self._taskProfiler = TaskProfiler()

    def logTaskProfiles(self, name=None):
        if self._taskProfiler:
            self._taskProfiler.logProfiles(name)

    def flushTaskProfiles(self, name=None):
        if self._taskProfiler:
            self._taskProfiler.flush(name)

    def _setProfileTask(self, task):
        if self._taskProfileInfo.session:
            self._taskProfileInfo.session.release()
            self._taskProfileInfo.session = None
        self._taskProfileInfo = ScratchPad(
            taskId = task.id,
            profiled = False,
            session = None,
            )

    def _hasProfiledDesignatedTask(self):
        # have we run a profile of the designated task yet?
        return self._taskProfileInfo.profiled

    def _getLastTaskProfileSession(self):
        return self._taskProfileInfo.session

    def _getRandomTask(self):
        numTasks = 0
        for name in self.nameDict.iterkeys():
            numTasks += len(self.nameDict[name])
        numDoLaters = len(self.__doLaterList)
        if random.random() < (numDoLaters / float(numTasks + numDoLaters)):
            # grab a doLater that will most likely trigger in the next frame
            tNow = globalClock.getFrameTime()
            avgFrameRate = globalClock.getAverageFrameRate()
            if avgFrameRate < .00001:
                avgFrameDur = 0.
            else:
                avgFrameDur = (1. / globalClock.getAverageFrameRate())
            tNext = tNow + avgFrameDur
            # binary search to find doLaters that are likely to trigger on the next frame
            curIndex = int(numDoLaters / 2)
            rangeStart = 0
            rangeEnd = numDoLaters
            while True:
                if tNext < self.__doLaterList[curIndex].wakeTime:
                    rangeEnd = curIndex
                else:
                    rangeStart = curIndex
                prevIndex = curIndex
                curIndex = int((rangeStart + rangeEnd) / 2)
                if curIndex == prevIndex:
                    break
            index = curIndex
            task = self.__doLaterList[random.randrange(index+1)]
        else:
            # grab a task
            name = random.choice(self.nameDict.keys())
            task = random.choice(self.nameDict[name])
        return task

    def step(self):
        # assert TaskManager.notify.debug('step: begin')
        self.currentTime, self.currentFrame = self.__getTimeFrame()
        startFrameTime = None
        if self.globalClock:
            startFrameTime = self.globalClock.getRealTime()
            
        # Replace keyboard interrupt handler during task list processing
        # so we catch the keyboard interrupt but don't handle it until
        # after task list processing is complete.
        self.fKeyboardInterrupt = 0
        self.interruptCount = 0
        signal.signal(signal.SIGINT, self.keyboardInterruptHandler)

        # Traverse the task list in order because it is in sort order
        priIndex = 0
        while priIndex < len(self.taskList):
            taskPriList = self.taskList[priIndex]
            pri = taskPriList._sort
            # assert TaskManager.notify.debug(
            #    'step: running through taskList at pri: %s, priIndex: %s' %
            #    (pri, priIndex))
            self.__stepThroughList(taskPriList)

            # Now see if that generated any pending tasks for this taskPriList
            pendingTasks = self.pendingTaskDict.get(pri)
            while pendingTasks:
                # assert TaskManager.notify.debug('step: running through pending tasks at pri: %s' % (pri))
                # Remove them from the pendingTaskDict
                del self.pendingTaskDict[pri]
                # Execute them
                self.__stepThroughList(pendingTasks)
                # Add these to the real taskList
                for task in pendingTasks:
                    if (task and not task._removed):
                        # assert TaskManager.notify.debug('step: moving %s from pending to taskList' % (task.name))
                        self.__addNewTask(task)
                # See if we generated any more for this pri level
                pendingTasks = self.pendingTaskDict.get(pri)

            # Any new tasks that were made pending should be converted
            # to real tasks now in case they need to run this frame at a
            # later sort level
            self.__addPendingTasksToTaskList()

            # Go to the next sort level
            priIndex += 1

        # Add new pending tasks
        self.__addPendingTasksToTaskList()
        
        if startFrameTime:
            #this is the spot for a Internal Yield Function
            nextTaskTime = self.__getNextDoLaterTime()
            self.doYield(startFrameTime,nextTaskTime)            
        
        # Restore default interrupt handler
        signal.signal(signal.SIGINT, signal.default_int_handler)
        if self.fKeyboardInterrupt:
            raise KeyboardInterrupt


    def run(self):
        # do things that couldn't be done earlier because of import dependencies
        if (not TaskManager._DidTests) and __debug__:
            TaskManager._DidTests = True
            self._runTests()

        if not self._profileTasks:
            if 'base' in __builtins__ or \
               'simbase' in __builtins__:
                from direct.fsm.StatePush import StateVar
                self._profileTasks = StateVar(False)
                self.setProfileTasks(getBase().config.GetBool('profile-task-spikes', 0))
                self._profileFrames = StateVar(False)
                self.setProfileFrames(ConfigVariableBool('profile-frames', 0).getValue())

        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        if self.globalClock:
            t = self.globalClock.getFrameTime()
            timeDelta = t - globalClock.getRealTime()
            self.globalClock.setRealTime(t)
            messenger.send("resetClock", [timeDelta])

        if self.resumeFunc != None:
            self.resumeFunc()

        if self.stepping:
            self.step()
        else:
            self.running = 1
            while self.running:
                try:
                    if len(self._frameProfileQueue):
                        numFrames, session, callback = self._frameProfileQueue.pop()
                        def _profileFunc(numFrames=numFrames):
                            self._doProfiledFrames(numFrames)
                        session.setFunc(_profileFunc)
                        session.run()
                        _profileFunc = None
                        if callback:
                            callback()
                        session.release()
                    else:
                        self.step()
                except KeyboardInterrupt:
                    self.stop()
                except IOError, ioError:
                    code, message = self._unpackIOError(ioError)
                    # Since upgrading to Python 2.4.1, pausing the execution
                    # often gives this IOError during the sleep function:
                    #     IOError: [Errno 4] Interrupted function call
                    # So, let's just handle that specific exception and stop.
                    # All other IOErrors should still get raised.
                    # Only problem: legit IOError 4s will be obfuscated.
                    if code == 4:
                        self.stop()
                    else:
                        raise
                except Exception, e:
                    if self.extendedExceptions:
                        self.stop()
                        print_exc_plus()
                    else:
                        if (ExceptionVarDump.wantVariableDump and
                            ExceptionVarDump.dumpOnExceptionInit):
                            ExceptionVarDump._varDump__print(e)
                        raise
                except:
                    if self.extendedExceptions:
                        self.stop()
                        print_exc_plus()
                    else:
                        raise

    def _unpackIOError(self, ioError):
        # IOError unpack from http://www.python.org/doc/essays/stdexceptions/
        # this needs to be in its own method, exceptions that occur inside
        # a nested try block are not caught by the inner try block's except
        try:
            (code, message) = ioError
        except:
            code = 0
            message = ioError
        return code, message

    def stop(self):
        # Set a flag so we will stop before beginning next frame
        self.running = 0

    def __tryReplaceTaskMethod(self, task, oldMethod, newFunction):
        if (task is None) or task._removed:
            return 0
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
            import new
            newMethod = new.instancemethod(newFunction,
                                           method.im_self,
                                           method.im_class)
            task.__call__ = newMethod
            # Found a match
            return 1
        return 0

    def replaceMethod(self, oldMethod, newFunction):
        numFound = 0
        # Look through the regular tasks
        for taskPriList in self.taskList:
            for task in taskPriList:
                if task:
                    numFound += self.__tryReplaceTaskMethod(task, oldMethod, newFunction)
        # Look through the pending tasks
        for pri, taskList in self.pendingTaskDict.items():
            for task in taskList:
                if task:
                    numFound += self.__tryReplaceTaskMethod(task, oldMethod, newFunction)
        # Look through the doLaters
        for task in self.__doLaterList:
            if task:
                numFound += self.__tryReplaceTaskMethod(task, oldMethod, newFunction)
        return numFound

    def __repr__(self):
        taskNameWidth = 32
        dtWidth = 10
        sortWidth = 10
        totalDt = 0
        totalAvgDt = 0
        str = "The taskMgr is handling:\n"
        str += ('taskList'.ljust(taskNameWidth)
               + 'dt(ms)'.rjust(dtWidth)
               + 'avg'.rjust(dtWidth)
               + 'max'.rjust(dtWidth)
               + 'sort'.rjust(sortWidth)
               + '\n')
        str += '-------------------------------------------------------------------------\n'
        dtfmt = '%%%d.2f' % (dtWidth)
        for taskPriList in self.taskList:
            sort = `taskPriList._sort`
            for task in taskPriList:
                if task is None:
                    break
                if task._removed:
                    taskName = '(R)' + task.name
                else:
                    taskName = task.name
                if self.taskTimerVerbose:
                    totalDt = totalDt + task.dt
                    totalAvgDt = totalAvgDt + task.avgDt
                    str += (taskName.ljust(taskNameWidth)
                            + dtfmt % (task.dt*1000)
                            + dtfmt % (task.avgDt*1000)
                            + dtfmt % (task.maxDt*1000)
                            + sort.rjust(sortWidth)
                            + '\n')
                else:
                    str += (task.name.ljust(taskNameWidth)
                            + '----'.rjust(dtWidth)
                            + '----'.rjust(dtWidth)
                            + '----'.rjust(dtWidth)
                            + sort.rjust(sortWidth)
                            + '\n')
        str += '-------------------------------------------------------------------------\n'
        str += 'pendingTasks\n'
        str += '-------------------------------------------------------------------------\n'
        for pri, taskList in self.pendingTaskDict.items():
            for task in taskList:
                if task._removed:
                    taskName = '(PR)' + task.name
                else:
                    taskName = '(P)' + task.name
                if (self.taskTimerVerbose):
                    str += ('  ' + taskName.ljust(taskNameWidth-2)
                            +  dtfmt % (pri)
                            + '\n')
                else:
                    str += ('  ' + taskName.ljust(taskNameWidth-2)
                            +  '----'.rjust(dtWidth)
                            + '\n')
        str += '-------------------------------------------------------------------------\n'
        if (self.taskTimerVerbose):
            str += ('total'.ljust(taskNameWidth)
                    + dtfmt % (totalDt*1000)
                    + dtfmt % (totalAvgDt*1000)
                    + '\n')
        else:
            str += ('total'.ljust(taskNameWidth)
                    + '----'.rjust(dtWidth)
                    + '----'.rjust(dtWidth)
                    + '\n')
        str += '-------------------------------------------------------------------------\n'
        str += ('doLaterList'.ljust(taskNameWidth)
               + 'waitTime(s)'.rjust(dtWidth)
               + '\n')
        str += '-------------------------------------------------------------------------\n'
        # When we print, show the doLaterList in actual sorted order.
        # The sort heap is not actually in order - it is a tree
        # Make a shallow copy so we can sort it
        sortedDoLaterList = self.__doLaterList[:]
        sortedDoLaterList.sort(lambda a, b: cmp(a.wakeTime, b.wakeTime))
        sortedDoLaterList.reverse()
        for task in sortedDoLaterList:
            remainingTime = ((task.wakeTime) - self.currentTime)
            if task._removed:
                taskName = '(R)' + task.name
            else:
                taskName = task.name
            str += ('  ' + taskName.ljust(taskNameWidth-2)
                    +  dtfmt % (remainingTime)
                    + '\n')
        str += '-------------------------------------------------------------------------\n'
        str += "End of taskMgr info\n"
        return str

    def getTasks(self):
        # returns list of all tasks in arbitrary order
        tasks = []
        for taskPriList in self.taskList:
            for task in taskPriList:
                if task is not None and not task._removed:
                    tasks.append(task)
        for pri, taskList in self.pendingTaskDict.iteritems():
            for task in taskList:
                if not task._removed:
                    tasks.append(task)
        return tasks

    def getDoLaters(self):
        # returns list of all doLaters in arbitrary order
        return [doLater for doLater in self.__doLaterList
                if not doLater._removed]

    def resetStats(self):
        # WARNING: this screws up your do-later timings
        if self.taskTimerVerbose:
            for task in self.taskList:
                task.dt = 0
                task.avgDt = 0
                task.maxDt = 0
                task.runningTotal = 0
                task.setStartTimeFrame(self.currentTime, self.currentFrame)

    def popupControls(self):
        from direct.tkpanels import TaskManagerPanel
        return TaskManagerPanel.TaskManagerPanel(self)

    def __getTimeFrame(self):
        # WARNING: If you are testing tasks without an igLoop,
        # you must manually tick the clock
        # Ask for the time last frame
        if self.globalClock:
            return self.globalClock.getFrameTime(), self.globalClock.getFrameCount()
        # OK, we don't have a globalClock yet.  This is therefore
        # running before the first frame.
        return self.trueClock.getShortTime(), 0

    def __getTime(self):
        if self.globalClock:
            return self.globalClock.getFrameTime()
        return self.trueClock.getShortTime()

    if __debug__:
        # to catch memory leaks during the tests at the bottom of the file
        def _startTrackingMemLeaks(self):
            self._memUsage = ScratchPad()
            mu = self._memUsage
            mu.lenTaskList = len(self.taskList)
            mu.lenPendingTaskDict = len(self.pendingTaskDict)
            mu.lenDoLaterList = len(self.__doLaterList)
            mu.lenNameDict = len(self.nameDict)

        def _stopTrackingMemLeaks(self):
            self._memUsage.destroy()
            del self._memUsage

        def _checkMemLeaks(self):
            # flush removed doLaters
            self.__doLaterFilter()
            # give the mgr a chance to clear out removed tasks
            self.step()
            mu = self._memUsage
            # the task list looks like it grows and never shrinks, replacing finished
            # tasks with 'None' in the TaskSortLists.
            # TODO: look at reducing memory usage here--clear out excess at the end of every frame?
            #assert mu.lenTaskList == len(self.taskList)
            assert mu.lenPendingTaskDict == len(self.pendingTaskDict)
            assert mu.lenDoLaterList == len(self.__doLaterList)
            assert mu.lenNameDict == len(self.nameDict)

    def startOsd(self):
        self.add(self.doOsd, 'taskMgr.doOsd')
        self._osdEnabled = None
    def osdEnabled(self):
        return hasattr(self, '_osdEnabled')
    def stopOsd(self):
        onScreenDebug.removeAllWithPrefix(TaskManager.OsdPrefix)
        self.remove('taskMgr.doOsd')
        del self._osdEnabled
    def doOsd(self, task):
        if not onScreenDebug.enabled:
            return
        prefix = TaskManager.OsdPrefix
        onScreenDebug.removeAllWithPrefix(prefix)
        taskNameWidth = 32
        dtWidth = 10
        sortWidth = 10
        totalDt = 0
        totalAvgDt = 0
        i = 0
        onScreenDebug.add(
            ('%s%02i.taskList' % (prefix, i)).ljust(taskNameWidth),
            '%s %s %s %s' % (
            'dt(ms)'.rjust(dtWidth),
            'avg'.rjust(dtWidth),
            'max'.rjust(dtWidth),
            'sort'.rjust(sortWidth),))
        i += 1
        for taskPriList in self.taskList:
            sort = `taskPriList._sort`
            for task in taskPriList:
                if task is None:
                    break
                if task._removed:
                    taskName = '(R)' + task.name
                else:
                    taskName = task.name
                totalDt = totalDt + task.dt
                totalAvgDt = totalAvgDt + task.avgDt
                onScreenDebug.add(
                    ('%s%02i.%s' % (prefix, i, task.name)).ljust(taskNameWidth),
                    '%s %s %s %s' % (
                    dtfmt % (task.dt*1000),
                    dtfmt % (task.avgDt*1000),
                    dtfmt % (task.maxDt*1000),
                    sort.rjust(sortWidth)))
                i += 1
        onScreenDebug.add(('%s%02i.total' % (prefix, i)).ljust(taskNameWidth),
                          '%s %s' % (
            dtfmt % (totalDt*1000),
            dtfmt % (totalAvgDt*1000),))
        return cont

    def _runTests(self):
        if __debug__:
            tm = TaskManager()
            # looks like nothing runs on the first frame...?
            # step to get past the first frame
            tm.step()

            # check for memory leaks after every test
            tm._startTrackingMemLeaks()
            tm._checkMemLeaks()

            # run-once task
            l = []
            def _testDone(task, l=l):
                l.append(None)
                return task.done
            tm.add(_testDone, 'testDone')
            tm.step()
            assert len(l) == 1
            tm.step()
            assert len(l) == 1
            _testDone = None
            tm._checkMemLeaks()

            # remove by name
            def _testRemoveByName(task):
                return task.done
            tm.add(_testRemoveByName, 'testRemoveByName')
            assert tm.remove('testRemoveByName') == 1
            assert tm.remove('testRemoveByName') == 0
            _testRemoveByName = None
            tm._checkMemLeaks()

            # duplicate named tasks
            def _testDupNamedTasks(task):
                return task.done
            tm.add(_testDupNamedTasks, 'testDupNamedTasks')
            tm.add(_testDupNamedTasks, 'testDupNamedTasks')
            assert tm.remove('testRemoveByName') == 0
            _testDupNamedTasks = None
            tm._checkMemLeaks()

            # continued task
            l = []
            def _testCont(task, l = l):
                l.append(None)
                return task.cont
            tm.add(_testCont, 'testCont')
            tm.step()
            assert len(l) == 1
            tm.step()
            assert len(l) == 2
            tm.remove('testCont')
            _testCont = None
            tm._checkMemLeaks()

            # continue until done task
            l = []
            def _testContDone(task, l = l):
                l.append(None)
                if len(l) >= 2:
                    return task.done
                else:
                    return task.cont
            tm.add(_testContDone, 'testContDone')
            tm.step()
            assert len(l) == 1
            tm.step()
            assert len(l) == 2
            tm.step()
            assert len(l) == 2
            assert not tm.hasTaskNamed('testContDone')
            _testContDone = None
            tm._checkMemLeaks()

            # hasTaskNamed
            def _testHasTaskNamed(task):
                return task.done
            tm.add(_testHasTaskNamed, 'testHasTaskNamed')
            assert tm.hasTaskNamed('testHasTaskNamed')
            tm.step()
            assert not tm.hasTaskNamed('testHasTaskNamed')
            _testHasTaskNamed = None
            tm._checkMemLeaks()

            # task sort
            l = []
            def _testPri1(task, l = l):
                l.append(1)
                return task.cont
            def _testPri2(task, l = l):
                l.append(2)
                return task.cont
            tm.add(_testPri1, 'testPri1', sort = 1)
            tm.add(_testPri2, 'testPri2', sort = 2)
            tm.step()
            assert len(l) == 2
            assert l == [1, 2,]
            tm.step()
            assert len(l) == 4
            assert l == [1, 2, 1, 2,]
            tm.remove('testPri1')
            tm.remove('testPri2')
            _testPri1 = None
            _testPri2 = None
            tm._checkMemLeaks()

            # task extraArgs
            l = []
            def _testExtraArgs(arg1, arg2, l=l):
                l.extend([arg1, arg2,])
                return done
            tm.add(_testExtraArgs, 'testExtraArgs', extraArgs=[4,5])
            tm.step()
            assert len(l) == 2
            assert l == [4, 5,]
            _testExtraArgs = None
            tm._checkMemLeaks()

            # task appendTask
            l = []
            def _testAppendTask(arg1, arg2, task, l=l):
                l.extend([arg1, arg2,])
                return task.done
            tm.add(_testAppendTask, '_testAppendTask', extraArgs=[4,5], appendTask=True)
            tm.step()
            assert len(l) == 2
            assert l == [4, 5,]
            _testAppendTask = None
            tm._checkMemLeaks()

            # task uponDeath
            l = []
            def _uponDeathFunc(task, l=l):
                l.append(task.name)
            def _testUponDeath(task):
                return done
            tm.add(_testUponDeath, 'testUponDeath', uponDeath=_uponDeathFunc)
            tm.step()
            assert len(l) == 1
            assert l == ['testUponDeath']
            _testUponDeath = None
            _uponDeathFunc = None
            tm._checkMemLeaks()

            # task owner
            class _TaskOwner:
                def _addTask(self, task):
                    self.addedTaskName = task.name
                def _clearTask(self, task):
                    self.clearedTaskName = task.name
            to = _TaskOwner()
            l = []
            def _testOwner(task):
                return done
            tm.add(_testOwner, 'testOwner', owner=to)
            tm.step()
            assert getattr(to, 'addedTaskName', None) == 'testOwner'
            assert getattr(to, 'clearedTaskName', None) == 'testOwner'
            _testOwner = None
            del to
            _TaskOwner = None
            tm._checkMemLeaks()


            doLaterTests = [0,]

            # doLater
            l = []
            def _testDoLater1(task, l=l):
                l.append(1)
            def _testDoLater2(task, l=l):
                l.append(2)
            def _monitorDoLater(task, tm=tm, l=l, doLaterTests=doLaterTests):
                if task.time > .03:
                    assert l == [1, 2,]
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLater1, 'testDoLater1')
            tm.doMethodLater(.02, _testDoLater2, 'testDoLater2')
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLater, 'monitorDoLater', sort=10)
            _testDoLater1 = None
            _testDoLater2 = None
            _monitorDoLater = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # doLater sort
            l = []
            def _testDoLaterPri1(task, l=l):
                l.append(1)
            def _testDoLaterPri2(task, l=l):
                l.append(2)
            def _monitorDoLaterPri(task, tm=tm, l=l, doLaterTests=doLaterTests):
                if task.time > .02:
                    assert l == [1, 2,]
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLaterPri1, 'testDoLaterPri1', sort=1)
            tm.doMethodLater(.01, _testDoLaterPri2, 'testDoLaterPri2', sort=2)
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLaterPri, 'monitorDoLaterPri', sort=10)
            _testDoLaterPri1 = None
            _testDoLaterPri2 = None
            _monitorDoLaterPri = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # doLater extraArgs
            l = []
            def _testDoLaterExtraArgs(arg1, l=l):
                l.append(arg1)
            def _monitorDoLaterExtraArgs(task, tm=tm, l=l, doLaterTests=doLaterTests):
                if task.time > .02:
                    assert l == [3,]
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLaterExtraArgs, 'testDoLaterExtraArgs', extraArgs=[3,])
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLaterExtraArgs, 'monitorDoLaterExtraArgs', sort=10)
            _testDoLaterExtraArgs = None
            _monitorDoLaterExtraArgs = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # doLater appendTask
            l = []
            def _testDoLaterAppendTask(arg1, task, l=l):
                assert task.name == 'testDoLaterAppendTask'
                l.append(arg1)
            def _monitorDoLaterAppendTask(task, tm=tm, l=l, doLaterTests=doLaterTests):
                if task.time > .02:
                    assert l == [4,]
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLaterAppendTask, 'testDoLaterAppendTask',
                             extraArgs=[4,], appendTask=True)
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLaterAppendTask, 'monitorDoLaterAppendTask', sort=10)
            _testDoLaterAppendTask = None
            _monitorDoLaterAppendTask = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # doLater uponDeath
            l = []
            def _testUponDeathFunc(task, l=l):
                assert task.name == 'testDoLaterUponDeath'
                l.append(10)
            def _testDoLaterUponDeath(arg1, l=l):
                return done
            def _monitorDoLaterUponDeath(task, tm=tm, l=l, doLaterTests=doLaterTests):
                if task.time > .02:
                    assert l == [10,]
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLaterUponDeath, 'testDoLaterUponDeath',
                             uponDeath=_testUponDeathFunc)
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLaterUponDeath, 'monitorDoLaterUponDeath', sort=10)
            _testUponDeathFunc = None
            _testDoLaterUponDeath = None
            _monitorDoLaterUponDeath = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # doLater owner
            class _DoLaterOwner:
                def _addTask(self, task):
                    self.addedTaskName = task.name
                def _clearTask(self, task):
                    self.clearedTaskName = task.name
            doLaterOwner = _DoLaterOwner()
            l = []
            def _testDoLaterOwner(l=l):
                pass
            def _monitorDoLaterOwner(task, tm=tm, l=l, doLaterOwner=doLaterOwner,
                                     doLaterTests=doLaterTests):
                if task.time > .02:
                    assert getattr(doLaterOwner, 'addedTaskName', None) == 'testDoLaterOwner'
                    assert getattr(doLaterOwner, 'clearedTaskName', None) == 'testDoLaterOwner'
                    doLaterTests[0] -= 1
                    return task.done
                return task.cont
            tm.doMethodLater(.01, _testDoLaterOwner, 'testDoLaterOwner',
                             owner=doLaterOwner)
            doLaterTests[0] += 1
            # make sure we run this task after the doLaters if they all occur on the same frame
            tm.add(_monitorDoLaterOwner, 'monitorDoLaterOwner', sort=10)
            _testDoLaterOwner = None
            _monitorDoLaterOwner = None
            del doLaterOwner
            _DoLaterOwner = None
            # don't check until all the doLaters are finished
            #tm._checkMemLeaks()

            # run the doLater tests
            while doLaterTests[0] > 0:
                tm.step()
            del doLaterTests
            tm._checkMemLeaks()

            # getTasks
            def _testGetTasks(task):
                return task.cont
            # the doLaterProcessor is always running
            assert len(tm.getTasks()) == 1
            tm.add(_testGetTasks, 'testGetTasks1')
            assert len(tm.getTasks()) == 2
            assert (tm.getTasks()[0].name == 'testGetTasks1' or
                    tm.getTasks()[1].name == 'testGetTasks1')
            tm.add(_testGetTasks, 'testGetTasks2')
            tm.add(_testGetTasks, 'testGetTasks3')
            assert len(tm.getTasks()) == 4
            tm.remove('testGetTasks2')
            assert len(tm.getTasks()) == 3
            tm.remove('testGetTasks1')
            tm.remove('testGetTasks3')
            assert len(tm.getTasks()) == 1
            _testGetTasks = None
            tm._checkMemLeaks()

            # getDoLaters
            def _testGetDoLaters():
                pass
            # the doLaterProcessor is always running
            assert len(tm.getDoLaters()) == 0
            tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater1')
            assert len(tm.getDoLaters()) == 1
            assert tm.getDoLaters()[0].name == 'testDoLater1'
            tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater2')
            tm.doMethodLater(.1, _testGetDoLaters, 'testDoLater3')
            assert len(tm.getDoLaters()) == 3
            tm.remove('testDoLater2')
            assert len(tm.getDoLaters()) == 2
            tm.remove('testDoLater1')
            tm.remove('testDoLater3')
            assert len(tm.getDoLaters()) == 0
            _testGetDoLaters = None
            tm._checkMemLeaks()

            # duplicate named doLaters removed via taskMgr.remove
            def _testDupNameDoLaters():
                pass
            # the doLaterProcessor is always running
            tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
            tm.doMethodLater(.1, _testDupNameDoLaters, 'testDupNameDoLater')
            assert len(tm.getDoLaters()) == 2
            tm.remove('testDupNameDoLater')
            assert len(tm.getDoLaters()) == 0
            _testDupNameDoLaters = None
            tm._checkMemLeaks()

            # duplicate named doLaters removed via remove()
            def _testDupNameDoLatersRemove():
                pass
            # the doLaterProcessor is always running
            dl1 = tm.doMethodLater(.1, _testDupNameDoLatersRemove, 'testDupNameDoLaterRemove')
            dl2 = tm.doMethodLater(.1, _testDupNameDoLatersRemove, 'testDupNameDoLaterRemove')
            assert len(tm.getDoLaters()) == 2
            dl2.remove()
            assert len(tm.getDoLaters()) == 1
            dl1.remove()
            assert len(tm.getDoLaters()) == 0
            _testDupNameDoLatersRemove = None
            # nameDict etc. isn't cleared out right away with task.remove()
            tm._checkMemLeaks()

            # getTasksNamed
            def _testGetTasksNamed(task):
                return task.cont
            assert len(tm.getTasksNamed('testGetTasksNamed')) == 0
            tm.add(_testGetTasksNamed, 'testGetTasksNamed')
            assert len(tm.getTasksNamed('testGetTasksNamed')) == 1
            assert tm.getTasksNamed('testGetTasksNamed')[0].name == 'testGetTasksNamed'
            tm.add(_testGetTasksNamed, 'testGetTasksNamed')
            tm.add(_testGetTasksNamed, 'testGetTasksNamed')
            assert len(tm.getTasksNamed('testGetTasksNamed')) == 3
            tm.remove('testGetTasksNamed')
            assert len(tm.getTasksNamed('testGetTasksNamed')) == 0
            _testGetTasksNamed = None
            tm._checkMemLeaks()

            # removeTasksMatching
            def _testRemoveTasksMatching(task):
                return task.cont
            tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching')
            assert len(tm.getTasksNamed('testRemoveTasksMatching')) == 1
            tm.removeTasksMatching('testRemoveTasksMatching')
            assert len(tm.getTasksNamed('testRemoveTasksMatching')) == 0
            tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching1')
            tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching2')
            assert len(tm.getTasksNamed('testRemoveTasksMatching1')) == 1
            assert len(tm.getTasksNamed('testRemoveTasksMatching2')) == 1
            tm.removeTasksMatching('testRemoveTasksMatching*')
            assert len(tm.getTasksNamed('testRemoveTasksMatching1')) == 0
            assert len(tm.getTasksNamed('testRemoveTasksMatching2')) == 0
            tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching1a')
            tm.add(_testRemoveTasksMatching, 'testRemoveTasksMatching2a')
            assert len(tm.getTasksNamed('testRemoveTasksMatching1a')) == 1
            assert len(tm.getTasksNamed('testRemoveTasksMatching2a')) == 1
            tm.removeTasksMatching('testRemoveTasksMatching?a')
            assert len(tm.getTasksNamed('testRemoveTasksMatching1a')) == 0
            assert len(tm.getTasksNamed('testRemoveTasksMatching2a')) == 0
            _testRemoveTasksMatching = None
            tm._checkMemLeaks()

            # create Task object and add to mgr
            l = []
            def _testTaskObj(task, l=l):
                l.append(None)
                return task.cont
            t = Task(_testTaskObj)
            tm.add(t, 'testTaskObj')
            tm.step()
            assert len(l) == 1
            tm.step()
            assert len(l) == 2
            tm.remove('testTaskObj')
            tm.step()
            assert len(l) == 2
            _testTaskObj = None
            tm._checkMemLeaks()

            # remove Task via task.remove()
            l = []
            def _testTaskObjRemove(task, l=l):
                l.append(None)
                return task.cont
            t = Task(_testTaskObjRemove)
            tm.add(t, 'testTaskObjRemove')
            tm.step()
            assert len(l) == 1
            tm.step()
            assert len(l) == 2
            t.remove()
            tm.step()
            assert len(l) == 2
            del t
            _testTaskObjRemove = None
            tm._checkMemLeaks()

            """
            # this test fails, and it's not clear what the correct behavior should be.
            # sort passed to Task.__init__ is always overridden by taskMgr.add()
            # even if no sort is specified, and calling Task.setSort() has no
            # effect on the taskMgr's behavior.
            # set/get Task sort
            l = []
            def _testTaskObjSort(arg, task, l=l):
                l.append(arg)
                return task.cont
            t1 = Task(_testTaskObjSort, sort=1)
            t2 = Task(_testTaskObjSort, sort=2)
            tm.add(t1, 'testTaskObjSort1', extraArgs=['a',], appendTask=True)
            tm.add(t2, 'testTaskObjSort2', extraArgs=['b',], appendTask=True)
            tm.step()
            assert len(l) == 2
            assert l == ['a', 'b']
            assert t1.getSort() == 1
            assert t2.getSort() == 2
            t1.setSort(3)
            assert t1.getSort() == 3
            tm.step()
            assert len(l) == 4
            assert l == ['a', 'b', 'b', 'a',]
            t1.remove()
            t2.remove()
            tm.step()
            assert len(l) == 4
            del t1
            del t2
            _testTaskObjSort = None
            tm._checkMemLeaks()
            """

            del l
            tm.destroy()
            del tm


# These constants are moved to the top level of the module,
# to make it easier for legacy code.  In general though, putting
# constants at the top level of a module is deprecated.

exit  = Task.exit
done  = Task.done
cont  = Task.cont
again = Task.again


if __debug__:
    pass # 'if __debug__' is hint for CVS diff output

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

