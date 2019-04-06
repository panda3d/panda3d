from panda3d.core import MovieAudioCursor
from panda3d.core import MovieAudio
from panda3d.core import Datagram
from panda3d.core import DatagramIterator
import os

def test_read_samples():
    movie_path = os.path.join(os.path.dirname(__file__), "impulse.flac") #to make sure relative paths aren't a problem
    reference_file = MovieAudio.get(movie_path)
    movie_file = MovieAudio.open(reference_file)
    dg = Datagram()
    MovieAudioCursor.readSamples(movie_file, 9600, dg) #move the data into a datagram
    iterator = DatagramIterator(dg) #convert the datagram into a string
    integer_16_iterator = DatagramIterator.getInt16(iterator)
    integer_16_iterator = str(integer_16_iterator)
    integer_16_iterator = list(integer_16_iterator)

    i = 1
    while i < len(integer_16_iterator):
        if(i == 48000):
            assert integer_16_iterator[i] == 0.5
        else:
            assert integer_16_iterator[i] == 0
        i += 1


