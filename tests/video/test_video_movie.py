from panda3d.core import MovieVideo
from panda3d.core import Filename
from panda3d.core import MovieVideoCursor
import os

def test_cursor_check(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieVideo.get(movie_path)	    
    assert reference_file.get_filename() == movie_path
    assert reference_file.open() is not None

def test_video_length(): 
    movie_path = os.path.join(os.path.dirname(__file__), "../../samples/media-player/PandaSneezes.ogv")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieVideo.get(movie_path)	    
    cursor = reference_file.open()
    assert cursor.length() == 14

def test_video_size(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieVideo.get(movie_path)	    
    cursor = reference_file.open()
    assert cursor.size_x() == 560 #found the height and width using mkvinfo
    assert cursor.size_y() == 320
