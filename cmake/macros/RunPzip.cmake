function(run_pzip target_name source destination glob)
  if(NOT TARGET pzip)
    # If pzip isn't built, we just copy instead.
    file(COPY "${source}"
      DESTINATION "${destination}"
      FILES_MATCHING PATTERN "${glob}")
    return()
  endif()

  file(GLOB_RECURSE files RELATIVE "${source}" "${source}/${glob}")

  set(dstfiles "")
  foreach(filename ${files})
    string(REGEX REPLACE "^/" "" filename "${filename}")
    file(RELATIVE_PATH srcfile "${destination}" "${source}/${filename}")

    get_filename_component(dstdir "${destination}/${filename}" DIRECTORY)
    file(MAKE_DIRECTORY "${dstdir}")

    set(dstfile "${filename}.pz")
    list(APPEND dstfiles "${destination}/${dstfile}")

    add_custom_command(OUTPUT "${destination}/${dstfile}"
      COMMAND pzip -c > "${dstfile}" < "${srcfile}"
      WORKING_DIRECTORY "${destination}"
      DEPENDS pzip
      COMMENT "")

  endforeach(filename)

  add_custom_target(${target_name} ALL
    DEPENDS ${dstfiles}
    WORKING_DIRECTORY "${destination}")
endfunction(run_pzip)
