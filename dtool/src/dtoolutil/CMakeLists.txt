set(P3DTOOLUTIL_HEADERS
  config_dtoolutil.h
  dSearchPath.I dSearchPath.h
  executionEnvironment.I executionEnvironment.h filename.I
  filename.h
  globPattern.I globPattern.h
  lineStream.I lineStream.h
  lineStreamBuf.I lineStreamBuf.h
  load_dso.h
  pandaFileStream.h pandaFileStream.I
  pandaFileStreamBuf.h
  pandaSystem.h
  panda_getopt.h panda_getopt_long.h panda_getopt_impl.h
  pfstream.h pfstream.I pfstreamBuf.h
  preprocess_argv.h
  small_vector.h small_vector.I
  string_utils.h string_utils.I
  stringDecoder.h stringDecoder.I
  textEncoder.h textEncoder.I
  unicodeLatinMap.h
  vector_double.h
  vector_float.h
  vector_int.h
  vector_stdfloat.h
  vector_string.h
  vector_uchar.h
  vector_src.h
  win32ArgParser.h
)

if(APPLE)
  set(P3DTOOLUTIL_HEADERS ${P3DTOOLUTIL_HEADERS}
    filename_assist.mm filename_assist.h)

  set_source_files_properties(
    filename_assist.mm filename_assist.h PROPERTIES
    WRAP_EXCLUDE YES
    SKIP_UNITY_BUILD_INCLUSION YES)
endif()

set(P3DTOOLUTIL_SOURCES
  config_dtoolutil.cxx
  dSearchPath.cxx
  executionEnvironment.cxx filename.cxx
  globPattern.cxx
  lineStream.cxx lineStreamBuf.cxx
  load_dso.cxx
  pandaFileStream.cxx pandaFileStreamBuf.cxx
  pandaSystem.cxx
  panda_getopt_impl.cxx
  pfstreamBuf.cxx pfstream.cxx
  preprocess_argv.cxx
  string_utils.cxx
  stringDecoder.cxx
  textEncoder.cxx
  unicodeLatinMap.cxx
  vector_double.cxx
  vector_float.cxx
  vector_int.cxx
  vector_string.cxx
  vector_uchar.cxx
  win32ArgParser.cxx
)

set(P3DTOOLUTIL_IGATEEXT
  filename_ext.cxx
  filename_ext.h
  globPattern_ext.cxx
  globPattern_ext.h
  iostream_ext.cxx
  iostream_ext.h
  pyenv_init.cxx
  pyenv_init.h
  textEncoder_ext.cxx
  textEncoder_ext.h
)

composite_sources(p3dtoolutil P3DTOOLUTIL_SOURCES)
add_component_library(p3dtoolutil SYMBOL BUILDING_DTOOL_DTOOLUTIL
  ${P3DTOOLUTIL_HEADERS} ${P3DTOOLUTIL_SOURCES})
target_link_libraries(p3dtoolutil p3dtoolbase ${CMAKE_DL_LIBS})
target_interrogate(p3dtoolutil ALL EXTENSIONS ${P3DTOOLUTIL_IGATEEXT})

if(APPLE)
  find_library(FOUNDATION_LIBRARY Foundation)
  find_library(APPKIT_LIBRARY AppKit)
  target_link_libraries(p3dtoolutil ${FOUNDATION_LIBRARY} ${APPKIT_LIBRARY})

  mark_as_advanced(FOUNDATION_LIBRARY APPKIT_LIBRARY)
endif()

# These are all used by executionEnvironment.cxx/filename.cxx
foreach(var
    # executionEnvironment.cxx:
    HAVE_GLOBAL_ARGV
    PROTOTYPE_GLOBAL_ARGV
    GLOBAL_ARGV
    GLOBAL_ARGC

    HAVE_PROC_CURPROC_CMDLINE
    HAVE_PROC_CURPROC_FILE
    HAVE_PROC_CURPROC_MAP
    HAVE_PROC_SELF_CMDLINE
    HAVE_PROC_SELF_ENVIRON
    HAVE_PROC_SELF_EXE
    HAVE_PROC_SELF_MAPS

    STATIC_INIT_GETENV

    # filename.cxx:
    HAVE_IOS_BINARY
    PHAVE_DIRENT_H
    PHAVE_GLOB_H
    PHAVE_LOCKF
    PHAVE_UTIME_H
)

  if(${var})
    target_compile_definitions(p3dtoolutil PRIVATE "${var}=${${var}}")
  endif()

endforeach(var)

if(BUILD_SHARED_LIBS)
  # executionEnvironment.cxx needs to know the name(s) of the library file it
  # will be linked into
  if(BUILD_METALIBS)
    set(library "p3dtool")
  else()
    set(library "p3dtoolutil")
  endif()

  set(filenames)
  foreach(prop TARGET_FILE_NAME TARGET_SONAME_FILE_NAME TARGET_LINKER_FILE_NAME)
    list(APPEND filenames "$<${prop}:${library}>")

    if(CMAKE_IMPORT_LIBRARY_SUFFIX)
      # Only the first property is valid/relevant on DLL platforms; bail out
      # now and skip the others.
      # (Yes, CMAKE_IMPORT_LIBRARY_SUFFIX being non-empty is how CMake defines
      #  a DLL platform where TARGET_SONAME_FILE_NAME is invalid.)
      break()
    endif()
  endforeach(prop)

  target_compile_definitions(p3dtoolutil
    PRIVATE "LIBP3DTOOL_FILENAMES=\"$<JOIN:${filenames},\"$<COMMA>\">\"")
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3dtoolutil
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DTOOLUTIL_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
