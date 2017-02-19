set(P3DOWNLOADER_HEADERS
  bioPtr.I bioPtr.h
  bioStreamPtr.I bioStreamPtr.h
  bioStream.I bioStream.h bioStreamBuf.h
  chunkedStream.I chunkedStream.h
  chunkedStreamBuf.h chunkedStreamBuf.I
  config_downloader.h
  decompressor.h decompressor.I
  documentSpec.h documentSpec.I
  download_utils.h downloadDb.h downloadDb.I
  extractor.h
  httpAuthorization.I httpAuthorization.h
  httpBasicAuthorization.I httpBasicAuthorization.h
  httpChannel.I httpChannel.h
  httpClient.I httpClient.h
  httpCookie.I httpCookie.h
  httpDate.I httpDate.h
  httpDigestAuthorization.I httpDigestAuthorization.h
  httpEntityTag.I httpEntityTag.h
  httpEnum.h
  identityStream.I identityStream.h
  identityStreamBuf.h identityStreamBuf.I
  multiplexStream.I multiplexStream.h
  multiplexStreamBuf.I multiplexStreamBuf.h
  patcher.h patcher.I
  socketStream.h socketStream.I
  stringStreamBuf.I stringStreamBuf.h
  stringStream.I stringStream.h
  urlSpec.h urlSpec.I
  virtualFileHTTP.I virtualFileHTTP.h
  virtualFileMountHTTP.I virtualFileMountHTTP.h
)

set(P3DOWNLOADER_SOURCES
  config_downloader.cxx
  bioPtr.cxx
  bioStreamPtr.cxx
  bioStream.cxx bioStreamBuf.cxx
  chunkedStream.cxx chunkedStreamBuf.cxx
  decompressor.cxx
  documentSpec.cxx
  downloadDb.cxx
  download_utils.cxx
  extractor.cxx
  httpAuthorization.cxx
  httpBasicAuthorization.cxx
  httpChannel.cxx
  httpClient.cxx
  httpCookie.cxx
  httpDate.cxx
  httpDigestAuthorization.cxx
  httpEntityTag.cxx
  httpEnum.cxx
  identityStream.cxx identityStreamBuf.cxx
  multiplexStream.cxx multiplexStreamBuf.cxx
  patcher.cxx
  socketStream.cxx
  stringStreamBuf.cxx
  stringStream.cxx
  urlSpec.cxx
  virtualFileHTTP.cxx
  virtualFileMountHTTP.cxx
)

set(P3DOWNLOADER_IGATEEXT
  stringStream_ext.cxx
  stringStream_ext.h
)

composite_sources(p3downloader P3DOWNLOADER_SOURCES)
add_library(p3downloader ${P3DOWNLOADER_HEADERS} ${P3DOWNLOADER_SOURCES})
target_link_libraries(p3downloader p3express p3pipeline)
target_interrogate(p3downloader ALL EXTENSIONS ${P3DOWNLOADER_IGATEEXT})

install(TARGETS p3downloader DESTINATION lib)
install(FILES ${P3DOWNLOADER_HEADERS} DESTINATION include/panda3d)
