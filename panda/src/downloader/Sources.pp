#define LOCAL_LIBS event express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES zlib net ssl

#begin lib_target
  #define TARGET downloader

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx \
    $[if $[HAVE_NET], $[TARGET]_composite3.cxx] \
    $[if $[HAVE_ZLIB], $[TARGET]_composite4.cxx] \

  #define SOURCES \
    config_downloader.h \
    asyncUtility.I asyncUtility.h \
    bioStream.I bioStream.h bioStreamBuf.h \
    chunkedStream.I chunkedStream.h chunkedStreamBuf.h \
    extractor.h \
    httpClient.I httpClient.h \
    httpDocument.I httpDocument.h \
    multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.h \
    urlSpec.I urlSpec.h \
    $[if $[HAVE_NET], downloadDb.I downloadDb.h downloader.I downloader.h] \
    $[if $[HAVE_ZLIB], decompressor.h zcompressor.I zcompressor.h download_utils.h] \
    $[if $[HAVE_CRYPTO], patcher.cxx patcher.h patcher.I]
    
  #define INCLUDED_SOURCES                 \
    config_downloader.cxx \
    asyncUtility.cxx \
    bioStream.cxx bioStreamBuf.cxx \
    chunkedStream.cxx chunkedStreamBuf.cxx \
    extractor.cxx \
    httpClient.cxx \
    httpDocument.cxx \
    multiplexStream.cxx multiplexStreamBuf.cxx \
    urlSpec.cxx \
    $[if $[HAVE_NET], downloadDb.cxx downloader.cxx] \
    $[if $[HAVE_ZLIB], decompressor.cxx zcompressor.cxx download_utils.cxx]

  #define INSTALL_HEADERS \
    asyncUtility.h asyncUtility.I \
    bioStream.I bioStream.h bioStreamBuf.h \
    chunkedStream.I chunkedStream.h chunkedStreamBuf.h \
    config_downloader.h \
    decompressor.h \
    download_utils.h downloadDb.h downloadDb.I \
    downloader.h downloader.I \
    extractor.h \
    httpClient.I httpClient.h \
    httpDocument.I httpDocument.h \
    multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.I \
    patcher.h patcher.I \
    urlSpec.h urlSpec.I \
    zcompressor.I zcompressor.h
    
  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_proxy

  #define SOURCES test_proxy.cxx
  #define LOCAL_LIBS $[LOCAL_LIBS] putil net
  #define OTHER_LIBS $[OTHER_LIBS] pystub
#end test_bin_target
