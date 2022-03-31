set(P3MOVIES_HEADERS
  dr_flac.h
  config_movies.h
  flacAudio.h flacAudio.I
  flacAudioCursor.h flacAudioCursor.I
  inkblotVideo.h inkblotVideo.I
  inkblotVideoCursor.h inkblotVideoCursor.I
  microphoneAudio.h microphoneAudio.I
  movieAudio.h movieAudio.I
  movieAudioCursor.h movieAudioCursor.I
  movieTypeRegistry.h movieTypeRegistry.I
  movieVideo.h movieVideo.I
  movieVideoCursor.h movieVideoCursor.I
  opusAudio.h opusAudio.I
  opusAudioCursor.h opusAudioCursor.I
  userDataAudio.h userDataAudio.I
  userDataAudioCursor.h userDataAudioCursor.I
  vorbisAudio.h vorbisAudio.I
  vorbisAudioCursor.h vorbisAudioCursor.I
  wavAudio.h wavAudio.I
  wavAudioCursor.h wavAudioCursor.I
)

set(P3MOVIES_SOURCES
  config_movies.cxx
  flacAudio.cxx
  flacAudioCursor.cxx
  inkblotVideo.cxx
  inkblotVideoCursor.cxx
  microphoneAudio.cxx
  microphoneAudioDS.cxx
  movieAudio.cxx
  movieAudioCursor.cxx
  movieTypeRegistry.cxx
  movieVideo.cxx
  movieVideoCursor.cxx
  opusAudio.cxx
  opusAudioCursor.cxx
  userDataAudio.cxx
  userDataAudioCursor.cxx
  vorbisAudio.cxx
  vorbisAudioCursor.cxx
  wavAudio.cxx
  wavAudioCursor.cxx
)

composite_sources(p3movies P3MOVIES_SOURCES)
add_component_library(p3movies SYMBOL BUILDING_PANDA_MOVIES
  ${P3MOVIES_HEADERS} ${P3MOVIES_SOURCES})
target_link_libraries(p3movies p3pstatclient p3gobj p3pandabase pandaexpress
  PKG::VORBIS PKG::OPUS)
target_interrogate(p3movies ALL)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  if(HAVE_OPUS OR HAVE_VORBIS)
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libogg.a")
  endif()

  if(HAVE_VORBIS)
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libvorbis.a")
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libvorbisenc.a")
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libvorbisfile.a")
  endif()

  if(HAVE_OPUS)
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libopus.a")
    target_link_options(p3movies PRIVATE "LINKER:--exclude-libs,libopusfile.a")
  endif()
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3movies
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3MOVIES_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
