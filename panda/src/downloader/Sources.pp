#define LOCAL_LIBS event ipc express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_ZLIB yes
#define USE_IPC yes
#define USE_NET yes

#begin lib_target
  #define TARGET downloader
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_downloader.h asyncUtility.I asyncUtility.h \
    extractor.h  multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.h 
    
  #define INCLUDED_SOURCES                 \
    config_downloader.cxx asyncUtility.cxx \
    extractor.cxx multiplexStream.cxx multiplexStreamBuf.cxx

  #define IF_NET_SOURCES \
    downloadDb.I downloadDb.h downloader.I downloader.h
    
  #define IF_NET_INCLUDED_SOURCES \
    downloadDb.cxx downloader.cxx 
    
  #define IF_NET_COMBINED_SOURCES \ 
    $[TARGET]_composite3.cxx        

  #define IF_ZLIB_SOURCES \
    decompressor.h zcompressor.I zcompressor.h download_utils.h
    
  #define IF_ZLIB_COMBINED_SOURCES \    
    $[TARGET]_composite4.cxx    
    
  #define IF_ZLIB_INCLUDED_SOURCES \
    decompressor.cxx zcompressor.cxx download_utils.cxx

  #define IF_CRYPTO_SOURCES \
    patcher.cxx                             \
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
