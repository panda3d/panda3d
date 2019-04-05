import pytest

def test_missing_file(audiomgr):
    filename = "/not/a/valid/file.ogg"
    sound = audiomgr.get_sound(filename)
    assert str(sound).startswith('NullAudioSound')