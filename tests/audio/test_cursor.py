from panda3d.core import MovieAudio
import pytest

def test_cursor_type():
    movie_path = "impulse.flac"
    get_file = MovieAudio.get(movie_path)
    cursor_type = MovieAudio.open(get_file)
    assert cursor_type is not None