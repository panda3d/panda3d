if(NOT HAVE_FFMPEG)
  return()
endif()

set(P3FFMPEG_HEADERS
  config_ffmpeg.h
  ffmpegVideo.h ffmpegVideo.I
  ffmpegVideoCursor.h ffmpegVideoCursor.I
  ffmpegAudio.h ffmpegAudio.I
  ffmpegAudioCursor.h ffmpegAudioCursor.I
  ffmpegVirtualFile.h ffmpegVirtualFile.I
)

set(P3FFMPEG_SOURCES
  config_ffmpeg.cxx
  ffmpegVideo.cxx
  ffmpegVideoCursor.cxx
  ffmpegAudio.cxx
  ffmpegAudioCursor.cxx
  ffmpegVirtualFile.cxx
)

composite_sources(p3ffmpeg P3FFMPEG_SOURCES)
add_library(p3ffmpeg ${MODULE_TYPE} ${P3FFMPEG_HEADERS} ${P3FFMPEG_SOURCES})
set_target_properties(p3ffmpeg PROPERTIES DEFINE_SYMBOL BUILDING_FFMPEG)
target_link_libraries(p3ffmpeg panda
  PKG::FFMPEG PKG::SWSCALE PKG::SWRESAMPLE)

if(HAVE_SWSCALE)
  target_compile_definitions(p3ffmpeg PRIVATE HAVE_SWSCALE)
endif()

if(HAVE_SWRESAMPLE)
  target_compile_definitions(p3ffmpeg PRIVATE HAVE_SWRESAMPLE)
endif()

install(TARGETS p3ffmpeg
  EXPORT FFmpeg COMPONENT FFmpeg
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT FFmpegDevel)
install(FILES ${P3FFMPEG_HEADERS} COMPONENT FFmpegDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(FFmpeg COMPONENT FFmpegDevel)
