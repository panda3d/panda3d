// Filename: datagramInputFile.I
// Created by:  drose (30Oct00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DatagramInputFile::
DatagramInputFile() {
  _error = true;
  _read_first_datagram = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramInputFile::
open(Filename filename) {
  // DatagramInputFiles are always binary.
  _read_first_datagram = false;
  _error = false;
  filename.set_binary();
  return filename.open_read(_in);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramInputFile destructs.
////////////////////////////////////////////////////////////////////
INLINE void DatagramInputFile::
close() {
  _in.close();
}
