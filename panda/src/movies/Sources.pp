#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES 

#begin lib_target
  #define TARGET movies
  #define LOCAL_LIBS \
        gobj
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
	movieAudio.h movieAudio.I \
	movieVideo.h movieVideo.I \
        inkblotVideo.h inkblotVideo.I \
        ffmpegVideo.h ffmpegVideo.I \
        ffmpegAudio.h ffmpegAudio.I \
	config_movies.h
    
  #define INCLUDED_SOURCES \
        movieVideo.cxx \
        movieAudio.cxx \
        inkblotVideo.cxx \
        ffmpegVideo.cxx \
        ffmpegAudio.cxx \
        config_movies.cxx
    
  #define INSTALL_HEADERS \
	movieAudio.h movieAudio.I \
	movieVideo.h movieVideo.I \
        inkblotVideo.h inkblotVideo.I \
        ffmpegVideo.h ffmpegVideo.I \
        ffmpegAudio.h ffmpegAudio.I \
	config_movies.h

  #define IGATESCAN all

#end lib_target

