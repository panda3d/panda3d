""" This module defines a Python-level wrapper around the C++
AsyncTaskManager interface.  It replaces the old full-Python
implementation of the Task system. """

__all__ = ['Task', 'TaskManager',
           'exit', 'cont', 'done', 'again', 'restart']

from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase import ExceptionVarDump
import signal
import types
import time

from pandac.PandaModules import *

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

# For historical purposes, we remap the C++-defined enumeration to
# these Python names, and define them both at the module level, here,
# and at the class level (below).  The preferred access is via the
# class level.
done = AsyncTask.DSDone
cont = AsyncTask.DSCont
again = AsyncTask.DSAgain
restart = AsyncTask.DSRestart

# Alias PythonTask to Task for historical purposes.
Task = PythonTask

# Copy the module-level enums above into the class level.  This funny
# syntax is necessary because it's a C++-wrapped extension type, not a
# true Python class.
Task.DtoolClassDict['done'] = done
Task.DtoolClassDict['cont'] = cont
Task.DtoolClassDict['again'] = again
Task.DtoolClassDict['restart'] = restart

class TaskManager:
    notify = directNotify.newCategory("TaskManager")

    extendedExceptions = False
    MaxEpochSpeed = 1.0/30.0

    def __init__(self):
        self.mgr = AsyncTaskManager('TaskManager')

        self.resumeFunc = None
        self.globalClock = self.mgr.getClock()
        self.stepping = False
        self.running = False
        self.fKeyboardInterrupt = False
        self.interruptCount = 0

    def destroy(self):
        self.mgr.cleanup()

    def setClock(self, clockObject):
        self.mgr.setClock(clockObject)
        self.globalClock = clockObject

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

    def setupTaskChain(self, chainName, numThreads = None, tickClock = None,
                       threadPriority = None, frameBudget = None):
        """Defines a new task chain.  Each task chain executes tasks
        potentially in parallel with all of the other task chains (if
        numThreads is more than zero).  When a new task is created, it
        may be associated with any of the task chains, by name (or you
        can move a task to another task chain with
        task.setTaskChain()).  You can have any number of task chains,
        but each must have a unique name.

        numThreads is the number of threads to allocate for this task
        chain.  If it is 1 or more, then the tasks on this task chain
        will execute in parallel with the tasks on other task chains.
        If it is greater than 1, then the tasks on this task chain may
        execute in parallel with themselves (within tasks of the same
        sort value).

        If tickClock is True, then this task chain will be responsible
        for ticking the global clock each frame (and thereby
        incrementing the frame counter).  There should be just one
        task chain responsible for ticking the clock, and usually it
        is the default, unnamed task chain.

        threadPriority specifies the priority level to assign to
        threads on this task chain.  It may be one of TPLow, TPNormal,
        TPHigh, or TPUrgent.  This is passed to the underlying
        threading system to control the way the threads are scheduled.

        frameBudget is the maximum amount of time (in seconds) to
        allow this task chain to run per frame.  Set it to -1 to mean
        no limit (the default).  It's not directly related to
        threadPriority.
        """
        
        chain = self.mgr.makeTaskChain(chainName)
        if numThreads is not None:
            chain.setNumThreads(numThreads)
        if tickClock is not None:
            chain.setTickClock(tickClock)
        if threadPriority is not None:
            chain.setThreadPriority(threadPriority)
        if frameBudget is not None:
            chain.setFrameBudget(frameBudget)

    def hasTaskNamed(self, taskName):
        """Returns true if there is at least one task, active or
        sleeping, with the indicated name. """
        
        return bool(self.mgr.findTask(taskName))

    def getTasksNamed(self, taskName):
        """Returns a list of all tasks, active or sleeping, with the
        indicated name. """
        return self.__makeTaskList(self.mgr.findTasks(taskName))

    def getTasks(self):
        """Returns list of all active tasks in arbitrary order. """
        return self.__makeTaskList(self.mgr.getActiveTasks())

    def getDoLaters(self):
        """Returns list of all sleeping tasks in arbitrary order. """
        return self.__makeTaskList(self.mgr.getSleepingTasks())

    def __makeTaskList(self, taskCollection):
        l = []
        for i in range(taskCollection.getNumTasks()):
            l.append(taskCollection.getTask(i))
        return l

    def doMethodLater(self, delayTime, funcOrTask, name, priority = None,
                      sort = None, extraArgs = None, taskChain = None,
                      appendTask = False, owner = None, uponDeath = None):
        if delayTime < 0:
            assert self.notify.warning('doMethodLater: added task: %s with negative delay: %s' % (name, delayTime))

        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask, owner, uponDeath)
        task.setDelay(delayTime)
        self.mgr.add(task)
        return task

    def add(self, funcOrTask, name, priority = None, sort = None,
            extraArgs = None, taskChain = None, appendTask = False,
            owner = None, uponDeath = None):
        
        """
        Add a new task to the taskMgr.
        You can add a Task object or a method that takes one argument.
        """
        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask, owner, uponDeath)
        self.mgr.add(task)
        return task

    def __setupTask(self, funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask, owner, uponDeath):
        if isinstance(funcOrTask, PythonTask):
            task = funcOrTask
        elif callable(funcOrTask):
            task = PythonTask(funcOrTask)
        else:
            self.notify.error(
                'add: Tried to add a task that was not a Task or a func')
        assert isinstance(name, types.StringTypes), 'Name must be a string type'
        if extraArgs is None:
            extraArgs = []
            appendTask = True
        task.setArgs(extraArgs, appendTask)

        task.setName(name)

        # For historical reasons, if priority is specified but not
        # sort, it really means sort.
        if priority is not None and sort is None:
            task.setSort(priority)
        else:
            if priority is not None:
                task.setPriority(priority)
            if sort is not None:
                task.setSort(sort)

        if extraArgs is None:
            extraArgs = []
            appendTask = True
        task.setArgs(extraArgs, appendTask)

        if taskChain is not None:
            task.setTaskChain(taskChain)

        if owner is not None:
            task.setOwner(owner)

        if uponDeath is not None:
            task.setUponDeath(uponDeath)

        return task
        
    def remove(self, taskOrName):
        if isinstance(taskOrName, types.StringTypes):
            tasks = self.mgr.findTasks(taskOrName)
            return self.mgr.remove(tasks)
        elif isinstance(taskOrName, AsyncTask):
            return self.mgr.remove(taskOrName)
        else:
            self.notify.error('remove takes a string or a Task')

    def removeTasksMatching(self, taskPattern):
        """removeTasksMatching(self, string taskPattern)
        Removes tasks whose names match the pattern, which can include
        standard shell globbing characters like *, ?, and [].
        """
        tasks = self.mgr.findTasksMatching(GlobPattern(taskPattern))
        return self.mgr.remove(tasks)

    def step(self):
        # Replace keyboard interrupt handler during task list processing
        # so we catch the keyboard interrupt but don't handle it until
        # after task list processing is complete.
        self.fKeyboardInterrupt = 0
        self.interruptCount = 0
        signal.signal(signal.SIGINT, self.keyboardInterruptHandler)

        startFrameTime = self.globalClock.getRealTime()

        self.mgr.poll()

        # This is the spot for an internal yield function
        nextTaskTime = self.mgr.getNextWakeTime()
        self.doYield(startFrameTime, nextTaskTime)
        
        # Restore default interrupt handler
        signal.signal(signal.SIGINT, signal.default_int_handler)
        if self.fKeyboardInterrupt:
            raise KeyboardInterrupt

    def run(self):
        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        t = self.globalClock.getFrameTime()
        timeDelta = t - globalClock.getRealTime()
        self.globalClock.setRealTime(t)
        messenger.send("resetClock", [timeDelta])

        if self.resumeFunc != None:
            self.resumeFunc()

        if self.stepping:
            self.step()
        else:
            self.running = True
            while self.running:
                try:
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
        self.running = False

    def __repr__(self):
        return str(self.mgr)

    # In the event we want to do frame time managment, this is the
    # function to replace or overload.
    def doYield(self, frameStartTime, nextScheduledTaskTime):
          None
          
    def doYieldExample(self, frameStartTime, nextScheduledTaskTime):
        minFinTime = frameStartTime + self.MaxEpochSpeed
        if nextScheduledTaskTime > 0 and nextScheduledTaskTime < minFinTime:
            print ' Adjusting Time'
            minFinTime = nextScheduledTaskTime
        delta = minFinTime - self.globalClock.getRealTime()
        while(delta > 0.002):
            print ' sleep %s'% (delta)
            time.sleep(delta)           
            delta = minFinTime - self.globalClock.getRealTime()
    
    if __debug__:
        # to catch memory leaks during the tests at the bottom of the file
        def _startTrackingMemLeaks(self):
            pass

        def _stopTrackingMemLeaks(self):
            pass

        def _checkMemLeaks(self):
            pass

    def _runTests(self):
        if __debug__:
            tm = TaskManager()
            tm.setClock(ClockObject())
            tm.setupTaskChain("default", tickClock = True)

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
            # No doLaterProcessor in the new world.
            assert len(tm.getTasks()) == 0
            tm.add(_testGetTasks, 'testGetTasks1')
            assert len(tm.getTasks()) == 1
            assert (tm.getTasks()[0].name == 'testGetTasks1' or
                    tm.getTasks()[1].name == 'testGetTasks1')
            tm.add(_testGetTasks, 'testGetTasks2')
            tm.add(_testGetTasks, 'testGetTasks3')
            assert len(tm.getTasks()) == 3
            tm.remove('testGetTasks2')
            assert len(tm.getTasks()) == 2
            tm.remove('testGetTasks1')
            tm.remove('testGetTasks3')
            assert len(tm.getTasks()) == 0
            _testGetTasks = None
            tm._checkMemLeaks()

            # getDoLaters
            def _testGetDoLaters():
                pass
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
