import pytest

def test_missing_file(audiomgr):
    sound = audiomgr.get_sound('/not/a/valid/file.ogg')
    assert str(sound).startswith('NullAudioSound')
