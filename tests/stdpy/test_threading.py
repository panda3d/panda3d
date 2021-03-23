from direct.stdpy import threading
import pytest


def test_threading_error():
    with pytest.raises(threading.ThreadError):
        threading.stack_size()
