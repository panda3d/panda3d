
/**
 * A function that will peal a core message of the input buffer
 */
inline bool Buffered_DatagramReader::
GetMessageFromBuffer(Datagram &inmsg) {
  bool answer = false;
  size_t DataAvail = FastAmountBeffered();
  if (DataAvail >= sizeof(short)) {
    char *ff = FastGetMessageHead();
    unsigned short len=GetUnsignedShort(ff);
    len += sizeof(unsigned short);
    if (len <= DataAvail) {
      inmsg.assign(ff + 2, len - 2);
      _StartPos += len;
      answer = true;
    }
  }
  return answer;
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
