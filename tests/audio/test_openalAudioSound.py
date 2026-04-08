import os

import pytest
from panda3d.core import Filename, MovieAudio

def test_make_copy(audiomgr):
    if "openal" not in str(audiomgr).lower():
        # NULL audio manager (as well as fmod) don't support copying yet
        pytest.skip("Copying is currently only supported on OpenAL")

    sound_path = os.path.join(os.path.dirname(__file__), "wav_test.wav")
    sound_path = Filename.from_os_specific(sound_path)
    test_sound = MovieAudio.get(sound_path)
    if str(test_sound) == 'Load-Failure Stub':
        pytest.skip("Could not load audio file")
    test_sound = audiomgr.get_sound(test_sound)
    if str(test_sound).startswith("NullAudioSound"):
        pytest.skip("Sound loading failed")

    test_sound.setActive(1)
    test_sound.set3dMaxDistance(20.0)
    test_copy = test_sound.make_copy()

    assert(test_copy.getActive() == test_sound.getActive())
    assert(test_copy.getName() == test_sound.getName())
    assert(test_copy.get3dMaxDistance() == test_sound.get3dMaxDistance())
    # check that make_copy has copied manually-modified parameters
    assert(test_copy.get3dMaxDistance() == 20.0) 

