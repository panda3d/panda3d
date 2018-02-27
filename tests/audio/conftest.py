import pytest
from panda3d.core import *

@pytest.fixture(scope='module')
def audiomgr():
    return AudioManager.create_AudioManager()
