// Filename: pfstream.I
// Created by:  cary (12Dec00)
// 
////////////////////////////////////////////////////////////////////

INLINE IPipeStream::IPipeStream(const std::string cmd)
  : istream(&_psb), _psb(PipeStreamBuf::Input) {
  _psb.command(cmd);
}

INLINE void IPipeStream::flush(void) {
  _psb.flush();
}

INLINE IPipeStream::IPipeStream(void) : _psb(PipeStreamBuf::Input) {
  cerr << "should never call default constructor of IPipeStream" << endl;
}

INLINE OPipeStream::OPipeStream(const std::string cmd)
  : ostream(&_psb), _psb(PipeStreamBuf::Output) {
  _psb.command(cmd);
}

INLINE void OPipeStream::flush(void) {
  _psb.flush();
}

INLINE OPipeStream::OPipeStream(void) : _psb(PipeStreamBuf::Output) {
  cerr << "should never call default constructor of OPipeStream" << endl;
}
