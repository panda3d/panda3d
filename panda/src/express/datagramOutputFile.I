// Filename: datagramOutputFile.I
// Created by:  drose (27Oct00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DatagramOutputFile::
DatagramOutputFile() {
  _error = true;
  _wrote_first_datagram = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramOutputFile::
open(Filename filename) {
  // DatagramOutputFiles are always binary.
  _wrote_first_datagram = false;
  _error = false;
  filename.set_binary();
  return filename.open_write(_out);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::is_error
//       Access: Public
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramOutputFile::
is_error() {
  if (_out.fail()) {
    _error = true;
  }
  return _error;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramOutputFile destructs.
////////////////////////////////////////////////////////////////////
INLINE void DatagramOutputFile::
close() {
  _out.close();
}
