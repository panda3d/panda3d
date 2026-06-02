import os, sys

import pytest
from panda3d.core import Filename, MovieAudio


def test_make_copy(audiomgr):
    sound = audiomgr.get_sound("/not/a/valid/file.ogg")
    assert str(sound).startswith("NullAudioSound")
    test_copy = sound.make_copy()
    assert str(test_copy).startswith("NullAudioSound")

