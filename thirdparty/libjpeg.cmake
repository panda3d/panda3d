# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec, David Manura
# Modified by rdb to not depend on cmake/dist.cmake
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( libjpeg C )
cmake_minimum_required ( VERSION 2.8 )

option(BUILD_SHARED_LIBS "Build shared instead of static library." OFF)

include ( CheckIncludeFile )
check_include_file ( stddef.h HAVE_STDDEF_H )
check_include_file ( stdlib.h HAVE_STDLIB_H )
if ( WIN32 AND NOT CYGWIN )
  #improve? see jconfig.*
  set ( TWO_FILE_COMMANDLINE true )
  # jconfig.h
endif ( )
configure_file ( jconfig.h.cmake jconfig.h )

include_directories ( ${CMAKE_CURRENT_BINARY_DIR} )
# jconfig.h

set ( HEADERS jerror.h jmorecfg.h jpeglib.h ${CMAKE_CURRENT_BINARY_DIR}/jconfig.h )

set ( SRC jmemnobs.c jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c 
  jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c 
  jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c jdarith.c jdatadst.c jdatasrc.c 
  jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c 
  jdmerge.c jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c jfdctint.c 
  jidctflt.c jidctfst.c jidctint.c jquant1.c jquant2.c jutils.c jmemmgr.c cderror.h 
  cdjpeg.h jdct.h jinclude.h jmemsys.h jpegint.h jversion.h transupp.h )

if(MSVC AND NOT BUILD_SHARED_LIBS)
  add_library(jpeg-static STATIC ${SRC} ${HEADERS})
  install(TARGETS jpeg-static DESTINATION lib)
else()
  add_library (jpeg ${SRC} ${HEADERS})
  install(TARGETS jpeg DESTINATION lib)
endif()

install(FILES ${HEADERS} DESTINATION include)
