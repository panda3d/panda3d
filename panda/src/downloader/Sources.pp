#define LOCAL_LIBS event ipc express pandabase
#define OTHER_LIBS interrogatedb dconfig dtoolutil
#define USE_ZLIB yes

#begin lib_target
  #define TARGET downloader
  
  #define SOURCES							\
    asyncUtility.I asyncUtility.cxx asyncUtility.h			\
    config_downloader.cxx						\
    config_downloader.h downloadDb.I					\
    downloadDb.cxx downloadDb.h						\
    downloader.I downloader.cxx downloader.h extractor.cxx extractor.h	\
    patcher.cxx								\
    patcher.h

  #define IF_ZLIB_SOURCES						\
    decompressor.cxx decompressor.h zcompressor.I zcompressor.cxx	\
    zcompressor.h download_utils.cxx download_utils.h

  #define INSTALL_HEADERS					\
    downloader.h downloader.I					\
    config_downloader.h zcompressor.I zcompressor.h		\
    asyncUtility.h asyncUtility.I decompressor.h		\
    extractor.h download_utils.h downloadDb.h downloadDb.I	\
    patcher.h

  #define IGATESCAN all

#end lib_target
