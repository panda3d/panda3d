// Filename: zcompressor.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::get_total_in
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ulong ZCompressorBase::
get_total_in(void) const {
  return (_stream->total_in);
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::get_total_out
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ulong ZCompressorBase::
get_total_out(void) const {
  return (_stream->total_out);
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::reset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ZCompressorBase::
reset(void) {
  reset_stream(_stream);
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::reset_stream
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ZCompressorBase::
reset_stream(z_stream *strm) const {
  strm->next_in = Z_NULL;  
  strm->avail_in = 0;
  strm->total_in = 0;
  strm->next_out = Z_NULL;  
  strm->avail_out = 0;
  strm->total_out = 0;
  strm->zalloc = Z_NULL;
  strm->zfree = Z_NULL;
  strm->opaque = Z_NULL;
}

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::put_stream
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ZCompressorBase::
put_stream(z_stream *strm, char *next_in, int avail_in, char *next_out, 
	int avail_out) const {
  strm->next_in = (uchar *)next_in;
  strm->avail_in = (ulong)avail_in;
  strm->next_out = (uchar *)next_out;
  strm->avail_out = (ulong)avail_out;
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::get_stream
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ZCompressorBase::
get_stream(z_stream *strm, char *&next_in, int &avail_in, char *&next_out,
        int &avail_out) const {
  next_in = (char *)strm->next_in;
  avail_in = (int)strm->avail_in;
  next_out = (char *)strm->next_out;
  avail_out = (int)strm->avail_out;
}
#endif
