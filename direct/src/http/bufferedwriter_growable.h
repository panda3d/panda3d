#ifndef __BufferedWriter_Growable_H__
#define __BufferedWriter_Growable_H__
///////////////////////////////////////////////////////
// this class is for the usage of growable output...
// it is slower than buffered writer but more robust..
// it also allows for writes to take more time to the out putt..
// ie.. Write buffering.. Not just one write..
///////////////////////////////////////////////////

class BufferedWriter_Growable : public std::string 
{
    int             _write_offset;
public:

    BufferedWriter_Growable(void); 
    ~BufferedWriter_Growable(void);
    int AmountBuffered(void);
    void AppendData(const char * buf, int len);
    void Reset() { clear(); _write_offset = 0; };
    const char * GetMessageHead(void);
    int  Flush(Socket_TCP &sck) ; // this is the ugly part
};


//////////////////////////////////////////////////////////////
// Function name    : BufferedWriter_Growable::BufferedWriter_Growable
// Description      : 
// Return type      : inline 
// Argument         : void
//////////////////////////////////////////////////////////////
inline BufferedWriter_Growable::BufferedWriter_Growable(void) 
{
    _write_offset = 0;
};


//////////////////////////////////////////////////////////////
// Function name    : ~BufferedWriter_Growable::BufferedWriter_Growable
// Description      : 
// Return type      : inline 
// Argument         : void
//////////////////////////////////////////////////////////////
inline BufferedWriter_Growable::~BufferedWriter_Growable(void)
{
}


//////////////////////////////////////////////////////////////
// Function name    : BufferedWriter_Growable::AmountBuffered
// Description      : 
// Return type      : inline int 
// Argument         : void
//////////////////////////////////////////////////////////////
inline int BufferedWriter_Growable::AmountBuffered(void)
{
    return (int) (size() - _write_offset);
}


//////////////////////////////////////////////////////////////
// Function name    : BufferedWriter_Growable::AppendData
// Description      : 
// Return type      : inline void 
// Argument         : const char * buf
// Argument         : int len
//////////////////////////////////////////////////////////////
inline void BufferedWriter_Growable::AppendData(const char * buf, int len)
{
    append(buf, len);
}


//////////////////////////////////////////////////////////////
// Function name    : char * BufferedWriter_Growable::GetMessageHead
// Description      : 
// Return type      : inline const 
// Argument         : void
//////////////////////////////////////////////////////////////
inline const char * BufferedWriter_Growable::GetMessageHead(void)
{
    return data() + _write_offset;
}


//////////////////////////////////////////////////////////////
// Function name    :  BufferedWriter_Growable::Flush
// Description      : 
// Return type      : inline int 
// Argument         : SocketTCP_Gm &sck
//////////////////////////////////////////////////////////////
inline int  BufferedWriter_Growable::Flush(Socket_TCP &sck)  // this is the ugly part
{   
    int answer = 0; 
    int Writesize = AmountBuffered();
    
    if(Writesize > 0)
    {
        const char * out1 = GetMessageHead();
        int Writen = sck.SendData(out1,Writesize);
        if(Writen > 0)
        {
            _write_offset += Writen;
            answer = 1;
        }
        else if(Writen < 0)
        {
            if(GETERROR() != LOCAL_BLOCKING_ERROR) 
                answer = -1;
        }
    }       
    return answer;
}


#endif //__BufferedWriter_Growable_H__


