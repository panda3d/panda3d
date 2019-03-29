import pytest
from panda3d.core import *

@pytest.fixture
def clockobj():
    return ClockObject()
