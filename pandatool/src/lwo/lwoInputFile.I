// Filename: lwoInputFile.I
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LwoInputFile::get_lwo_version
//       Access: Public
//  Description: Returns the version of the Lightwave file being read.
//               This is unknown until the header record has been
//               read; then it will be set by the header.
////////////////////////////////////////////////////////////////////
INLINE double LwoInputFile::
get_lwo_version() const {
  return _lwo_version;
}


////////////////////////////////////////////////////////////////////
//     Function: LwoInputFile::set_lwo_version
//       Access: Public
//  Description: Changes the version number reported for the Lightwave
//               file.  Normally this is only called by LwoHeader as
//               it is read.
////////////////////////////////////////////////////////////////////
INLINE void LwoInputFile::
set_lwo_version(double lwo_version) {
  _lwo_version = lwo_version;
}

