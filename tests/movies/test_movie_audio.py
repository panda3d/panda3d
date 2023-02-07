from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio
from panda3d.core import Filename
from panda3d.core import PandaSystem
from panda3d.core import MovieTexture
from panda3d.core import MovieVideo

import pytest
import os


def test_audio_rate(): #tests for audio rate
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieAudio.get(movie_path)
    movie_file = reference_file.open()
    assert movie_file.audio_rate() == 48000


def test_audio_length(): #test for testing audio length
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path)
    reference_file = MovieAudio.get(movie_path)
    movie_file = reference_file.open()
    assert movie_file.length() == 2


def test_can_seek(): #test for seeking
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path)
    reference_file = MovieAudio.get(movie_path)
    movie_file = reference_file.open()
    assert movie_file.can_seek() is True


def test_can_seek_fast(): #test for seeking fast
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path)
    reference_file = MovieAudio.get(movie_path)
    movie_file = reference_file.open()
    assert movie_file.can_seek_fast() is True


def test_audio_channel(): #tests for number of audio channels
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path)
    reference_file = MovieAudio.get(movie_path)
    movie_file = reference_file.open()
    assert movie_file.audio_channels() == 1


def test_cursor(): #opening the file returns a cursor
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac")
    movie_path = Filename.from_os_specific(movie_path)
    reference_file = MovieAudio.get(movie_path)
    file_name_return = reference_file.get_filename()
    assert reference_file.open() is not None #checks the cursor
    assert file_name_return == movie_path #checks the return name of the file
