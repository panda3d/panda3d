// Filename: bamFile.I
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: BamFile::is_valid_read
//       Access: Public
//  Description: Returns true if the Bam file is open and ready for
//               reading with no errors so far detected, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BamFile::
is_valid_read() const {
  return (_reader != (BamReader *)NULL);
}


////////////////////////////////////////////////////////////////////
//     Function: BamFile::is_valid_write
//       Access: Public
//  Description: Returns true if the Bam file is open and ready for
//               writing with no errors so far detected, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BamFile::
is_valid_write() const {
  return (_writer != (BamWriter *)NULL);
}

