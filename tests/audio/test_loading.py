import pytest

def test_missing_file(audiomgr):
    sound = audiomgr.get_sound('/not/a/valid/file.ogg')
    assert str(sound).startswith('NullAudioSound')

def test_found_file(audiomgr):
    sound = audiomgr.get_sound('../../samples/music-box/music/musicbox.ogg')
    assert not str(sound).startswith('NullAudioSound')
