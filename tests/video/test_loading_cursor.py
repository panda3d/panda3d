from panda3d.core import MovieVideo

def test_cursor_check(): 
    movie_path = "../../samples/media-player/PandaSneezes.ogv"
    reference_file = MovieVideo.get(movie_path)
    cursor = MovieVideo.open(reference_file)
    assert cursor is not None
