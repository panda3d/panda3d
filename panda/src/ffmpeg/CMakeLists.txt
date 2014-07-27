if(HAVE_FFMPEG)

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
  target_link_libraries(p3ffmpeg p3movies)
  target_use_packages(p3ffmpeg FFMPEG SWSCALE SWRESAMPLE)

  install(TARGETS p3ffmpeg DESTINATION lib)
endif()
