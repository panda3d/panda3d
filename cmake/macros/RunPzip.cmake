function(run_pzip target_name source destination glob)
  if(NOT TARGET host_pzip)
    if(CMAKE_CROSSCOMPILING AND NOT EXISTS "${HOST_BIN_DIR}/pzip")
      # If pzip isn't built, we just copy instead.
      file(COPY "${source}"
        DESTINATION "${destination}"
        FILES_MATCHING PATTERN "${glob}")

      return()
    endif()
  endif()

  file(GLOB_RECURSE files RELATIVE "${source}" "${source}/${glob}")

  if(CMAKE_CROSSCOMPILING)
    set(pzip_executable ${HOST_BIN_DIR}/pzip)
  else()
    set(pzip_executable host_pzip)
  endif()

  set(dstfiles "")
  foreach(filename ${files})
    string(REGEX REPLACE "^/" "" filename "${filename}")

    get_filename_component(dstdir "${destination}/${filename}" DIRECTORY)

    set(dstfile "${filename}.pz")
    list(APPEND dstfiles "${destination}/${dstfile}")

    add_custom_command(OUTPUT "${destination}/${dstfile}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${dstdir}"
      COMMAND ${pzip_executable} -c > "${destination}/${dstfile}" < "${source}/${filename}"
      DEPENDS ${pzip_executable}
      COMMENT "")

  endforeach(filename)

  add_custom_target(${target_name} ALL
    DEPENDS ${dstfiles}
    WORKING_DIRECTORY "${destination}")

endfunction(run_pzip)
