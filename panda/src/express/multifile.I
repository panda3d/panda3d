// Filename: multifile.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Multifile::write_header
//       Access: Private
//  Description: Writes the header uncompressed with platform-
//               independent byte ordering
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
write_header(ofstream &write_stream) {
  _datagram.clear();
  _datagram.add_uint32(_magic_number);
  _datagram.add_int32(_num_mfiles);
  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_header_length
//       Access: Public 
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Multifile::
get_header_length(void) const {
  return _header_length;
}
