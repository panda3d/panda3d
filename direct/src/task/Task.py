
from libpandaexpressModules import *
from DirectNotify import *
from PythonUtil import *
import time

exit = -1
done = 0
cont = 1

# Store the global clock
globalClock = ClockObject.getGlobalClock()

def getTimeFrame():
    # WARNING: If you are testing tasks without an igloop,
    # you must manually tick the clock

    # Ask for the time last frame
    t = globalClock.getTime()
    
    # Get the new frame count
    f = globalClock.getFrameCount()

    return t, f


class Task:
    def __init__(self, callback):
        self.__call__ = callback
        self.uponDeath = None
        
    def setStartTimeFrame(self, startTime, startFrame):
        self.starttime = startTime
        self.startframe = startFrame
        
    def setCurrentTimeFrame(self, currentTime, currentFrame):
        # Calculate and store this task's time (relative to when it started)
        self.time = currentTime - self.starttime
        self.frame = currentFrame - self.startframe

def doLater(delayTime, task, taskName):
    task.name = taskName
    # make a sequence out of the delay and the task
    seq = sequence(pause(delayTime), task)
    return seq
    
def pause(delayTime):
    def func(self):
        if (self.time < self.delayTime):
            return cont
        else:
            # Time is up, return done
            TaskManager.notify.debug('pause done: ' + self.name)
            return done
    task = Task(func)
    task.name = 'pause'
    task.delayTime = delayTime
    return task


def release():
    def func(self):
        # A release is immediately done
        TaskManager.notify.debug('release done: ' + self.name)
        return done
    task = Task(func)
    task.name = 'release'
    return task


def sequence(*taskList):
    return make_sequence(taskList)


def make_sequence(taskList):
    def func(self):
        # If we got to the end of the list, this sequence is done
        if (self.index >= len(self.taskList)):
            TaskManager.notify.debug('sequence done: ' + self.name)
            return done
        else:
            task = self.taskList[self.index]
            # If this is a new task, set it's start time and frame
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
                return cont
            
            # If this task is done, increment the index so that next frame
            # we will start executing the next task on the list
            elif (ret == done):
                self.index = self.index + 1
                return cont

            # If this task wants to exit, the sequence exits
            elif (ret == exit):
                return exit

    task = Task(func)
    task.name = 'sequence'
    task.taskList = taskList
    task.prevIndex = -1
    task.index = 0
    return task

def makeSpawner(task, taskName, taskMgr):
    def func(self):
        self.taskMgr.spawnTaskNamed(self.task, self.taskName)
        return done
    newTask = Task(func)
    newTask.name = taskName + "-spawner"
    newTask.task = task
    newTask.taskName = taskName
    newTask.taskMgr = taskMgr
    return newTask


def makeSequenceFromTimeline(timelineList, taskMgr):
    timeline = []
    lastPause = 0
    sortedList = list(timelineList)
    sortedList.sort()
    for triple in sortedList:
        t = triple[0] - lastPause
        lastPause = triple[0]
        task = triple[1]
        taskName = triple[2]
        timeline.append(pause(t))
        timeline.append(makeSpawner(task, taskName, taskMgr))
    return make_sequence(timeline)


def timeline(*timelineList):
    def func(self):
        # Step our sub task manager (returns the number of tasks remaining)
        lenTaskList = self.taskMgr.step()
        
        # The sequence start time is the same as our start time
        self.sequence.time = self.time
        self.sequence.frame = self.frame
        
        if (not self.sequenceDone):
            # Execute the sequence for this frame
            seqRet = self.sequence(self.sequence)
            # See if sequence is done
            if (seqRet == done):
                self.sequenceDone = 1
                # See if the timeline is done
                if (lenTaskList == 0):
                    TaskManager.notify.debug('timeline done: ' + self.name)
                    return done
                else:
                    return cont
            else:
                return cont
        else:
            return cont
    task = Task(func)
    task.name = 'timeline'
    task.taskMgr = TaskManager()
    task.sequence = makeSequenceFromTimeline(timelineList, task.taskMgr)
    task.sequenceDone = 0
    return task


class TaskManager:

    notify = None
    
    def __init__(self):
        self.running = 0
        self.stepping = 0
        self.taskList = []
        self.currentTime, self.currentFrame = getTimeFrame()
        if (TaskManager.notify == None):
            TaskManager.notify = directNotify.newCategory("TaskManager")
        # TaskManager.notify.setDebug(1)

    def stepping(value):
        self.stepping = value

    def spawnMethodNamed(self, func, name):
        task = Task(func)
        self.spawnTaskNamed(task, name)
        
    def spawnTaskNamed(self, task, name):
        TaskManager.notify.debug('spawning task named: ' + name)
        task.name = name
        task.setStartTimeFrame(self.currentTime, self.currentFrame)
        self.taskList.append(task)
        return task

    def removeAllTasks(self):
        # Make a shallow copy so we do not modify the list in place
        taskListCopy = self.taskList[:]
        for task in taskListCopy:
            self.removeTask(task)

    def removeTask(self, task):
        TaskManager.notify.debug('removing task: ' + `task`)
        try:
            self.taskList.remove(task)
        except:
            pass
        # TODO: upon death
        if task.uponDeath:
            task.uponDeath(task)

    def removeTasksNamed(self, taskName):
        removedTasks = []
        TaskManager.notify.debug('removing tasks named: ' + taskName)
        
        # Find the tasks that match by name and make a list of them
        for task in self.taskList:
            if (task.name == taskName):
                removedTasks.append(task)

        # Now iterate through the tasks we need to remove and remove them
        for task in removedTasks:
            self.removeTask(task)

        # Return the number of tasks removed
        return len(removedTasks)

    def step(self):
        TaskManager.notify.debug('step')
        self.currentTime, self.currentFrame = getTimeFrame()
        for task in self.taskList:
            task.setCurrentTimeFrame(self.currentTime, self.currentFrame)
            # Run the task and check the return value
            if task.name == 'test':
                print 'before task'
            ret = task(task)
            if task.name == 'test':
                print 'after  task'
            if (ret == cont):
                continue
            elif (ret == done):
                self.removeTask(task)
            elif (ret == exit):
                self.removeTask(task)
            else:
                raise 'invalid task state'
        return len(self.taskList)

    def run(self):
        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        t = globalClock.getTime()
        globalClock.setTime(t)
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
                    raise

    def stop(self):
        # Set a flag so we will stop before beginning next frame
        self.running = 0

    def __repr__(self):
        str = ''
        str = str + 'taskList\n'
        str = str + '--------------------\n'
        for task in self.taskList:
            str = str + ' ' + task.name + '\n'
        return str


"""
import Task
from ShowBaseGlobal import * # to get taskMgr, and run()

# sequence

def seq1(state):
    print 'seq1'
    return Task.done
def seq2(state):
    print 'seq2'
    return Task.done

t = Task.sequence(Task.pause(1.0), Task.Task(seq1), Task.release(),
                  Task.pause(3.0), Task.Task(seq2))
taskMgr.spawnTaskNamed(t, 'sequence')
run()

# timeline

def keyframe1(state):
    print 'keyframe1'
    return Task.done
def keyframe2(state):
    print 'keyframe2'
    return Task.done
def keyframe3(state):
    print 'keyframe3'
    return Task.done
    
testtl = Task.timeline(
    (0.5, Task.Task(keyframe1), 'key1'),
    (0.6, Task.Task(keyframe2), 'key2'),
    (0.7, Task.Task(keyframe3), 'key3')
    )

t = taskMgr.spawnTaskNamed(testtl, 'timeline')
run()

# do later - returns a sequence

def foo(state):
    print 'later...'
    return Task.done

seq = Task.doLater(3.0, Task.Task(foo), 'fooLater')
t = taskMgr.spawnTaskNamed(seq, 'doLater-fooLater')
run()

# tasks with state
# Combined with uponDeath

someValue = 1

def func(state):
    if (state.someValue > 10):
        print 'true!'
        return Task.done
    else:
        state.someValue = state.someValue + 1
        return Task.cont

def deathFunc(state):
    print 'Value at death: ', state.someValue

task = Task.Task(func)

# set task state here
task.someValue = someValue

# Use instance variable uponDeath to specify function
# to perform on task removal
# Default value of uponDeath is None
task.uponDeath = deathFunc

t = taskMgr.spawnTaskNamed(task, 'funcTask')
run()
"""
