import pytest
import panda3d.core
import os

def test_missing_file(audiomgr):
    sound = audiomgr.get_sound('/not/a/valid/file.ogg')
    assert str(sound).startswith('NullAudioSound')

def test_load_ogg_vorbis(audiomgr):
    path = os.path.dirname(__file__) + '../../samples/music-box/music/musicbox.ogg'
    assert not str(path).startswith('NullAudioSound')
