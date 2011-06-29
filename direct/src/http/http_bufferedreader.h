#ifndef __WEBBUFFEREDREADER_GM_H__
#define __WEBBUFFEREDREADER_GM_H__
// RHH

#include  <string>
#include   "strtargetbuffer.h"
#include   "ringbuffer_slide.h"
#include   "application_log.h"


class   Http_BufferedReader : protected RingBuffer_Slide
{
    inline bool GetTermedString(char * outdata, size_t maxlen,char termchar1, char termchar2);
    inline bool GetDoubleTermedString(char * outdata, int maxlen,char termchar1, char termchar2);
    inline bool GetTermedStringInPLace(char ** outdata,char termchars);
    inline bool GetTermedString(char * outdata, int maxlen,char * termchars);
    inline bool GetSizeString(StrTargetBuffer  & outdata);
public:
    inline Http_BufferedReader(int in_size = 8192) ;
//
// The Read Message Interface
//
    inline void ReSet(void); 
    inline int PumpCRRead(char * data, int   maxdata, Socket_TCP &sck);
    inline int PumpHTTPHeaderRead(char * data, int   maxdata, Socket_TCP &sck);
    inline int PumpSizeRead(StrTargetBuffer  & outdata,Socket_TCP &sck);
    inline int PumpEofRead(StrTargetBuffer  & outdata,Socket_TCP &sck);
    //inline int PumpMessageReader(CoreMessage &inmsg, Socket_TCP &sck);


    template < class SOCK_TYPE>
        inline int ReadPump(SOCK_TYPE &sck)
    {       
        int     answer = 0;
        size_t      readsize = BufferAvailabe();

        if(readsize < 1)
        {
            FullCompress();
            readsize = BufferAvailabe();
        }


        if(readsize > 0)
        {
            char * ff = GetBufferOpen();
            int gotbytes = sck.RecvData(ff,(int)readsize);


            if(gotbytes < 0)  // some error
            {
                int er = GETERROR(); 
                // if(err != LOCAL_BLOCKING_ERROR )
                if(!sck.ErrorIs_WouldBlocking(gotbytes) )
                {
                    answer = -3; 
                    LOGINFO("Http_BufferedReader::ReadPump->Socket Level Read Error %d %d %d %s",er,gotbytes,errno,sck.GetPeerName().get_ip_port().c_str());
                }
                else
                {
                    answer = 0; // try again nothing to read
                }
            }
            else if(gotbytes > 0) // ok got some lets process it
            {

                _EndPos +=  gotbytes;
                answer = 1;
            }
            else   // 0 mean other end disconect arggggg
            {
                answer = -1;
                LOGWARNING("Http_BufferedReader::ReadPump->Other End Closed Normal [%s]",sck.GetPeerName().get_ip_port().c_str());
            }
        }       
        else
        {
            std::string addstr = sck.GetPeerName().get_ip_port();
            LOGWARNING("Http_BufferedReader::ReadPump->** Very Important** No Internal buffer left for read[%s] BufferSIze=[%d][%d]",
                addstr.c_str(),
                AmountBuffered(),
                BufferAvailabe()
                );

            answer = -2;
        }
        return answer;
    }

};

#include "http_bufferedreader.i"

#endif //__BUFFEREDREADER_GM_H__


