#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c


#begin lib_target
  #define TARGET p3movies
  #define LOCAL_LIBS p3gobj

  #define USE_PACKAGES dx9 vorbis
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] strmiids.lib winmm.lib

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    config_movies.h \
    inkblotVideo.h inkblotVideo.I \
    inkblotVideoCursor.h inkblotVideoCursor.I \
    microphoneAudio.h microphoneAudio.I \
    movieAudio.h movieAudio.I \
    movieAudioCursor.h movieAudioCursor.I \
    movieTypeRegistry.h movieTypeRegistry.I \
    movieVideo.h movieVideo.I \
    movieVideoCursor.h movieVideoCursor.I \
    userDataAudio.h userDataAudio.I \
    userDataAudioCursor.h userDataAudioCursor.I \
    vorbisAudio.h vorbisAudio.I \
    vorbisAudioCursor.h vorbisAudioCursor.I \
    wavAudio.h wavAudio.I \
    wavAudioCursor.h wavAudioCursor.I

  #define INCLUDED_SOURCES \
    config_movies.cxx \
    inkblotVideo.cxx \
    inkblotVideoCursor.cxx \
    microphoneAudio.cxx \
    microphoneAudioDS.cxx \
    movieAudio.cxx \
    movieAudioCursor.cxx \
    movieTypeRegistry.cxx \
    movieVideo.cxx \
    movieVideoCursor.cxx \
    userDataAudio.cxx \
    userDataAudioCursor.cxx \
    vorbisAudio.cxx \
    vorbisAudioCursor.cxx \
    wavAudio.cxx \
    wavAudioCursor.cxx

  #define INSTALL_HEADERS \
    config_movies.h \
    inkblotVideo.h inkblotVideo.I \
    inkblotVideoCursor.h inkblotVideoCursor.I \
    microphoneAudio.h microphoneAudio.I \
    movieAudio.h movieAudio.I \
    movieAudioCursor.h movieAudioCursor.I \
    movieTypeRegistry.h movieTypeRegistry.I \
    movieVideo.h movieVideo.I \
    movieVideoCursor.h movieVideoCursor.I \
    vorbisAudio.h vorbisAudio.I \
    vorbisAudioCursor.h vorbisAudioCursor.I \
    wavAudio.h wavAudio.I \
    wavAudioCursor.h wavAudioCursor.I

  #define IGATESCAN all

#end lib_target
