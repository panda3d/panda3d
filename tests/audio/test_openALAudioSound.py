from panda3d.core import AudioSound
import pytest

def test_copy(audiomgr):
    if "openal" not in str(audiomgr).lower():
        # NULL audio manager (as well as fmod) don't support comment reading
        pytest.skip("Comment reading is only supported on OpenAL")

    test_sound = AudioSound()
    test_sound.setActive(1)
    test_sound.set3dMaxDistance(200.0)
    test_copy = test_sound.make_copy()

    # checking a few values for consistency
    assert(test_copy.getActive() == test_sound.getActive())
    assert(test_copy.getName() == (test_sound.getName() + "_copy"))
    assert(test_copy.get3dMaxDistance() == test_sound.get3dMaxDistance())

    base.run()

