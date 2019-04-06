from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio
import os

def test_audio_channel():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    channel_num = MovieAudioCursor.audio_channels(movie_file) 
    assert channel_num is 1
