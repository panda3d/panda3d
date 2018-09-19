if(NOT HAVE_FFMPEG)
  return()
endif()

set(P3FFMPEG_HEADERS
    config_ffmpeg.h
    ffmpegVideo.h ffmpegVideo.I
    ffmpegVideoCursor.h ffmpegVideoCursor.I
    ffmpegAudio.h ffmpegAudio.I
    ffmpegAudioCursor.h ffmpegAudioCursor.I
    ffmpegVirtualFile.h ffmpegVirtualFile.I)


set(P3FFMPEG_SOURCES
    config_ffmpeg.cxx
    ffmpegVideo.cxx
    ffmpegVideoCursor.cxx
    ffmpegAudio.cxx
    ffmpegAudioCursor.cxx
    ffmpegVirtualFile.cxx)

composite_sources(p3ffmpeg P3FFMPEG_SOURCES)
add_library(p3ffmpeg ${P3FFMPEG_HEADERS} ${P3FFMPEG_SOURCES})
set_target_properties(p3ffmpeg PROPERTIES DEFINE_SYMBOL BUILDING_FFMPEG)
target_link_libraries(p3ffmpeg panda
  PKG::FFMPEG PKG::SWSCALE PKG::SWRESAMPLE)

install(TARGETS p3ffmpeg DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${P3FFMPEG_HEADERS} DESTINATION include/panda3d)
