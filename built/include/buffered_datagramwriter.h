#ifndef BUFFERED_DATAGRAMWRITER_H
#define BUFFERED_DATAGRAMWRITER_H

#include "ringbuffer.h"

/**
 * This is the buffered writer.. it is used to buffer up Coremessages and
 * arbitrary data..
 *
 * GmCoreMessage
 *

 *
 * You must commit all rights to a socket with flush and flush may be called
 * internall if the buffersize is about to overrun.. This class does guaranty
 * no partial message rights at least to the TCP layer..
 *
 */
class Buffered_DatagramWriter : public RingBuffer {
public:
  inline void ReSet(void); // destroy all buffered data

  Buffered_DatagramWriter(size_t in_size, int in_flush_point = -1);
  inline int AddData(const void *data, size_t len, Socket_TCP &sck);
  inline int AddData(const void *data, size_t len);

  // THE FUNCTIONS THAT TAKE A SOCKET NEED TO BE TEMPLATED TO WORK..
  template<class SOCK_TYPE>
  int FlushNoBlock(SOCK_TYPE &sck) {  // this is the ugly part
    size_t writesize = AmountBuffered();
    if (writesize > 0) {
      int written = sck.SendData(GetMessageHead(), (int)writesize);
      if (written > 0) {
        _StartPos += written;
        FullCompress();
        if (AmountBuffered() > 0) {// send 0 if empty else send 1 for more to do
          return 1;
        }
      }
      else if (written < 0) {
        if (!sck.ErrorIs_WouldBlocking(written)) {
          return -1;
        } else {
          return 1;  // 1 = more to do.....
        }
      }
    }
    return 0;
  };

  template<class SOCK_TYPE>
  inline int Flush(SOCK_TYPE &sck) {
    size_t writesize = AmountBuffered();

    if (writesize > 0) {
      int written = sck.SendData(GetMessageHead(),(int)writesize);
      if (written > 0) {
        _StartPos += written;
        FullCompress();
        if (AmountBuffered() > 0) { //send 0 if empty else send 1 for more to do
          return 1;
        }
      }
      else if(written < 0) {
        if (!sck.ErrorIs_WouldBlocking(written)) {
          return -1;
        }
      }
    }
    return 0;
  };

private:
  int _flush_point;
};

#include "buffered_datagramwriter.I"

#endif // BUFFERED_DATAGRAMWRITER_H
