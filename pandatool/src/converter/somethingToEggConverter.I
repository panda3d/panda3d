// Filename: somethingToEggConverter.I
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::set_texture_path_convert
//       Access: Public
//  Description: Sets the mode for converting texture paths extracted
//               from the source file.  The following options are
//               defined:
//
//                 PC_relative - texture pathnames are made relative
//                 to the indicated tpc_directory.
//
//                 PC_absolute - textures are looked up along the
//                 search path and then converted to absolute
//                 filenames.  tpc_directory is ignored.
//
//                 PC_rel_abs - as above, but the resulting filename
//                 is guaranteed to be relative to tpc_directory, and
//                 may include a number of ../ paths to guarantee
//                 this, if necessary.
//
//                 PC_strip - pathname components are stripped from
//                 the texture names, and only the base filename is
//                 retained.
//
//                 PC_unchanged - texture pathnames are left exactly
//                 as they appear in the source file.
////////////////////////////////////////////////////////////////////
INLINE void SomethingToEggConverter::
set_texture_path_convert(PathConvert tpc,
			 const Filename &tpc_directory) {
  _tpc = tpc;
  _tpc_directory = tpc_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::set_model_path_convert
//       Access: Public
//  Description: Sets the mode for converting model paths extracted
//               from the source file.  These are typically for
//               external references.  See set_texture_path_convert
//               for a complete list of PathConvert values and their
//               interpretations.
////////////////////////////////////////////////////////////////////
INLINE void SomethingToEggConverter::
set_model_path_convert(PathConvert mpc,
		       const Filename &mpc_directory) {
  _mpc = mpc;
  _mpc_directory = mpc_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::set_merge_externals
//       Access: Public
//  Description: Sets the merge_externals flag.  When this is true,
//               external references within the source file are read
//               in and merged directly; otherwise, only a reference
//               to a similarly-named egg file is inserted.
////////////////////////////////////////////////////////////////////
INLINE void SomethingToEggConverter::
set_merge_externals(bool merge_externals) {
  _merge_externals = merge_externals;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::get_merge_externals
//       Access: Public
//  Description: Returns the current state of the merge_externals
//               flag.  See set_merge_externals().
////////////////////////////////////////////////////////////////////
INLINE bool SomethingToEggConverter::
get_merge_externals() const {
  return _merge_externals;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::clear_egg_data
//       Access: Public
//  Description: Sets the EggData to NULL and makes the converter
//               invalid.
////////////////////////////////////////////////////////////////////
INLINE void SomethingToEggConverter::
clear_egg_data() {
  set_egg_data((EggData *)NULL, false);
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::get_egg_data
//       Access: Public
//  Description: Returns the EggData structure.
////////////////////////////////////////////////////////////////////
INLINE EggData &SomethingToEggConverter::
get_egg_data() {
  return *_egg_data;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::handle_external_reference
//       Access: Public
//  Description: Handles an external reference in the source file.  If
//               the merge_externals flag is true (see
//               set_merge_externals()), this causes the named file to
//               be read in and converted, and the converted egg
//               geometry is parented to egg_parent.  Otherwise, only
//               a reference to a similarly named egg file is parented
//               to egg_parent.
//
//               The parameters orig_filename and searchpath are as
//               those passed to convert_model_path().
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool SomethingToEggConverter::
handle_external_reference(EggGroupNode *egg_parent,
			  const Filename &orig_filename) {
  return handle_external_reference(egg_parent, orig_filename, get_model_path());
}


////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_texture_path
//       Access: Public
//  Description: Converts the indicated texture filename to a relative
//               or absolute or whatever filename, according to _tpc
//               and _tpc_directory.  See convert_path().
////////////////////////////////////////////////////////////////////
INLINE Filename SomethingToEggConverter::
convert_texture_path(const Filename &orig_filename) {
  return convert_path(orig_filename, get_texture_path(), _tpc_directory, _tpc);
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_texture_path
//       Access: Public
//  Description: Converts the indicated texture filename to a relative
//               or absolute or whatever filename, according to _tpc
//               and _tpc_directory.  See convert_path().
////////////////////////////////////////////////////////////////////
INLINE Filename SomethingToEggConverter::
convert_texture_path(const Filename &orig_filename,
		     const DSearchPath &searchpath) {
  return convert_path(orig_filename, searchpath, _tpc_directory, _tpc);
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_model_path
//       Access: Public
//  Description: Converts the indicated model filename to a relative
//               or absolute or whatever filename, according to _mpc
//               and _mpc_directory.  See convert_path().
////////////////////////////////////////////////////////////////////
INLINE Filename SomethingToEggConverter::
convert_model_path(const Filename &orig_filename) {
  return convert_path(orig_filename, get_model_path(), _mpc_directory, _mpc);
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_model_path
//       Access: Public
//  Description: Converts the indicated model filename to a relative
//               or absolute or whatever filename, according to _mpc
//               and _mpc_directory.  See convert_path().
////////////////////////////////////////////////////////////////////
INLINE Filename SomethingToEggConverter::
convert_model_path(const Filename &orig_filename,
		   const DSearchPath &searchpath) {
  return convert_path(orig_filename, searchpath, _mpc_directory, _mpc);
}
