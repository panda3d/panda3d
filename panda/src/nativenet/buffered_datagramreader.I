
/**
 * A function that will peel a core message off the input buffer
 */
inline bool Buffered_DatagramReader::
GetMessageFromBuffer(Datagram &inmsg) {
  size_t DataAvail = _EndPos - _StartPos;
  if (DataAvail >= sizeof(short)) {
    char *ff = _Buffer + _StartPos;
    unsigned short len = *((unsigned short *)ff);
    len += sizeof(unsigned short);
    if (len <= DataAvail) {
      inmsg.assign(ff + 2, len - 2);
      _StartPos += len;
      return true;
    }
  }
  return false;
}
/**
 * Constructor.  Passes size up to ring buffer.
 */
inline Buffered_DatagramReader::
Buffered_DatagramReader(int in_size) : RingBuffer(in_size) {
}

/**
 * Reset all read content, ie.  zeroes out buffer.
 *
 * If you lose framing, this will not help.
 */
inline void Buffered_DatagramReader::
ReSet(void) {
  ResetContent();
}
