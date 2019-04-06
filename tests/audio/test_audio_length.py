from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio

def test_audio_rate():
    movie_path = "impulse.flac"
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    length_audio = MovieAudioCursor.length(movie_file)
    assert length_audio == 2