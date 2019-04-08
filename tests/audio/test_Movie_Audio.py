from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio
from panda3d.core import Filename
import os

def test_audio_rate(): #tests for audio rate
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    assert movie_file.audioRate() == 48000

def test_missing_file(audiomgr): #test whether file is loaded in properly
    filename = "/not/a/valid/file.ogg"
    sound = audiomgr.get_sound(filename)
    assert str(sound).startswith('NullAudioSound')

def test_audio_length(): #test for testing audio length
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    assert movie_file.length() == 2

def test_can_seek(): #test for seeking
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    assert movie_file.can_seek() is True

def test_can_seek_fast(): #test for seeking fast
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    assert movie_file.can_seek_fast() is True

def test_audio_channel(): #tests for number of audio channels
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    assert movie_file.audio_channels() == 1

def test_cursor(): #opening the file returns a cursor
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    reference_file = MovieAudio.get(movie_path)
    file_name_return = MovieAudio.get_filename(reference_file)
    assert file_name_return == Filename.from_os_specific(movie_path)
    assert reference_file.open() is not None #checks the cursor
