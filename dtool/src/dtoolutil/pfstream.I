// Filename: pfstream.I
// Created by:  cary (12Dec00)
// 
////////////////////////////////////////////////////////////////////

INLINE ipfstream::ipfstream(const string cmd) : _psb(PipeStreamBuf::Input) {
  _psb.command(cmd);
}

INLINE void ipfstream::flush(void) {
  _psb.flush();
}

INLINE ipfstream::ipfstream(void) : _psb(PipeStreamBuf::Input) {
  cerr << "should never call default constructor of ipfstream" << endl;
}

INLINE opfstream::opfstream(const string cmd) : _psb(PipeStreamBuf::Output) {
  _psb.command(cmd);
}

INLINE void opfstream::flush(void) {
  _psb.flush();
}

INLINE opfstream::opfstream(void) : _psb(PipeStreamBuf::Output) {
  cerr << "should never call default constructor of opfstream" << endl;
}
