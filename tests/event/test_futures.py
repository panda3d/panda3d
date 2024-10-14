from panda3d import core
from asyncio.exceptions import TimeoutError, CancelledError
import pytest
import time
import sys
import weakref

class MockFuture:
    _asyncio_future_blocking = False
    _state = 'PENDING'
    _cancel_return = False
    _result = None

    def __await__(self):
        while self._state == 'PENDING':
            yield self
        return self.result()

    def done(self):
        return self._state != 'PENDING'

    def cancelled(self):
        return self._state == 'CANCELLED'

    def cancel(self):
        return self._cancel_return

    def result(self):
        if self._state == 'CANCELLED':
            raise CancelledError

        return self._result

def test_future_cancelled():
    fut = core.AsyncFuture()

    assert not fut.done()
    assert not fut.cancelled()
    fut.cancel()
    assert fut.done()
    assert fut.cancelled()

    with pytest.raises(CancelledError):
        fut.result()

    # Works more than once
    with pytest.raises(CancelledError):
        fut.result()


def test_future_timeout():
    fut = core.AsyncFuture()

    with pytest.raises(TimeoutError):
        fut.result(0.001)

    # Works more than once
    with pytest.raises(TimeoutError):
        fut.result(0.001)


@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_future_wait():
    threading = pytest.importorskip("direct.stdpy.threading")

    fut = core.AsyncFuture()

    # Launch a thread to set the result value.
    def thread_main():
        time.sleep(0.001)
        fut.set_result(None)

    thread = threading.Thread(target=thread_main)

    assert not fut.done()
    thread.start()

    assert fut.result() is None

    assert fut.done()
    assert not fut.cancelled()
    assert fut.result() is None


@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_future_wait_timeout():
    threading = pytest.importorskip("direct.stdpy.threading")

    fut = core.AsyncFuture()

    # Launch a thread to set the result value.
    def thread_main():
        time.sleep(0.001)
        fut.set_result(None)

    thread = threading.Thread(target=thread_main)

    assert not fut.done()
    thread.start()

    assert fut.result(1.0) is None

    assert fut.done()
    assert not fut.cancelled()
    assert fut.result() is None


@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_future_wait_cancel():
    threading = pytest.importorskip("direct.stdpy.threading")

    fut = core.AsyncFuture()

    # Launch a thread to cancel the future.
    def thread_main():
        time.sleep(0.001)
        fut.cancel()

    thread = threading.Thread(target=thread_main)

    assert not fut.done()
    thread.start()

    with pytest.raises(CancelledError):
        fut.result()

    assert fut.done()
    assert fut.cancelled()
    with pytest.raises(CancelledError):
        fut.result()


def test_task_cancel():
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task = core.PythonTask(lambda task: task.done)
    task_mgr.add(task)

    assert not task.done()
    task_mgr.remove(task)
    assert task.done()
    assert task.cancelled()

    with pytest.raises(CancelledError):
        task.result()


def test_task_cancel_during_run():
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_chain = task_mgr.make_task_chain("test_task_cancel_during_run")

    def task_main(task):
        task.remove()

        # It won't yet be marked done until after it returns.
        assert not task.done()
        return task.done

    task = core.PythonTask(task_main)
    task.set_task_chain(task_chain.name)
    task_mgr.add(task)
    task_chain.wait_for_tasks()

    assert task.done()
    assert task.cancelled()
    with pytest.raises(CancelledError):
        task.result()


@pytest.mark.skipif(not core.Thread.is_threading_supported(),
                    reason="Threading support disabled")
def test_task_cancel_waiting():
    # Calling result() in a threaded task chain should cancel the future being
    # waited on if the surrounding task is cancelled.
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_chain = task_mgr.make_task_chain("test_task_cancel_waiting")
    task_chain.set_num_threads(1)

    fut = core.AsyncFuture()

    async def task_main(task):
        # This will block the thread this task is in until the future is done,
        # or until the task is cancelled (which implicitly cancels the future).
        fut.result()
        return task.done

    task = core.PythonTask(task_main, 'task_main')
    task.set_task_chain(task_chain.name)
    task_mgr.add(task)

    task_chain.start_threads()
    try:
        assert not task.done()
        fut.cancel()
        task.wait()

        assert task.cancelled()
        assert fut.cancelled()

    finally:
        task_chain.stop_threads()


def test_task_cancel_awaiting():
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_chain = task_mgr.make_task_chain("test_task_cancel_awaiting")

    fut = core.AsyncFuture()

    async def task_main(task):
        await fut
        return task.done

    task = core.PythonTask(task_main, 'task_main')
    task.set_task_chain(task_chain.name)
    task_mgr.add(task)

    task_chain.poll()
    assert not task.done()

    task_chain.poll()
    assert not task.done()

    task.cancel()
    task_chain.poll()
    assert task.done()
    assert task.cancelled()
    assert fut.done()
    assert fut.cancelled()


def test_task_result():
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_chain = task_mgr.make_task_chain("test_task_result")

    def task_main(task):
        task.set_result(42)

        # It won't yet be marked done until after it returns.
        assert not task.done()
        return core.PythonTask.done

    task = core.PythonTask(task_main)
    task.set_task_chain(task_chain.name)
    task_mgr.add(task)
    task_chain.wait_for_tasks()

    assert task.done()
    assert not task.cancelled()
    assert task.result() == 42


def test_coro_await_coro():
    # Await another coro in a coro.
    fut = core.AsyncFuture()
    async def coro2():
        await fut

    async def coro_main():
        await coro2()

    task = core.PythonTask(coro_main())

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    for i in range(5):
        task_mgr.poll()

    assert not task.done()
    fut.set_result(None)
    task_mgr.poll()
    assert task.done()
    assert not task.cancelled()


def test_coro_await_cancel_resistant_coro():
    # Await another coro in a coro, but cancel the outer.
    fut = core.AsyncFuture()
    cancelled_caught = [0]
    keep_going = [False]

    async def cancel_resistant_coro():
        while not fut.done():
            try:
                await core.AsyncFuture.shield(fut)
            except CancelledError as ex:
                cancelled_caught[0] += 1

    async def coro_main():
        await cancel_resistant_coro()

    task = core.PythonTask(coro_main(), 'coro_main')

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    assert not task.done()

    task_mgr.poll()
    assert not task.done()

    # No cancelling it once it started...
    for i in range(3):
        assert task.cancel()
        assert not task.done()

        for j in range(3):
            task_mgr.poll()
            assert not task.done()

    assert cancelled_caught[0] == 3

    fut.set_result(None)
    task_mgr.poll()
    assert task.done()
    assert not task.cancelled()


def test_coro_await_external():
    # Await an external future in a coro.
    fut = MockFuture()
    fut._result = 12345
    res = []

    async def coro_main():
        res.append(await fut)

    task = core.PythonTask(coro_main(), 'coro_main')

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    for i in range(5):
        task_mgr.poll()

    assert not task.done()
    fut._state = 'FINISHED'
    task_mgr.poll()
    assert task.done()
    assert not task.cancelled()
    assert res == [12345]


def test_coro_await_external_cancel_inner():
    # Cancel external future being awaited by a coro.
    fut = MockFuture()

    async def coro_main():
        await fut

    task = core.PythonTask(coro_main(), 'coro_main')

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    for i in range(5):
        task_mgr.poll()

    assert not task.done()
    fut._state = 'CANCELLED'
    assert not task.done()
    task_mgr.poll()
    assert task.done()
    assert task.cancelled()


def test_coro_await_external_cancel_outer():
    # Cancel task that is awaiting external future.
    fut = MockFuture()
    result = []

    async def coro_main():
        result.append(await fut)

    task = core.PythonTask(coro_main(), 'coro_main')

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    for i in range(5):
        task_mgr.poll()

    assert not task.done()
    fut._state = 'CANCELLED'
    assert not task.done()
    task_mgr.poll()
    assert task.done()
    assert task.cancelled()


def test_coro_exception():
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_chain = task_mgr.make_task_chain("test_coro_exception")

    def coro_main():
        raise RuntimeError
        yield None

    task = core.PythonTask(coro_main())
    task.set_task_chain(task_chain.name)
    task_mgr.add(task)
    task_chain.wait_for_tasks()

    assert task.done()
    assert not task.cancelled()
    with pytest.raises(RuntimeError):
        task.result()


def test_future_result():
    # Cancelled
    fut = core.AsyncFuture()
    assert not fut.done()
    fut.cancel()
    with pytest.raises(CancelledError):
        fut.result()

    # None
    fut = core.AsyncFuture()
    fut.set_result(None)
    assert fut.done()
    assert fut.result() is None

    # Store int
    fut = core.AsyncFuture()
    fut.set_result(123)
    assert fut.result() == 123

    # Store string
    fut = core.AsyncFuture()
    fut.set_result("test\000\u1234")
    assert fut.result() == "test\000\u1234"

    # Store TypedWritableReferenceCount
    tex = core.Texture()
    rc = tex.get_ref_count()
    fut = core.AsyncFuture()
    fut.set_result(tex)
    assert tex.get_ref_count() == rc + 1
    assert fut.result() == tex
    assert tex.get_ref_count() == rc + 1
    assert fut.result() == tex
    assert tex.get_ref_count() == rc + 1
    fut = None
    assert tex.get_ref_count() == rc

    # Store EventParameter (no longer gets unwrapped)
    ep = core.EventParameter(0.5)
    fut = core.AsyncFuture()
    fut.set_result(ep)
    assert fut.result() is ep
    assert fut.result() is ep

    # Store TypedObject
    dg = core.Datagram(b"test")
    fut = core.AsyncFuture()
    fut.set_result(dg)
    assert fut.result() == dg
    assert fut.result() == dg

    # Store arbitrary Python object
    obj = object()
    rc = sys.getrefcount(obj)
    fut = core.AsyncFuture()
    fut.set_result(obj)
    assert sys.getrefcount(obj) == rc + 1
    assert fut.result() is obj
    assert sys.getrefcount(obj) == rc + 1
    assert fut.result() is obj
    assert sys.getrefcount(obj) == rc + 1
    fut = None
    assert sys.getrefcount(obj) == rc


def test_future_gather():
    fut1 = core.AsyncFuture()
    fut2 = core.AsyncFuture()

    # 0 and 1 arguments are special
    assert core.AsyncFuture.gather().done()
    assert core.AsyncFuture.gather(fut1) == fut1

    # Gathering not-done futures
    gather = core.AsyncFuture.gather(fut1, fut2)
    assert not gather.done()

    # One future done
    fut1.set_result(1)
    assert not gather.done()

    # Two futures done
    fut2.set_result(2)
    assert gather.done()

    assert not gather.cancelled()
    assert tuple(gather.result()) == (1, 2)


def test_future_gather_cancel_inner():
    fut1 = core.AsyncFuture()
    fut2 = core.AsyncFuture()

    # Gathering not-done futures
    gather = core.AsyncFuture.gather(fut1, fut2)
    assert not gather.done()

    # One future cancelled
    fut1.cancel()
    assert not gather.done()

    # Two futures cancelled
    fut2.set_result(2)
    assert gather.done()

    assert not gather.cancelled()
    with pytest.raises(CancelledError):
        assert gather.result()


def test_future_gather_cancel_outer():
    fut1 = core.AsyncFuture()
    fut2 = core.AsyncFuture()

    # Gathering not-done futures
    gather = core.AsyncFuture.gather(fut1, fut2)
    assert not gather.done()

    assert gather.cancel()
    assert gather.done()
    assert gather.cancelled()

    with pytest.raises(CancelledError):
        assert gather.result()


def test_future_shield():
    # An already done future is returned as-is (no cancellation can occur)
    inner = core.AsyncFuture()
    inner.set_result(None)
    outer = core.AsyncFuture.shield(inner)
    assert inner == outer

    # Normally finishing future
    inner = core.AsyncFuture()
    outer = core.AsyncFuture.shield(inner)
    assert not outer.done()
    inner.set_result(None)
    assert outer.done()
    assert not outer.cancelled()
    assert inner.result() is None

    # Normally finishing future with result
    inner = core.AsyncFuture()
    outer = core.AsyncFuture.shield(inner)
    assert not outer.done()
    inner.set_result(123)
    assert outer.done()
    assert not outer.cancelled()
    assert inner.result() == 123

    # Cancelled inner future does propagate cancellation outward
    inner = core.AsyncFuture()
    outer = core.AsyncFuture.shield(inner)
    assert not outer.done()
    inner.cancel()
    assert outer.done()
    assert outer.cancelled()

    # Finished outer future does nothing to inner
    inner = core.AsyncFuture()
    outer = core.AsyncFuture.shield(inner)
    outer.set_result(None)
    assert not inner.done()
    inner.cancel()
    assert not outer.cancelled()

    # Cancelled outer future does nothing to inner
    inner = core.AsyncFuture()
    outer = core.AsyncFuture.shield(inner)
    outer.cancel()
    assert not inner.done()
    inner.cancel()

    # Can be shielded multiple times
    inner = core.AsyncFuture()
    outer1 = core.AsyncFuture.shield(inner)
    outer2 = core.AsyncFuture.shield(inner)
    outer1.cancel()
    assert not inner.done()
    assert not outer2.done()
    inner.cancel()
    assert outer1.done()
    assert outer2.done()


def test_future_done_callback():
    fut = core.AsyncFuture()

    # Use the list hack since Python 2 doesn't have the "nonlocal" keyword.
    called = [False]
    def on_done(arg):
        assert arg == fut
        called[0] = True

    fut.add_done_callback(on_done)
    fut.cancel()
    assert fut.done()

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.poll()
    assert called[0]


def test_future_done_callback_already_done():
    # Same as above, but with the future already done when add_done_callback
    # is called.
    fut = core.AsyncFuture()
    fut.cancel()
    assert fut.done()

    # Use the list hack since Python 2 doesn't have the "nonlocal" keyword.
    called = [False]
    def on_done(arg):
        assert arg == fut
        called[0] = True

    fut.add_done_callback(on_done)

    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.poll()
    assert called[0]


def test_event_future():
    queue = core.EventQueue()
    handler = core.EventHandler(queue)

    fut = handler.get_future("test")

    # If we ask again, we should get the same one.
    assert handler.get_future("test") == fut

    event = core.Event("test")
    handler.dispatch_event(event)

    assert fut.done()
    assert not fut.cancelled()
    assert fut.result() == event


def test_event_future_cancel():
    # This is a very strange thing to do, but it's possible, so let's make
    # sure it gives defined behavior.
    queue = core.EventQueue()
    handler = core.EventHandler(queue)

    fut = handler.get_future("test")
    fut.cancel()

    assert fut.done()
    assert fut.cancelled()

    event = core.Event("test")
    handler.dispatch_event(event)

    assert fut.done()
    assert fut.cancelled()


def test_event_future_cancel2():
    queue = core.EventQueue()
    handler = core.EventHandler(queue)

    # Make sure we get a new future if we cancelled the first one.
    fut = handler.get_future("test")
    fut.cancel()
    fut2 = handler.get_future("test")

    assert fut != fut2
    assert fut.done()
    assert fut.cancelled()
    assert not fut2.done()
    assert not fut2.cancelled()

def test_task_manager_cleanup_non_panda_future():
    future = MockFuture()
    # Create a weakref so we can verify the future was cleaned up.
    future_ref = weakref.ref(future)

    async def coro_main():
        return await future

    task = core.PythonTask(coro_main(), 'coro_main')
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    # Poll the task_mgr so the PythonTask starts polling on future.done()
    task_mgr.poll()
    future._result = 'Done123'
    future._state = 'DONE'
    # Drop our reference to the future, `task` should hold the only reference
    del future
    # Recognize the future has completed
    task_mgr.poll()

    # Verify the task was completed.
    assert task.result() == 'Done123'
    del coro_main # this should break the last strong reference to the mock future

    assert future_ref() is None, "MockFuture was not cleaned up!"

def test_await_future_result_cleanup():
    # Create a simple future and an object for it to return
    future = core.AsyncFuture()

    class TestObject:
        pass

    test_result = TestObject()
    future_result_ref = weakref.ref(test_result)

    # Setup an async environment and dispatch it to a task chain to await the future
    async def coro_main():
        nonlocal test_result
        future.set_result(test_result)
        del test_result
        await future

    task = core.PythonTask(coro_main(), 'coro_main')
    task_mgr = core.AsyncTaskManager.get_global_ptr()
    task_mgr.add(task)
    # Poll the task_mgr so the PythonTask starts polling on future.done()
    task_mgr.poll()
    # Break all possible references to the future object
    del task
    del coro_main
    del future # this should break the last strong reference to the future

    assert future_result_ref() is None, "TestObject was not cleaned up!"
