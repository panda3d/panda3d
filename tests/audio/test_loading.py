import os

import pytest
from panda3d.core import Filename


def test_missing_file(audiomgr):
    sound = audiomgr.get_sound("/not/a/valid/file.ogg")
    assert str(sound).startswith("NullAudioSound")


@pytest.mark.parametrize("extension", ["ogg", "opus", "mp3"])
def test_comments(audiomgr, extension):
    if "openal" not in str(audiomgr).lower():
        # NULL audio manager (as well as fmod) don't support comment reading
        pytest.skip("Comment reading is only supported on OpenAL")
    # ogg should be loaded with libvorbis, opus with libopus, mp3 with ffmpeg
    # we cannot test this though because after loading the loader information is gone
    sound_path = os.path.join(
        os.path.dirname(__file__), f"openclose_with_comments.{extension}"
    )
    sound_path = Filename.from_os_specific(sound_path)
    sound = audiomgr.get_sound(sound_path)
    if extension == "mp3":  # FFMPEG encodes/decodes tags differently
        tags = ["artist", "comment", "genre"]
    else:
        tags = ["ARTIST", "COMMENTS", "GENRE"]
    assert sorted(sound.raw_comments) == [
        f"{tags[0]}=Example Artist",
        f"{tags[1]}=This is an example OGG comment",
        f"{tags[2]}=Blues",
    ]
    assert sound.comments[tags[-1]] == "Blues"
    assert sound.get_comment(tags[0]) == "Example Artist"
    assert sound.get_comment("DOES NOT EXIST") == ""
    assert sound.has_comment(tags[0])
