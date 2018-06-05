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
  foreach(srcfile ${files})
    file(RELATIVE_PATH srcfile_rel "${destination}" "${source}/${srcfile}")
    file(RELATIVE_PATH dstfile_rel "${destination}" "${destination}/${srcfile}.pz")

    list(APPEND dstfiles "${dstfile_rel}")
    add_custom_command(OUTPUT "${dstfile_rel}"
      COMMAND pzip -c > "${dstfile_rel}" < "${srcfile_rel}"
      WORKING_DIRECTORY "${destination}"
      DEPENDS pzip
      COMMENT "")

    get_filename_component(dstdir "${destination}/${dstfile_rel}" DIRECTORY)
    file(MAKE_DIRECTORY "${dstdir}")
  endforeach(srcfile)

  add_custom_target(${target_name} ALL
    DEPENDS ${dstfiles}
    WORKING_DIRECTORY "${destination}")
endfunction(run_pzip)
