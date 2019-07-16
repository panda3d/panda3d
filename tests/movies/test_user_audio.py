import pytest

from panda3d.core import UserDataAudio


@pytest.mark.parametrize("remove_after_read", [True, False])
def test_userdata_audio(remove_after_read):
    audio = UserDataAudio(48000, 2, remove_after_read)
    audio.append(b'abcdefgh')
    audio.done()
    cursor = audio.open()
    assert cursor.read_samples(0) == b''
    assert cursor.read_samples(1) == b'abcd'
    assert cursor.read_samples(1) == b'efgh'
    assert cursor.read_samples(1) == b''
