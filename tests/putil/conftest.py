import pytest
from panda3d.core import ClockObject

@pytest.fixture
def clockobj():
    return ClockObject()
