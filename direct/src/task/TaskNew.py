""" This module defines a Python-level wrapper around the C++
AsyncTaskManager interface.  It replaces the old full-Python
implementation of the Task system. """

from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase import ExceptionVarDump
import signal
import types

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

# These constants are moved to the top level of the module,
# to make it easier for legacy code.  In general though, putting
# constants at the top level of a module is deprecated.

done = AsyncTask.DSDone
cont = AsyncTask.DSCont
again = AsyncTask.DSAgain

class Task(PythonTask):

    done = AsyncTask.DSDone
    cont = AsyncTask.DSCont
    again = AsyncTask.DSAgain

    def __init__(self, function, name = ''):
        PythonTask.__init__(self, function, name)

class TaskManager:
    notify = directNotify.newCategory("TaskManager")

    extendedExceptions = False

    def __init__(self):
        self.mgr = AsyncTaskManager('TaskManager')

        self.resumeFunc = None
        self.globalClock = None
        self.stepping = False
        self.running = False
        self.fKeyboardInterrupt = False
        self.interruptCount = 0

    def destroy(self):
        self.mgr.stopThreads()
        self.removeTasksMatching('*')

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
        return bool(self.mgr.findTask(taskName))

    def getTasksNamed(self, taskName):
        return self.__makeTaskList(self.mgr.findTasks(taskName))

    def __makeTaskList(self, taskCollection):
        l = []
        for i in range(taskCollection.getNumTasks()):
            l.append(taskCollection.getTask(i))
        return l

    def doMethodLater(self, delayTime, funcOrTask, name, priority = None,
                      sort = None, extraArgs = None, taskChain = None,
                      appendTask = False):
        if delayTime < 0:
            assert self.notify.warning('doMethodLater: added task: %s with negative delay: %s' % (name, delayTime))

        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask)
        task.setDelay(delayTime)
        self.mgr.add(task)
        return task

    def add(self, funcOrTask, name, priority = None, sort = None,
            extraArgs = None, taskChain = None, appendTask = False):
        
        """
        Add a new task to the taskMgr.
        You can add a Task object or a method that takes one argument.
        """
        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask)
        self.mgr.add(task)
        return task

    def __setupTask(self, funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask):
        if isinstance(funcOrTask, PythonTask):
            task = funcOrTask
        elif callable(funcOrTask):
            task = PythonTask(funcOrTask)
        else:
            self.notify.error(
                'add: Tried to add a task that was not a Task or a func')
        assert isinstance(name, types.StringTypes), 'Name must be a string type'
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

        if taskChain is not None:
            task.setTaskChain(taskChain)

        if extraArgs is None:
            extraArgs = []
            appendTask = True
        task.setArgs(extraArgs, appendTask)

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

        self.mgr.poll()
        
        # Restore default interrupt handler
        signal.signal(signal.SIGINT, signal.default_int_handler)
        if self.fKeyboardInterrupt:
            raise KeyboardInterrupt

    def run(self):
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
