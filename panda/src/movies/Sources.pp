#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES 

#begin lib_target
  #define TARGET movies
  #define LOCAL_LIBS \
        gobj
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
	movie.h movie.I \
	movieAudio.h movieAudio.I \
	movieVideo.h movieVideo.I \
	config_movies.h
    
  #define INCLUDED_SOURCES \
        movieVideo.cxx \
        movieAudio.cxx \
        movie.cxx \
        config_movies.cxx
    
  #define INSTALL_HEADERS \
	movie.h movie.I \
	movieAudio.h movieAudio.I \
	movieVideo.h movieVideo.I \
	config_movies.h

  #define IGATESCAN all

#end lib_target

