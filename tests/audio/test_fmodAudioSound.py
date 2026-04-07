from panda3d.core import AudioSound
import pytest

def test_make_copy(audiomgr):
    test_sound = audiomgr.get_sound("../models/audio/sfx/GUI_click.wav")
    test_copy = test_sound.make_copy()
    assert(test_copy == None)

