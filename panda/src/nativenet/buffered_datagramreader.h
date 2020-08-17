#ifndef BUFFERED_DATAGRAMREADER_H
#define BUFFERED_DATAGRAMREADER_H

#include "ringbuffer.h"
#include "datagram.h"
#include "config_nativenet.h"

/**
 *
 */
class Buffered_DatagramReader : protected RingBuffer {
private:
  inline bool GetMessageFromBuffer(Datagram &inmsg);

public:
  inline Buffered_DatagramReader(int in_size = 8192) ;
  inline void ReSet(void);
  // SOCK_TYPE is used to allow for abstract socket type to be used .. see
  // socket_tcp and socket_ssl

  template<class SOCK_TYPE>
  inline int PumpMessageReader(Datagram &inmsg, SOCK_TYPE &sck) {
    if (GetMessageFromBuffer(inmsg)) {
      return 1;
    }
    int rp = ReadPump(sck);
    if (rp == 0) {
      return 0;
    }

    if (rp < 1) {
      return -1;
    }
    if (GetMessageFromBuffer(inmsg)) {
      return 1;
    }
    return 0;
  }

  template<class SOCK_TYPE>
  inline int ReadPump(SOCK_TYPE &sck) {
    int answer = 0;
    size_t readsize = BufferAvailable();

    if (readsize < 1) {
      Compress();
      readsize = BufferAvailable();
    }

    if (readsize > 0) {
        char *ff = GetBufferOpen();
        int gotbytes = sck.RecvData(ff, (int)readsize);
        if (gotbytes < 0) { // some error
          // int er = GETERROR();
          if (!sck.ErrorIs_WouldBlocking(gotbytes)) {
            nativenet_cat.error()
              << "buffered_datagram_reader:ReadPump socket read error -- " << GETERROR() <<", " <<  sck.GetPeerName().get_ip_port().c_str() << "\n";
            return -3;  // hard error ?
          }
          else {
            return 0; // try again nothing to read
          }
        }
        else if (gotbytes > 0) { // ok got some lets process it
          _EndPos +=  gotbytes;
          return 1;
        }
        else { // 0 mean other end disconect arggggg
          nativenet_cat.error()
            << "buffered_datagram_reader:ReadPump other end of socket closed -- " <<  sck.GetPeerName().get_ip_port().c_str() << "\n";
          return -1;
        }
    }
    else {
      nativenet_cat.error()
        << "buffered_datagram_reader:ReadPump Yeep! buffer has no room to read to -- " << sck.GetPeerName().get_ip_port().c_str() << "\nBufferAvailable = " << readsize <<" AmountBuffered = " << AmountBuffered() << " BufferSize " << GetBufferSize() << "\n";
      return -2;
    }
    return answer;
  }
};

#include "buffered_datagramreader.I"

#endif // BUFFERED_DATAGRAMREADER_H
