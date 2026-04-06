
/**
 * used to clear the buffers ... use of this in mid stream is a very bad
 * thing as you cannot guarantee network writes are message-aligned
 */
inline void Buffered_DatagramWriter::
ReSet(void) {
  ResetContent();
}

/**
 *
 */
inline Buffered_DatagramWriter::
Buffered_DatagramWriter(size_t in_size, int in_flush_point) : RingBuffer(in_size) {
  _flush_point = in_flush_point;
}

/**
 *
 */
inline int Buffered_DatagramWriter::
AddData(const void * data, size_t len, Socket_TCP &sck) {
  int answer = 0;

  if (len > BufferAvailable()) {
    answer = Flush(sck);
  }

  if (answer >= 0) {
    answer = AddData(data,len);
  }

  if (answer >= 0 && _flush_point != -1) {
    if (_flush_point < (int)AmountBuffered()) {
      if (Flush(sck) < 0) {
        answer = -1;
      }
    }
  }

  return answer;
}

/**
 *
 */
inline int Buffered_DatagramWriter::
AddData(const void *data, size_t len) {
  if (BufferAvailable() > len + 2) {
    unsigned short len1(len);
    TS_GetInteger(len1, (char *)&len1);
    if (Put((char *)&len1, sizeof(len1))) {
      if (Put((char *)data, len)) {
        return 1;
      }
    }
  }
  return -1;
}
