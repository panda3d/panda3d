// Filename: fltToEggConverter.I
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FltToEggConverter::
~FltToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::set_texture_path_convert
//       Access: Public
//  Description: Sets the mode for converting texture paths extracted
//               from the flt file.  If this is PC_relative, the
//               texture paths are made relative to the indicated
//               directory; if it is PC_absolute, they are made
//               absolute to the indicated directory.  PC_unchanged
//               leaves them alone (and the directory, if specified,
//               is ignored).
////////////////////////////////////////////////////////////////////
INLINE void FltToEggConverter::
set_texture_path_convert(PathConvert tpc,
			 const Filename &tpc_directory) {
  _tpc = tpc;
  _tpc_directory = tpc_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::set_model_path_convert
//       Access: Public
//  Description: Sets the mode for converting model paths extracted
//               from the flt file.  If this is PC_relative, the
//               model paths are made relative to the indicated
//               directory; if it is PC_absolute, they are made
//               absolute to the indicated directory.  PC_unchanged
//               leaves them alone (and the directory, if specified,
//               is ignored).
////////////////////////////////////////////////////////////////////
INLINE void FltToEggConverter::
set_model_path_convert(PathConvert mpc,
		       const Filename &mpc_directory) {
  _mpc = mpc;
  _mpc_directory = mpc_directory;
}
