""" This module defines a Python-level wrapper around the C++
:class:`~panda3d.core.AsyncTaskManager` interface.  It replaces the old
full-Python implementation of the Task system.

For more information about the task system, consult the
:ref:`tasks-and-event-handling` page in the programming manual.
"""

from __future__ import annotations

__all__ = ['Task', 'TaskManager',
           'cont', 'done', 'again', 'pickup', 'exit',
           'sequence', 'loop', 'pause']

from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.showbase.PythonUtil import Functor, ScratchPad
from direct.showbase.MessengerGlobal import messenger
from typing import Any, Callable, Coroutine, Final, Generator, Sequence, TypeVar, Union
import types
import random
import importlib
import sys

# On Android, there's no use handling SIGINT, and in fact we can't, since we
# run the application in a separate thread from the main thread.
signal: types.ModuleType | None
if hasattr(sys, 'getandroidapilevel'):
    signal = None
else:
    try:
        import _signal as signal  # type: ignore[import-not-found, no-redef]
    except ImportError:
        signal = None

from panda3d.core import (
    AsyncTask,
    AsyncTaskPause,
    AsyncTaskManager,
    AsyncTaskSequence,
    ClockObject,
    ConfigVariableBool,
    GlobPattern,
    PythonTask,
    Thread,
)
from direct.extensions_native import HTTPChannel_extensions # pylint: disable=unused-import

# The following variables are typing constructs used in annotations
# to succinctly express all the types that can be converted into tasks.
_T = TypeVar('_T', covariant=True)
_TaskCoroutine = Union[Coroutine[Any, None, _T], Generator[Any, None, _T]]
_TaskFunction = Callable[..., Union[int, _TaskCoroutine[Union[int, None]], None]]
_FuncOrTask = Union[_TaskFunction, _TaskCoroutine[Any], AsyncTask]


def print_exc_plus() -> None:
    """
    Print the usual traceback information, followed by a listing of all the
    local variables in each frame.
    """
    import traceback

    tb = sys.exc_info()[2]
    assert tb is not None
    while 1:
        if not tb.tb_next:
            break
        tb = tb.tb_next
    stack = []
    f: types.FrameType | None = tb.tb_frame
    while f:
        stack.append(f)
        f = f.f_back
    stack.reverse()
    traceback.print_exc()
    print("Locals by frame, innermost last")
    for frame in stack:
        print("")
        print("Frame %s in %s at line %s" % (frame.f_code.co_name,
                                             frame.f_code.co_filename,
                                             frame.f_lineno))
        for key, value in list(frame.f_locals.items()):
            #We have to be careful not to cause a new error in our error
            #printer! Calling str() on an unknown object could cause an
            #error we don't want.
            try:
                valueStr = str(value)
            except Exception:
                valueStr = "<ERROR WHILE PRINTING VALUE>"
            print("\t%20s = %s" % (key, valueStr))


# For historical purposes, we remap the C++-defined enumeration to
# these Python names, and define them both at the module level, here,
# and at the class level (below).  The preferred access is via the
# class level.
done: Final = AsyncTask.DSDone
cont: Final = AsyncTask.DSCont
again: Final = AsyncTask.DSAgain
pickup: Final = AsyncTask.DSPickup
exit: Final = AsyncTask.DSExit

#: Task aliases to :class:`panda3d.core.PythonTask` for historical purposes.
Task = PythonTask

# Copy the module-level enums above into the class level.  This funny
# syntax is necessary because it's a C++-wrapped extension type, not a
# true Python class.
# We can't override 'done', which is already a known method.  We have a
# special check in PythonTask for when the method is being returned.
#Task.DtoolClassDict['done'] = done
Task.DtoolClassDict['cont'] = cont
Task.DtoolClassDict['again'] = again
Task.DtoolClassDict['pickup'] = pickup
Task.DtoolClassDict['exit'] = exit

# Alias the AsyncTaskPause constructor as Task.pause().
pause = AsyncTaskPause
Task.DtoolClassDict['pause'] = staticmethod(pause)

gather = Task.gather
shield = Task.shield


def sequence(*taskList: AsyncTask) -> AsyncTaskSequence:
    seq = AsyncTaskSequence('sequence')
    for task in taskList:
        seq.addTask(task)
    return seq


Task.DtoolClassDict['sequence'] = staticmethod(sequence)


def loop(*taskList: AsyncTask) -> AsyncTaskSequence:
    seq = AsyncTaskSequence('loop')
    for task in taskList:
        seq.addTask(task)
    seq.setRepeatCount(-1)
    return seq


Task.DtoolClassDict['loop'] = staticmethod(loop)


class TaskManager:
    notify = directNotify.newCategory("TaskManager")

    taskTimerVerbose = ConfigVariableBool('task-timer-verbose', False)
    extendedExceptions = ConfigVariableBool('extended-exceptions', False)
    pStatsTasks = ConfigVariableBool('pstats-tasks', False)

    MaxEpochSpeed = 1.0/30.0

    __prevHandler: Any

    def __init__(self) -> None:
        self.mgr = AsyncTaskManager.getGlobalPtr()

        self.resumeFunc: Callable[[], object] | None = None
        self.globalClock = self.mgr.getClock()
        self.stepping = False
        self.running = False
        self.destroyed = False
        self.fKeyboardInterrupt = False
        self.interruptCount = 0
        if signal:
            self.__prevHandler = signal.default_int_handler

        self._frameProfileQueue: list[tuple[int, Any, Callable[[], object] | None]] = []

        # this will be set when it's safe to import StateVar
        self._profileFrames: Any = None
        self._frameProfiler = None
        self._profileTasks: Any = None
        self._taskProfiler = None
        self._taskProfileInfo = ScratchPad(
            taskId = None,
            profiled = False,
            session = None,
        )

    def finalInit(self) -> None:
        # This function should be called once during startup, after
        # most things are imported.
        from direct.fsm.StatePush import StateVar
        self._profileTasks = StateVar(False)
        self.setProfileTasks(ConfigVariableBool('profile-task-spikes', 0).getValue())
        self._profileFrames = StateVar(False)
        self.setProfileFrames(ConfigVariableBool('profile-frames', 0).getValue())

    def destroy(self) -> None:
        # This should be safe to call multiple times.
        self.running = False
        self.notify.info("TaskManager.destroy()")
        self.destroyed = True
        self._frameProfileQueue.clear()
        self.mgr.cleanup()

    def __getClock(self) -> ClockObject:
        return self.mgr.getClock()

    def setClock(self, clockObject: ClockObject) -> None:
        self.mgr.setClock(clockObject)
        self.globalClock = clockObject

    clock = property(__getClock, setClock)

    def invokeDefaultHandler(self, signalNumber, stackFrame):
        print('*** allowing mid-frame keyboard interrupt.')
        # Restore default interrupt handler
        if signal:
            signal.signal(signal.SIGINT, self.__prevHandler)
        # and invoke it
        raise KeyboardInterrupt

    def keyboardInterruptHandler(self, signalNumber, stackFrame):
        self.fKeyboardInterrupt = 1
        self.interruptCount += 1
        if self.interruptCount == 1:
            print('* interrupt by keyboard')
        elif self.interruptCount == 2:
            print('** waiting for end of frame before interrupting...')
            # The user must really want to interrupt this process
            # Next time around invoke the default handler
            signal.signal(signal.SIGINT, self.invokeDefaultHandler)

    def getCurrentTask(self) -> AsyncTask | None:
        """ Returns the task currently executing on this thread, or
        None if this is being called outside of the task manager. """

        return Thread.getCurrentThread().getCurrentTask()

    def hasTaskChain(self, chainName: str) -> bool:
        """ Returns true if a task chain with the indicated name has
        already been defined, or false otherwise.  Note that
        setupTaskChain() will implicitly define a task chain if it has
        not already been defined, or modify an existing one if it has,
        so in most cases there is no need to check this method
        first. """

        return self.mgr.findTaskChain(chainName) is not None

    def setupTaskChain(
        self,
        chainName: str,
        numThreads: int | None = None,
        tickClock: bool | None = None,
        threadPriority: int | None = None,
        frameBudget: float | None = None,
        frameSync: bool | None = None,
        timeslicePriority: bool | None = None,
    ) -> None:
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

        frameSync is true to force the task chain to sync to the
        clock.  When this flag is false, the default, the task chain
        will finish all of its tasks and then immediately start from
        the first task again, regardless of the clock frame.  When it
        is true, the task chain will finish all of its tasks and then
        wait for the clock to tick to the next frame before resuming
        the first task.  This only makes sense for threaded tasks
        chains; non-threaded task chains are automatically
        synchronous.

        timeslicePriority is False in the default mode, in which each
        task runs exactly once each frame, round-robin style,
        regardless of the task's priority value; or True to change the
        meaning of priority so that certain tasks are run less often,
        in proportion to their time used and to their priority value.
        See AsyncTaskManager.setTimeslicePriority() for more.
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
        if frameSync is not None:
            chain.setFrameSync(frameSync)
        if timeslicePriority is not None:
            chain.setTimeslicePriority(timeslicePriority)

    def hasTaskNamed(self, taskName: str) -> bool:
        """Returns true if there is at least one task, active or
        sleeping, with the indicated name. """

        return bool(self.mgr.findTask(taskName))

    def getTasksNamed(self, taskName: str) -> list[AsyncTask]:
        """Returns a list of all tasks, active or sleeping, with the
        indicated name. """
        return list(self.mgr.findTasks(taskName))

    def getTasksMatching(self, taskPattern: GlobPattern | str) -> list[AsyncTask]:
        """Returns a list of all tasks, active or sleeping, with a
        name that matches the pattern, which can include standard
        shell globbing characters like \\*, ?, and []. """

        return list(self.mgr.findTasksMatching(GlobPattern(taskPattern)))

    def getAllTasks(self) -> list[AsyncTask]:
        """Returns list of all tasks, active and sleeping, in
        arbitrary order. """
        return list(self.mgr.getTasks())

    def getTasks(self) -> list[AsyncTask]:
        """Returns list of all active tasks in arbitrary order. """
        return list(self.mgr.getActiveTasks())

    def getDoLaters(self) -> list[AsyncTask]:
        """Returns list of all sleeping tasks in arbitrary order. """
        return list(self.mgr.getSleepingTasks())

    def doMethodLater(
        self,
        delayTime: float,
        funcOrTask: _FuncOrTask,
        name: str | None,
        extraArgs: Sequence | None = None,
        sort: int | None = None,
        priority: int | None = None,
        taskChain: str | None = None,
        uponDeath: Callable[[], object] | None = None,
        appendTask: bool = False,
        owner = None,
    ) -> AsyncTask:
        """Adds a task to be performed at some time in the future.
        This is identical to `add()`, except that the specified
        delayTime is applied to the Task object first, which means
        that the task will not begin executing until at least the
        indicated delayTime (in seconds) has elapsed.

        After delayTime has elapsed, the task will become active, and
        will run in the soonest possible frame thereafter.  If you
        wish to specify a task that will run in the next frame, use a
        delayTime of 0.
        """

        if delayTime < 0:
            assert self.notify.warning('doMethodLater: added task: %s with negative delay: %s' % (name, delayTime))

        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask, owner, uponDeath)
        task.setDelay(delayTime)
        self.mgr.add(task)
        return task

    do_method_later = doMethodLater

    def add(
        self,
        funcOrTask: _FuncOrTask,
        name: str | None = None,
        sort: int | None = None,
        extraArgs: Sequence | None = None,
        priority: int | None = None,
        uponDeath: Callable[[], object] | None = None,
        appendTask: bool = False,
        taskChain: str | None = None,
        owner = None,
        delay: float | None = None,
    ) -> AsyncTask:
        """
        Add a new task to the taskMgr.  The task will begin executing
        immediately, or next frame if its sort value has already
        passed this frame.

        Parameters:
            funcOrTask: either an existing Task object (not already
                added to the task manager), or a callable function
                object. If this is a function, a new Task object will be
                created and returned. You may also pass in a coroutine
                object.

            name (str): the name to assign to the Task.  Required,
                unless you are passing in a Task object that already has
                a name.

            extraArgs (list): the list of arguments to pass to the task
                function.  If this is omitted, the list is just the task
                object itself.

            appendTask (bool): If this is true, then the task object
                itself will be appended to the end of the extraArgs list
                before calling the function.

            sort (int): the sort value to assign the task.  The default
                sort is 0.  Within a particular task chain, it is
                guaranteed that the tasks with a lower sort value will
                all run before tasks with a higher sort value run.

            priority (int): the priority at which to run the task.  The
                default priority is 0.  Higher priority tasks are run
                sooner, and/or more often.  For historical purposes, if
                you specify a priority without also specifying a sort,
                the priority value is understood to actually be a sort
                value. (Previously, there was no priority value, only a
                sort value, and it was called "priority".)

            uponDeath (bool): a function to call when the task
                terminates, either because it has run to completion, or
                because it has been explicitly removed.

            taskChain (str): the name of the task chain to assign the
                task to.

            owner: an optional Python object that is declared as the
                "owner" of this task for maintenance purposes.  The
                owner must have two methods:
                ``owner._addTask(self, task)``, which is called when the
                task begins, and ``owner._clearTask(self, task)``, which
                is called when the task terminates.  This is all the
                ownermeans.

            delay: an optional amount of seconds to wait before starting
                the task (equivalent to doMethodLater).

        Returns:
            The new Task object that has been added, or the original
            Task object that was passed in.
        """

        task = self.__setupTask(funcOrTask, name, priority, sort, extraArgs, taskChain, appendTask, owner, uponDeath)
        if delay is not None:
            task.setDelay(delay)
        self.mgr.add(task)
        return task

    def __setupTask(
        self,
        funcOrTask: _FuncOrTask,
        name: str | None,
        priority: int | None,
        sort: int | None,
        extraArgs: Sequence | None,
        taskChain: str | None,
        appendTask: bool,
        owner,
        uponDeath: Callable[[], object] | None,
    ) -> AsyncTask:
        wasTask = False
        if isinstance(funcOrTask, AsyncTask):
            task = funcOrTask
            wasTask = True
        elif hasattr(funcOrTask, '__call__') or \
             hasattr(funcOrTask, 'cr_await') or \
             isinstance(funcOrTask, types.GeneratorType):
            # It's a function, coroutine, or something emulating a coroutine.
            task = PythonTask(funcOrTask)
            if name is None:
                name = getattr(funcOrTask, '__qualname__', None) or \
                       getattr(funcOrTask, '__name__', None)
        else:
            self.notify.error(
                'add: Tried to add a task that was not a Task or a func')

        if hasattr(task, 'setArgs'):
            # It will only accept arguments if it's a PythonTask.
            if extraArgs is None:
                if wasTask:
                    extraArgs = task.getArgs()
                    #do not append the task to an existing task. It was already there
                    #from the last time it was addeed
                    appendTask = False
                else:
                    extraArgs = []
                    appendTask = True
            task.setArgs(extraArgs, appendTask)
        elif extraArgs is not None:
            self.notify.error(
                'Task %s does not accept arguments.' % (repr(task)))

        if name is not None:
            task.setName(name)
        assert task.hasName()

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

        if owner is not None:
            task.setOwner(owner)

        if uponDeath is not None:
            task.setUponDeath(uponDeath)

        return task

    def remove(self, taskOrName: AsyncTask | str | list[AsyncTask | str]) -> int:
        """Removes a task from the task manager.  The task is stopped,
        almost as if it had returned task.done.  (But if the task is
        currently executing, it will finish out its current frame
        before being removed.)  You may specify either an explicit
        Task object, or the name of a task.  If you specify a name,
        all tasks with the indicated name are removed.  Returns the
        number of tasks removed. """

        if isinstance(taskOrName, AsyncTask):
            return self.mgr.remove(taskOrName)
        elif isinstance(taskOrName, list):
            count = 0
            for task in taskOrName:
                count += self.remove(task)
            return count
        else:
            tasks = self.mgr.findTasks(taskOrName)
            return self.mgr.remove(tasks)

    def removeTasksMatching(self, taskPattern: GlobPattern | str) -> int:
        """Removes all tasks whose names match the pattern, which can
        include standard shell globbing characters like \\*, ?, and [].
        See also :meth:`remove()`.

        Returns the number of tasks removed.
        """
        tasks = self.mgr.findTasksMatching(GlobPattern(taskPattern))
        return self.mgr.remove(tasks)

    def step(self) -> None:
        """Invokes the task manager for one frame, and then returns.
        Normally, this executes each task exactly once, though task
        chains that are in sub-threads or that have frame budgets
        might execute their tasks differently. """

        startFrameTime = self.globalClock.getRealTime()

        # Replace keyboard interrupt handler during task list processing
        # so we catch the keyboard interrupt but don't handle it until
        # after task list processing is complete.
        self.fKeyboardInterrupt = False
        self.interruptCount = 0

        if signal:
            self.__prevHandler = signal.signal(signal.SIGINT, self.keyboardInterruptHandler)

        try:
            self.mgr.poll()

            # This is the spot for an internal yield function
            nextTaskTime = self.mgr.getNextWakeTime()
            self.doYield(startFrameTime, nextTaskTime)

        finally:
            # Restore previous interrupt handler
            if signal:
                signal.signal(signal.SIGINT, self.__prevHandler)
                self.__prevHandler = signal.default_int_handler

        if self.fKeyboardInterrupt:
            raise KeyboardInterrupt

    def run(self) -> None:
        """Starts the task manager running.  Does not return until an
        exception is encountered (including KeyboardInterrupt). """

        if sys.platform == 'emscripten':
            return

        # Set the clock to have last frame's time in case we were
        # Paused at the prompt for a long time
        t = self.globalClock.getFrameTime()
        timeDelta = t - self.globalClock.getRealTime()
        self.globalClock.setRealTime(t)
        messenger.send("resetClock", [timeDelta])

        if self.resumeFunc is not None:
            self.resumeFunc()

        if self.stepping:
            self.step()
        else:
            self.running = True
            while self.running:
                try:
                    if len(self._frameProfileQueue) > 0:
                        numFrames, session, callback = self._frameProfileQueue.pop(0)

                        def _profileFunc(numFrames: int = numFrames) -> None:
                            self._doProfiledFrames(numFrames)
                        session.setFunc(_profileFunc)
                        session.run()
                        del _profileFunc
                        if callback:
                            callback()
                        session.release()
                    else:
                        self.step()
                except KeyboardInterrupt:
                    self.stop()
                except SystemExit:
                    self.stop()
                    raise
                except IOError as ioError:
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
                except Exception as e:
                    if self.extendedExceptions:
                        self.stop()
                        print_exc_plus()
                    else:
                        from direct.showbase import ExceptionVarDump
                        if ExceptionVarDump.wantStackDumpLog and \
                           ExceptionVarDump.dumpOnExceptionInit:
                            ExceptionVarDump._varDump__print(e)
                        raise
                except:
                    if self.extendedExceptions:
                        self.stop()
                        print_exc_plus()
                    else:
                        raise

        self.mgr.stopThreads()

    def _unpackIOError(self, ioError):
        # IOError unpack from http://www.python.org/doc/essays/stdexceptions/
        # this needs to be in its own method, exceptions that occur inside
        # a nested try block are not caught by the inner try block's except
        try:
            (code, message) = ioError
        except Exception:
            code = 0
            message = ioError
        return code, message

    def stop(self) -> None:
        # Set a flag so we will stop before beginning next frame
        self.running = False

    def __tryReplaceTaskMethod(self, task, oldMethod, newFunction):
        if not isinstance(task, PythonTask):
            return 0

        method = task.getFunction()
        if isinstance(method, types.MethodType):
            function = method.__func__
        else:
            function = method
        if function == oldMethod:
            newMethod = types.MethodType(newFunction, method.__self__)
            task.setFunction(newMethod)
            # Found a match
            return 1
        return 0

    def replaceMethod(self, oldMethod, newFunction):
        numFound = 0
        for task in self.getAllTasks():
            numFound += self.__tryReplaceTaskMethod(task, oldMethod, newFunction)
        return numFound

    def popupControls(self):
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        TaskManagerPanel = importlib.import_module('direct.tkpanels.TaskManagerPanel')
        return TaskManagerPanel.TaskManagerPanel(self)

    def getProfileSession(self, name=None):
        # call to get a profile session that you can modify before passing to profileFrames()
        if name is None:
            name = 'taskMgrFrameProfile'

        # Defer this import until we need it: some Python
        # distributions don't provide the profile and pstats modules.
        PS = importlib.import_module('direct.showbase.ProfileSession')
        return PS.ProfileSession(name)

    def profileFrames(self, num=None, session=None, callback=None):
        if num is None:
            num = 1
        if session is None:
            session = self.getProfileSession()
        # make sure the profile session doesn't get destroyed before we're done with it
        session.acquire()
        self._frameProfileQueue.append((num, session, callback))

    def _doProfiledFrames(self, numFrames):
        for i in range(numFrames):
            self.step()

    def getProfileFrames(self):
        return self._profileFrames.get()

    def getProfileFramesSV(self):
        return self._profileFrames

    def setProfileFrames(self, profileFrames):
        self._profileFrames.set(profileFrames)
        if (not self._frameProfiler) and profileFrames:
            # import here due to import dependencies
            FP = importlib.import_module('direct.task.FrameProfiler')
            self._frameProfiler = FP.FrameProfiler()

    def getProfileTasks(self):
        return self._profileTasks.get()

    def getProfileTasksSV(self):
        return self._profileTasks

    def setProfileTasks(self, profileTasks):
        self._profileTasks.set(profileTasks)
        if (not self._taskProfiler) and profileTasks:
            # import here due to import dependencies
            TP = importlib.import_module('direct.task.TaskProfiler')
            self._taskProfiler = TP.TaskProfiler()

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
            taskFunc = task.getFunction(),
            taskArgs = task.getArgs(),
            task = task,
            profiled = False,
            session = None,
        )

        # Temporarily replace the task's own function with our
        # _profileTask method.
        task.setFunction(self._profileTask)
        task.setArgs([self._taskProfileInfo], True)

    def _profileTask(self, profileInfo, task):
        # This is called instead of the task function when we have
        # decided to profile a task.

        assert profileInfo.task == task
        # don't profile the same task twice in a row
        assert not profileInfo.profiled

        # Restore the task's proper function for next time.
        appendTask = False
        taskArgs = profileInfo.taskArgs
        if taskArgs and taskArgs[-1] == task:
            appendTask = True
            taskArgs = taskArgs[:-1]
        task.setArgs(taskArgs, appendTask)
        task.setFunction(profileInfo.taskFunc)

        # Defer this import until we need it: some Python
        # distributions don't provide the profile and pstats modules.
        PS = importlib.import_module('direct.showbase.ProfileSession')
        profileSession = PS.ProfileSession('profiled-task-%s' % task.getName(),
                                           Functor(profileInfo.taskFunc, *profileInfo.taskArgs))
        ret = profileSession.run()

        # set these values *after* profiling in case we're profiling the TaskProfiler
        profileInfo.session = profileSession
        profileInfo.profiled = True

        return ret

    def _hasProfiledDesignatedTask(self):
        # have we run a profile of the designated task yet?
        return self._taskProfileInfo.profiled

    def _getLastTaskProfileSession(self):
        return self._taskProfileInfo.session

    def _getRandomTask(self):
        # Figure out when the next frame is likely to expire, so we
        # won't grab any tasks that are sleeping for a long time.
        now = self.globalClock.getFrameTime()
        avgFrameRate = self.globalClock.getAverageFrameRate()
        if avgFrameRate < .00001:
            avgFrameDur = 0.
        else:
            avgFrameDur = (1. / self.globalClock.getAverageFrameRate())
        next = now + avgFrameDur

        # Now grab a task at random, until we find one that we like.
        tasks = self.mgr.getTasks()
        i = random.randrange(tasks.getNumTasks())
        task = tasks.getTask(i)
        while not isinstance(task, PythonTask) or \
              task.getWakeTime() > next:
            tasks.removeTask(i)
            i = random.randrange(tasks.getNumTasks())
            task = tasks.getTask(i)
        return task

    def __repr__(self) -> str:
        return str(self.mgr)

    # In the event we want to do frame time managment, this is the
    # function to replace or overload.
    def doYield(self, frameStartTime: float, nextScheduledTaskTime: float) -> None:
        pass

    #def doYieldExample(self, frameStartTime, nextScheduledTaskTime):
    #    minFinTime = frameStartTime + self.MaxEpochSpeed
    #    if nextScheduledTaskTime > 0 and nextScheduledTaskTime < minFinTime:
    #        print(' Adjusting Time')
    #        minFinTime = nextScheduledTaskTime
    #    delta = minFinTime - self.globalClock.getRealTime()
    #    while delta > 0.002:
    #        print ' sleep %s'% (delta)
    #        time.sleep(delta)
    #        delta = minFinTime - self.globalClock.getRealTime()


if __debug__:
    def checkLeak():
        import gc
        gc.enable()
        from direct.showbase.DirectObject import DirectObject
        from direct.task.TaskManagerGlobal import taskMgr

        class TestClass(DirectObject):
            def doTask(self, task):
                return task.done
        obj = TestClass()
        startRefCount = sys.getrefcount(obj)
        print('sys.getrefcount(obj): %s' % sys.getrefcount(obj))
        print('** addTask')
        t = obj.addTask(obj.doTask, 'test')
        print('sys.getrefcount(obj): %s' % sys.getrefcount(obj))
        print('task.getRefCount(): %s' % t.getRefCount())
        print('** removeTask')
        obj.removeTask('test')
        print('sys.getrefcount(obj): %s' % sys.getrefcount(obj))
        print('task.getRefCount(): %s' % t.getRefCount())
        print('** step')
        taskMgr.step()
        taskMgr.step()
        taskMgr.step()
        print('sys.getrefcount(obj): %s' % sys.getrefcount(obj))
        print('task.getRefCount(): %s' % t.getRefCount())
        print('** task release')
        t = None
        print('sys.getrefcount(obj): %s' % sys.getrefcount(obj))
        assert sys.getrefcount(obj) == startRefCount
