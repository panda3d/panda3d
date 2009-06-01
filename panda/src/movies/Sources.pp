#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES ffmpeg dx9
#define WIN_SYS_LIBS strmiids.lib

#begin lib_target
  #define TARGET movies
  #define LOCAL_LIBS \
        gobj
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    movieAudio.h movieAudio.I \
    movieAudioCursor.h movieAudioCursor.I \
    movieVideo.h movieVideo.I \
    movieVideoCursor.h movieVideoCursor.I \
    inkblotVideo.h inkblotVideo.I \
    inkblotVideoCursor.h inkblotVideoCursor.I \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I \
    userDataAudio.h userDataAudio.I \
    userDataAudioCursor.h userDataAudioCursor.I \
    webcamVideo.h webcamVideo.I \
    microphoneAudio.h microphoneAudio.I \
    config_movies.h
    
  #define INCLUDED_SOURCES \
    movieVideo.cxx \
    movieVideoCursor.cxx \
    movieAudio.cxx \
    movieAudioCursor.cxx \
    inkblotVideo.cxx \
    inkblotVideoCursor.cxx \
    ffmpegVideo.cxx \
    ffmpegVideoCursor.cxx \
    ffmpegAudio.cxx \
    ffmpegAudioCursor.cxx \
    ffmpegVirtualFile.cxx \
    userDataAudio.cxx \
    userDataAudioCursor.cxx \
    webcamVideo.cxx \
    webcamVideoDS.cxx \
    microphoneAudio.cxx \
    microphoneAudioDS.cxx \
    config_movies.cxx
    
  #define INSTALL_HEADERS \
    movieAudio.h movieAudio.I \
    movieAudioCursor.h movieAudioCursor.I \
    movieVideo.h movieVideo.I \
    movieVideoCursor.h movieVideoCursor.I \
    inkblotVideo.h inkblotVideo.I \
    inkblotVideoCursor.h inkblotVideoCursor.I \
    ffmpegVideo.h ffmpegVideo.I \
    ffmpegVideoCursor.h ffmpegVideoCursor.I \
    ffmpegAudio.h ffmpegAudio.I \
    ffmpegAudioCursor.h ffmpegAudioCursor.I \
    ffmpegVirtualFile.h ffmpegVirtualFile.I \
    webcamVideo.h webcamVideo.I \
    microphoneAudio.h microphoneAudio.I \
    config_movies.h

  #define IGATESCAN all

#end lib_target

