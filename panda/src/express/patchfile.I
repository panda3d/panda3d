// Filename: patchfile.I
// Created by:  mike (09Jan97)
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

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::write_entry
//       Access: Private
//  Description: Writes an entry consisting of 1. byte position 
//		 2. number of bytes being replaced 3. number of
//		 bytes being inserted with platform-independent
//		 byte ordering.
////////////////////////////////////////////////////////////////////
INLINE void Patchfile::
write_entry(ofstream &write_stream, int pos, int n, int m) {
  express_cat.debug()
    << "write_entry(" << pos << ", " << n << ", " << m << ")" << endl;

  _datagram.clear();
  _datagram.add_int32(pos);
  _datagram.add_int32(n);
  _datagram.add_int32(m);
  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());
}
