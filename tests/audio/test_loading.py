import pytest
import os
def test_missing_file(audiomgr):
    filename = "/not/a/valid/file.ogg"
    sound = audiomgr.get_sound(filename)
    assert str(sound).startswith('NullAudioSound')

def test_check_file_extension(audiomgr):
    filename, file_ext = os.path.splitext("/not/a/valid/file.ogg")
    assert file_ext.startswith((".mp3",".wav","aiff",".midi",".mod",".wma",".ogg",".vorbis"))

