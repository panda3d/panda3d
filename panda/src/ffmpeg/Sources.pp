#define BUILD_DIRECTORY $[HAVE_FFMPEG]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define BUILDING_DLL BUILDING_FFMPEG

#define USE_PACKAGES ffmpeg

#begin lib_target
  #define TARGET p3ffmpeg

  #define LOCAL_LIBS p3movies

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    config_ffmpeg.h \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I
    
  #define INCLUDED_SOURCES \
    config_ffmpeg.cxx \
    ffmpegVideo.cxx \
    ffmpegVideoCursor.cxx \
    ffmpegAudio.cxx \
    ffmpegAudioCursor.cxx \
    ffmpegVirtualFile.cxx
    
  #define INSTALL_HEADERS \
    config_ffmpeg.h \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I

#end lib_target
