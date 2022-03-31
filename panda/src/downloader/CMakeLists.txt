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
  extractor.h extractor.I
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
  urlSpec.cxx
  virtualFileHTTP.cxx
  virtualFileMountHTTP.cxx
)

composite_sources(p3downloader P3DOWNLOADER_SOURCES)
add_component_library(p3downloader SYMBOL BUILDING_PANDA_DOWNLOADER
  ${P3DOWNLOADER_HEADERS} ${P3DOWNLOADER_SOURCES})
target_link_libraries(p3downloader p3express)
target_interrogate(p3downloader ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3downloader
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DOWNLOADER_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
