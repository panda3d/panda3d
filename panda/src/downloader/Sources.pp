#define LOCAL_LIBS event express pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES zlib net ssl

#begin lib_target
  #define TARGET downloader

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx \
    $[if $[HAVE_NET], $[TARGET]_composite3.cxx] \
    $[if $[HAVE_ZLIB], $[TARGET]_composite4.cxx] \

  #define SOURCES \
    config_downloader.h \
    asyncUtility.I asyncUtility.h \
    bioPtr.I bioPtr.h \
    bioStreamPtr.I bioStreamPtr.h \
    bioStream.I bioStream.h bioStreamBuf.h \
    chunkedStream.I chunkedStream.h chunkedStreamBuf.h \
    documentSpec.I documentSpec.h \
    extractor.h \
    httpAuthorization.I httpAuthorization.h \
    httpBasicAuthorization.I httpBasicAuthorization.h \
    httpClient.I httpClient.h \
    httpChannel.I httpChannel.h \
    httpDate.I httpDate.h \
    httpDigestAuthorization.I httpDigestAuthorization.h \
    httpEntityTag.I httpEntityTag.h \
    httpEnum.h \
    identityStream.I identityStream.h identityStreamBuf.h \
    multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.h \
    socketStream.h socketStream.I \
    urlSpec.I urlSpec.h \
    $[if $[HAVE_NET], downloadDb.I downloadDb.h] \
    $[if $[HAVE_ZLIB], decompressor.h download_utils.h] \
    $[if $[HAVE_CRYPTO], patcher.cxx patcher.h patcher.I]
    
  #define INCLUDED_SOURCES                 \
    config_downloader.cxx \
    asyncUtility.cxx \
    bioPtr.cxx \
    bioStreamPtr.cxx \
    bioStream.cxx bioStreamBuf.cxx \
    chunkedStream.cxx chunkedStreamBuf.cxx \
    documentSpec.cxx \
    extractor.cxx \
    httpAuthorization.cxx \
    httpBasicAuthorization.cxx \
    httpClient.cxx \
    httpChannel.cxx \
    httpDate.cxx \
    httpDigestAuthorization.cxx \
    httpEntityTag.cxx \
    httpEnum.cxx \
    identityStream.cxx identityStreamBuf.cxx \
    multiplexStream.cxx multiplexStreamBuf.cxx \
    socketStream.cxx \
    urlSpec.cxx \
    $[if $[HAVE_NET], downloadDb.cxx] \
    $[if $[HAVE_ZLIB], decompressor.cxx download_utils.cxx]

  #define INSTALL_HEADERS \
    asyncUtility.h asyncUtility.I \
    bioPtr.I bioPtr.h \
    bioStreamPtr.I bioStreamPtr.h \
    bioStream.I bioStream.h bioStreamBuf.h \
    chunkedStream.I chunkedStream.h chunkedStreamBuf.h \
    config_downloader.h \
    decompressor.h \
    documentSpec.h documentSpec.I \
    download_utils.h downloadDb.h downloadDb.I \
    extractor.h \
    httpAuthorization.I httpAuthorization.h \
    httpBasicAuthorization.I httpBasicAuthorization.h \
    httpClient.I httpClient.h \
    httpChannel.I httpChannel.h \
    httpDate.I httpDate.h \
    httpDigestAuthorization.I httpDigestAuthorization.h \
    httpEntityTag.I httpEntityTag.h \
    httpEnum.h \
    identityStream.I identityStream.h identityStreamBuf.h \
    multiplexStream.I multiplexStream.h \
    multiplexStreamBuf.I multiplexStreamBuf.h \
    patcher.h patcher.I \
    socketStream.h socketStream.I \
    urlSpec.h urlSpec.I
    
  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_proxy

  #define SOURCES test_proxy.cxx
  #define LOCAL_LIBS $[LOCAL_LIBS] putil net
  #define OTHER_LIBS $[OTHER_LIBS] pystub
#end test_bin_target

// This is a handy file for identifying public certificate authorities
// (and thus almost any public https server).  It was taken from the
// OpenSSL distribution; if you need a fresher copy, go get a new one
// from the latest OpenSSL.  To use this file, point the variable
// ssl-certificates in your Configrc file to its installed location,
// e.g:
//
// ssl-certificates /usr/local/panda/install/shared/ca-bundle.crt
//
#define INSTALL_DATA ca-bundle.crt
