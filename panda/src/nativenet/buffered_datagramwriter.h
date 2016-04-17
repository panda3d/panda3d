#ifndef __BufferedWriter_H__
#define __BufferedWriter_H__

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
class   Buffered_DatagramWriter  :  public RingBuffer
{
    int     _flush_point;
public:
    inline void ReSet(void);            // destroy all buffered data

    Buffered_DatagramWriter( size_t in_size , int in_flush_point = -1);
    inline int AddData(const void * data, size_t len, Socket_TCP &sck);
    inline int AddData(const void * data, size_t len);
// THE FUNCTIONS THAT TAKE A SOCKET NEED TO BE TEMPLATED TO WORK..

        template < class SOCK_TYPE>
        int  FlushNoBlock(SOCK_TYPE &sck) {  // this is the ugly part

          int answer = 0;
          size_t Writesize = AmountBuffered();

          if(Writesize > 0) {
            int Writen = sck.SendData(GetMessageHead(),(int)Writesize);
            if(Writen > 0) {
              _StartPos += Writen;
              FullCompress();
              if(AmountBuffered() > 0) // send 0 if empty else send 1 for more to do
                answer = 1;
            }
            else if(Writen < 0) {
              if(!sck.ErrorIs_WouldBlocking(Writen))
                answer = -1;
              else
                answer = 1;  // 1 = more to do.....
            }
          }
          return answer;
        };


        template < class SOCK_TYPE>
        inline int  Flush(SOCK_TYPE &sck) {
          int answer = 0;
          size_t Writesize = AmountBuffered();

          if(Writesize > 0) {
            int Writen = sck.SendData(GetMessageHead(),(int)Writesize);

            if(Writen > 0) {
              _StartPos += Writen;
              FullCompress();
              if(AmountBuffered() > 0) //send 0 if empty else send 1 for more to do
                answer = 1;
            }
            else if(Writen < 0) {
              if(sck.ErrorIs_WouldBlocking(Writen) != true)
                answer = -1;
            }
          }

          return answer;
        };
};

/**
 * used to clear the buffrers ... use of this in mid stream is a very bad
 * thing as you can not guarany network writes are message alligned
 */
inline void Buffered_DatagramWriter::ReSet(void) {
  ResetContent();
}
// Buffered_DatagramWriter::Buffered_DatagramWriter
inline Buffered_DatagramWriter::Buffered_DatagramWriter( size_t in_size , int in_flush_point) : RingBuffer(in_size) {
  _flush_point = in_flush_point;
}

/**
 *
 */
inline int Buffered_DatagramWriter::AddData(const void * data, size_t len, Socket_TCP &sck) {
  int answer = 0;

  if(len >  BufferAvailabe())
    answer = Flush(sck);

  if(answer >= 0)
    answer = AddData(data,len);


  if(answer >= 0 && _flush_point != -1)
    if(_flush_point <  (int)AmountBuffered())
      if(Flush(sck) < 0)
        answer = -1;

  return answer;
}

/**
 *
 */
inline int Buffered_DatagramWriter::AddData(const void * data, size_t len)
{
  int answer = -1;
  if(BufferAvailabe() > len+2) {
    unsigned short len1(len);
    TS_GetInteger(len1,(char *)&len1);
    if(Put((char *)&len1,sizeof(len1)) == true) {
      if(Put((char *)data,len) == true) {
        answer = 1;
      }
    }
  }

  return answer;
}
#endif //__BufferedWriter_H__
