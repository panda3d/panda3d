import sys
from panda3d import core
from direct.stdpy import threading
import pytest


def test_threading_error():
    with pytest.raises(threading.ThreadError):
        threading.stack_size()


@pytest.mark.skipif(sys.platform == "emscripten", reason="No threading")
def test_threading():
    from collections import deque

    _sleep = core.Thread.sleep

    _VERBOSE = False

    class _Verbose(object):

        def __init__(self, verbose=None):
            if verbose is None:
                verbose = _VERBOSE
            self.__verbose = verbose

        def _note(self, format, *args):
            if self.__verbose:
                format = format % args
                format = "%s: %s\n" % (
                    threading.currentThread().getName(), format)
                sys.stderr.write(format)

    class BoundedQueue(_Verbose):

        def __init__(self, limit):
            _Verbose.__init__(self)
            self.mon = threading.Lock(name = "BoundedQueue.mon")
            self.rc = threading.Condition(self.mon)
            self.wc = threading.Condition(self.mon)
            self.limit = limit
            self.queue = deque()

        def put(self, item):
            self.mon.acquire()
            while len(self.queue) >= self.limit:
                self._note("put(%s): queue full", item)
                self.wc.wait()
            self.queue.append(item)
            self._note("put(%s): appended, length now %d",
                        item, len(self.queue))
            self.rc.notify()
            self.mon.release()

        def get(self):
            self.mon.acquire()
            while not self.queue:
                self._note("get(): queue empty")
                self.rc.wait()
            item = self.queue.popleft()
            self._note("get(): got %s, %d left", item, len(self.queue))
            self.wc.notify()
            self.mon.release()
            return item

    class ProducerThread(threading.Thread):

        def __init__(self, queue, quota):
            threading.Thread.__init__(self, name="Producer")
            self.queue = queue
            self.quota = quota

        def run(self):
            from random import random
            counter = 0
            while counter < self.quota:
                counter = counter + 1
                self.queue.put("%s.%d" % (self.getName(), counter))
                _sleep(random() * 0.00001)

    class ConsumerThread(threading.Thread):

        def __init__(self, queue, count):
            threading.Thread.__init__(self, name="Consumer")
            self.queue = queue
            self.count = count

        def run(self):
            while self.count > 0:
                item = self.queue.get()
                print(item)
                self.count = self.count - 1

    NP = 3
    QL = 4
    NI = 5

    Q = BoundedQueue(QL)
    P = []
    for i in range(NP):
        t = ProducerThread(Q, NI)
        t.setName("Producer-%d" % (i+1))
        P.append(t)
    C = ConsumerThread(Q, NI*NP)
    for t in P:
        t.start()
        _sleep(0.000001)
    C.start()
    for t in P:
        t.join()
    C.join()
