// Filename: patchfile.I
// Created by:  mike, darren (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::write_header
//       Access: Private
//  Description: Writes the header with platform-independent
//               byte ordering
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
write_header(ofstream &write_stream, const string &name) {
  _datagram.clear();
  _datagram.add_uint32(_magic_number);
  _datagram.add_int32(name.length());
  _datagram.append_data(name.c_str(), name.length());
  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());
}
