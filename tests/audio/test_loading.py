import pytest

@pytest.mark.xfail
def test_missing_file(audiomgr):
    sound = audiomgr.get_sound('/not/a/valid/file.ogg')
    assert sound is None
