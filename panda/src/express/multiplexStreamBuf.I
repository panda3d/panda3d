// Filename: multiplexStreamBuf.I
// Created by:  drose (27Nov00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MultiplexStreamBuf::add_output
//       Access: Public
//  Description: Adds the indicated output destinition to the set of
//               things that will be written to when characters are
//               output to the MultiplexStream.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStreamBuf::
add_output(MultiplexStreamBuf::BufferType buffer_type,
	   MultiplexStreamBuf::OutputType output_type,
	   ostream *out, FILE *fout, bool owns_obj) {
  Output o;
  o._buffer_type = buffer_type;
  o._output_type = output_type;
  o._out = out;
  o._fout = fout;
  o._owns_obj = owns_obj;
  _outputs.push_back(o);
}


////////////////////////////////////////////////////////////////////
//     Function: MultiplexStreamBuf::flush
//       Access: Public
//  Description: Forces out all output that hasn't yet been written.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStreamBuf::
flush() {
  write_chars("", 0, true);
}
