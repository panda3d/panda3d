function(set_from_setup_metadata dst_var metadata_item)
  set(_s "[\\t ]*") # CMake doesn't support \s*
  file(STRINGS "setup.cfg" _output REGEX "^${metadata_item}${_s}=${_s}")
  string(REGEX REPLACE "^.*=${_s}" "" _output "${_output}")
  set(${dst_var} ${_output} PARENT_SCOPE)
endfunction()
