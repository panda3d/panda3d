#define LOCAL_LIBS event ipc express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_ZLIB yes
#define USE_IPC yes

#begin lib_target
  #define TARGET downloader

  #define SOURCES							\
    config_downloader.cxx						\
    config_downloader.h downloadDb.I					\
    asyncUtility.I asyncUtility.cxx asyncUtility.h			\
    downloadDb.cxx downloadDb.h						\
    downloader.I downloader.cxx downloader.h extractor.cxx extractor.h	\
    multiplexStream.I multiplexStream.cxx multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.cxx multiplexStreamBuf.h 

  #define IF_ZLIB_SOURCES						\
    decompressor.cxx decompressor.h zcompressor.I zcompressor.cxx	\
    zcompressor.h download_utils.cxx download_utils.h

  #define IF_CRYPTO_SOURCES \
    patcher.cxx								\
    patcher.h patcher.I

  #define INSTALL_HEADERS \
    asyncUtility.h asyncUtility.I \
    config_downloader.h \
    decompressor.h \
    download_utils.h downloadDb.h downloadDb.I \
    downloader.h downloader.I \
    extractor.h \
    multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.I \
    patcher.h patcher.I \
    zcompressor.I zcompressor.h

  #define IGATESCAN all

#end lib_target
