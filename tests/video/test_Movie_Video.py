from panda3d.core import MovieVideo
from panda3d.core import Filename
from panda3d.core import MovieVideoCursor
import os

def test_cursor_check(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    assert reference_file.get_filename() == Filename.from_os_specific(movie_path)
    assert reference_file.open() is not None

def test_video_length(): 
    movie_path = os.path.join(os.path.dirname(__file__), "../../samples/media-player/PandaSneezes.ogv")
    reference_file = MovieVideo.get(movie_path)	    
    cursor = reference_file.open()
    assert cursor.length() == 14

def test_video_size(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    cursor = reference_file.open()
    assert MovieVideoCursor.size_x(cursor) == 560 #found the height and width using mkvinfo
    assert MovieVideoCursor.size_y(cursor) == 320
