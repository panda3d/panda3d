from panda3d.core import MovieAudio
from panda3d.core import MovieAudioCursor
import os 

def test_can_seek():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    can_seek_test = MovieAudioCursor.can_seek(movie_file)
    assert can_seek_test is True

def test_can_seek_fast():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    can_seek_test_fast = MovieAudioCursor.can_seek(movie_file)
    assert can_seek_test_fast is True