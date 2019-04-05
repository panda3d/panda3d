from panda3d.core import MovieAudio

def test_cursor_type():
    movie_path = "impulse.flac"
    reference_file = MovieAudio.get(movie_path)
    cursor = MovieAudio.open(reference_file)
    file_name_return = MovieAudio.getFilename(reference_file)
    assert file_name_return == movie_path
    assert cursor is not None
    