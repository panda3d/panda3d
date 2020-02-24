function(run_pzip target_name source destination glob)
  file(GLOB_RECURSE files RELATIVE "${source}" "${source}/${glob}")

  set(dstfiles "")
  foreach(filename ${files})
    string(REGEX REPLACE "^/" "" filename "${filename}")

    get_filename_component(dstdir "${destination}/${filename}" DIRECTORY)

    if(TARGET host_pzip)
      set(dstfile "${filename}.pz")
      list(APPEND dstfiles "${destination}/${dstfile}")

      add_custom_command(OUTPUT "${destination}/${dstfile}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dstdir}"
        COMMAND host_pzip -c > "${destination}/${dstfile}" < "${source}/${filename}"
        DEPENDS host_pzip
        COMMENT "")

    else()
      # If pzip isn't built, we just copy instead.
      list(APPEND dstfiles "${destination}/${filename}")

      add_custom_command(OUTPUT "${destination}/${filename}"
        COMMAND ${CMAKE_COMMAND} -E
          copy_if_different "${source}/${filename}" "${destination}/${filename}"
        COMMENT "")

    endif()

  endforeach(filename)

  add_custom_target(${target_name} ALL
    DEPENDS ${dstfiles}
    WORKING_DIRECTORY "${destination}")

endfunction(run_pzip)
