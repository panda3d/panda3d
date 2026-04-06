from panda3d.core import AudioSound
import pytest

def test_make_copy(audiomgr):
    if "openal" not in str(audiomgr).lower():
        # NULL audio manager (as well as fmod) don't support copying yet
        pytest.skip("Copying is currently only supported on OpenAL")

    test_sound = audiomgr.get_sound("../models/audio/sfx/GUI_click.wav")
    test_sound.setActive(1)
    test_sound.set3dMaxDistance(20.0)
    test_copy = test_sound.make_copy()

    assert(test_copy.getActive() == test_sound.getActive())
    assert(test_copy.getName() == test_sound.getName())
    assert(test_copy.get3dMaxDistance() == test_sound.get3dMaxDistance())
    # check that make_copy hasn't changed user-modified value
    assert(test_copy.get3dMaxDistance() == 20.0) 

