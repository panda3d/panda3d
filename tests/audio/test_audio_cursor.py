from panda3d.core import MovieAudio
import os

def test_cursor():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    reference_file = MovieAudio.get(movie_path)
    cursor = MovieAudio.open(reference_file)
    file_name_return = MovieAudio.getFilename(reference_file)
    assert file_name_return == movie_path
    assert cursor is not None
