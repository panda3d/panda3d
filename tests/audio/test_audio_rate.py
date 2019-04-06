from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio
import os

def test_audio_rate():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    rate_audio = MovieAudioCursor.audioRate(movie_file)
    assert rate_audio == 48000
    