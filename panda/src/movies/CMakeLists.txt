set(P3MOVIES_HEADERS
    config_movies.h
    inkblotVideo.h inkblotVideo.I
    inkblotVideoCursor.h inkblotVideoCursor.I
    microphoneAudio.h microphoneAudio.I
    movieAudio.h movieAudio.I
    movieAudioCursor.h movieAudioCursor.I
    movieTypeRegistry.h movieTypeRegistry.I
    movieVideo.h movieVideo.I
    movieVideoCursor.h movieVideoCursor.I
    userDataAudio.h userDataAudio.I
    userDataAudioCursor.h userDataAudioCursor.I
    vorbisAudio.h vorbisAudio.I
    vorbisAudioCursor.h vorbisAudioCursor.I
    wavAudio.h wavAudio.I
    wavAudioCursor.h wavAudioCursor.I)

set(P3MOVIES_SOURCES
    config_movies.cxx
    inkblotVideo.cxx
    inkblotVideoCursor.cxx
    microphoneAudio.cxx
    microphoneAudioDS.cxx
    movieAudio.cxx
    movieAudioCursor.cxx
    movieTypeRegistry.cxx
    movieVideo.cxx
    movieVideoCursor.cxx
    userDataAudio.cxx
    userDataAudioCursor.cxx
    vorbisAudio.cxx
    vorbisAudioCursor.cxx
    wavAudio.cxx
    wavAudioCursor.cxx)

composite_sources(p3movies P3MOVIES_SOURCES)
add_library(p3movies ${P3MOVIES_HEADERS} ${P3MOVIES_SOURCES})
target_link_libraries(p3movies p3pandabase p3express p3pstatclient p3gobj p3dconfig p3prc)
target_interrogate(p3movies ALL)

install(TARGETS p3movies DESTINATION lib)
