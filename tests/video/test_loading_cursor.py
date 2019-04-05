from panda3d.core import MovieVideo
import pytest

def test_cursor_check(): 
    movie_path = "../../samples/media-player/PandaSneezes.ogv"
    get_file = MovieVideo.get(movie_path)
    cursor_type = MovieVideo.open(get_file)
    assert cursor_type is not None
