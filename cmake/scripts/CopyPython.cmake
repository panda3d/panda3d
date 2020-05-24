# Filename: CopyPython.cmake
#
# Description: When run, copies Python files from a source directory to a
#   build directory located somewhere in the project's binary path. If
#   PYTHON_EXECUTABLES is provided, it will also invoke compileall in the
#   destination dir(s) to precache .pyc/.pyo files.
#
# Usage:
#   This script is invoked via add_custom_target, like this:
#   cmake -D OUTPUT_DIR="/out/dir/Release"
#         -D SOURCE_DIR="/panda3d/direct/src/"
#         -D PYTHON_EXECUTABLES="/usr/bin/python3.6"
#         -P CopyPython.cmake
#

if(NOT CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "CopyPython.cmake should not be included but run in script mode.")
  return()
endif()

if(NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "OUTPUT_DIR should be defined when running CopyPython.cmake!")
  return()
endif()

# Ensure OUTPUT_DIR exists
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# If there's a SOURCE_DIR, glob for .py files and copy
# (this is done by hand to avoid the CMake bug where it creates empty dirs)
if(DEFINED SOURCE_DIR)
  file(GLOB_RECURSE py_files RELATIVE "${SOURCE_DIR}" "${SOURCE_DIR}/*.py")
  foreach(py_file ${py_files})
    get_filename_component(py_file_parent "${py_file}" DIRECTORY)
    file(TIMESTAMP "${SOURCE_DIR}/${py_file}" src_stamp)
    file(TIMESTAMP "${OUTPUT_DIR}/${py_file}" dst_stamp)

    # The file is only copied if:
    # - there's an __init__.py in its dir (or file is in the root) (i.e. file belongs to a package), and
    # - the modification timestamp differs (i.e. file changed or never copied)
    if((py_file_parent STREQUAL "." OR NOT py_file_parent
        OR EXISTS "${SOURCE_DIR}/${py_file_parent}/__init__.py")
       AND NOT src_stamp STREQUAL dst_stamp)

      file(COPY "${SOURCE_DIR}/${py_file}" DESTINATION "${OUTPUT_DIR}/${py_file_parent}")
      set(changed YES)
    endif()

  endforeach(py_file)

else()
  # No SOURCE_DIR; assume we're outdated since our caller might be populating
  # the OUTPUT_DIR themselves
  set(changed YES)

endif()

# Make sure Python recognizes OUTPUT_DIR as a package
if(NOT EXISTS "${OUTPUT_DIR}/__init__.py")
  file(WRITE "${OUTPUT_DIR}/__init__.py" "")
  set(changed YES)
endif()

# Generate .pyc files for each Python version, if our caller wants
if(changed AND DEFINED PYTHON_EXECUTABLES)
  foreach(interp ${PYTHON_EXECUTABLES})
    execute_process(
      COMMAND "${interp}" -m compileall .
      OUTPUT_QUIET
      WORKING_DIRECTORY "${OUTPUT_DIR}")

    execute_process(
      COMMAND "${interp}" -O -m compileall .
      OUTPUT_QUIET
      WORKING_DIRECTORY "${OUTPUT_DIR}")

  endforeach(interp)
endif()
