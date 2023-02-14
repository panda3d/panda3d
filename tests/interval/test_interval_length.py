from panda3d.core import *
from direct.interval.IntervalGlobal import *
import pytest


def test_invalid_sound_interval():
    load_prc_file_data('', 'audio-library-name p3fmod_audio')
    amgr = AudioManager.create_AudioManager()

    sound = SoundInterval(amgr.get_sound(MovieAudio('Load-Failure Stub')))
    seq = Sequence()

    seq.append(WaitInterval(1.0))
    seq.append(sound)

    seq.start()
    ivalMgr.step()

    amgr.shutdown()

    assert seq is not None


def test_long_interval():
    seq = Sequence()

    seq.append(WaitInterval(1.0E10))
    seq.append(WaitInterval(1))

    seq.start()
    ivalMgr.step()

    assert seq is not None
