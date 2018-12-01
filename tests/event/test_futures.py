from panda3d import core
import pytest
import time
import sys

if sys.version_info >= (3,):
    from concurrent.futures._base import TimeoutError, CancelledError
else:
    TimeoutError = Exception
    CancelledError = Exception


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

