# Filename: FindOpenCV.cmake
# Authors: CFSworks (3 Nov, 2018)
#
# Usage:
#   find_package(OpenCV [REQUIRED] [QUIET])
#
# This supports the following components:
#   calib3d
#   contrib
#   core
#   features2d
#   flann
#   gpu
#   highgui
#   imgproc
#   legacy
#   ml
#   nonfree
#   objdetect
#   photo
#   stitching
#   superres
#   video
#   videostab
#
# Once done this will define:
#   OPENCV_FOUND        - system has OpenCV
#   OpenCV_INCLUDE_DIRS - the include dir(s) containing OpenCV header files
#   OpenCV_comp_LIBRARY - the path to the OpenCV library for the particular
#                         component
#   OpenCV_LIBS         - the paths to the OpenCV libraries for the requested
#                         component(s)
#

set(OpenCV_INCLUDE_DIRS)
if(NOT OpenCV_V1_INCLUDE_DIR)
  find_path(OpenCV_V1_INCLUDE_DIR
    NAMES "cv.h"
    PATH_SUFFIXES "opencv")

  mark_as_advanced(OpenCV_V1_INCLUDE_DIR)
endif()
if(OpenCV_V1_INCLUDE_DIR)
  list(APPEND OpenCV_INCLUDE_DIRS "${OpenCV_V1_INCLUDE_DIR}")
endif()

if(NOT OpenCV_V2_INCLUDE_DIR)
  find_path(OpenCV_V2_INCLUDE_DIR "opencv2/core/core.hpp")

  mark_as_advanced(OpenCV_V2_INCLUDE_DIR)
endif()
if(OpenCV_V2_INCLUDE_DIR)
  list(APPEND OpenCV_INCLUDE_DIRS "${OpenCV_V2_INCLUDE_DIR}")
endif()

set(OpenCV_LIBS)
foreach(_component calib3d contrib core features2d flann gpu highgui imgproc
                  legacy ml nonfree objdetect photo stitching superres video
                  videostab)

  list(FIND OpenCV_FIND_COMPONENTS "${_component}" _index)
  if(_index GREATER -1 OR _component STREQUAL "core")
    if(NOT OpenCV_${_component}_LIBRARY)
      find_library(OpenCV_${_component}_LIBRARY
        NAMES "opencv_${_component}")
    endif()

    if(OpenCV_${_component}_LIBRARY)
      list(APPEND OpenCV_LIBS "${OpenCV_${_component}_LIBRARY}")
      set(OpenCV_${_component}_FOUND ON)
    endif()
  endif()
  unset(_index)
endforeach(_component)
unset(_component)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCV HANDLE_COMPONENTS
  REQUIRED_VARS OpenCV_INCLUDE_DIRS OpenCV_LIBS)
