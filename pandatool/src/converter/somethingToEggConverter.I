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
