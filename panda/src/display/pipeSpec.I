// Filename: pipeSpec.I
// Created by:  frang (07Mar99)
// 
////////////////////////////////////////////////////////////////////

INLINE void PipeSpecifier::set_name(const std::string& name) {
  _name = name;
}

INLINE std::string PipeSpecifier::get_name(void) const {
  return _name;
}

INLINE void PipeSpecifier::unset_machine(void) {
  if (!_is_file) {
    _is_file = true;
    _machine = "";
    _is_remote = false;
  }
}

INLINE void PipeSpecifier::unset_file(void) {
  if (_is_file) {
    _is_file = false;
    _filename = "";
  }
}

INLINE void PipeSpecifier::set_machine_name(const std::string& name) {
  _machine = name;
  if (_machine.empty())
    _is_remote = false;
  else
    _is_remote = true;
  unset_file();
}

INLINE std::string PipeSpecifier::get_machine_name(void) const {
  return _machine;
}

INLINE void PipeSpecifier::set_file_name(const std::string& name) {
  _filename = name;
  unset_machine();
}

INLINE std::string PipeSpecifier::get_file_name(void) const {
  return _filename;
}

INLINE void PipeSpecifier::set_pipe_number(const int p) {
  _pipe_number = p;
}

INLINE int PipeSpecifier::get_pipe_number(void) const {
  return _pipe_number;
}

INLINE bool PipeSpecifier::is_file(void) const {
  return _is_file;
}

INLINE bool PipeSpecifier::is_remote(void) const {
  return _is_remote;
}
