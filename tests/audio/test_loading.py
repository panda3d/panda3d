def test_missing_file(audiomgr): #test whether file is loaded in properly
    filename = "/not/a/valid/file.ogg"
    sound = audiomgr.get_sound(filename)
    assert str(sound).startswith('NullAudioSound')
