#ifndef HttpConnection_H 
#define HttpConnection_H 
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
#include "parsedhttprequest.h"
#include "baseincomingset.h"

#include "bufferedwriter_growable.h"
#include "http_bufferedreader.h"


class HttpConnection : public Socket_TCP 
{
protected:
    Http_BufferedReader             _Reader;
    BufferedWriter_Growable     _writer;
    Socket_Address              _MyAddress;

    Time_Out                    _Timer;

    enum    STATE_CONNECTIONS {  
        READING_HEADER =1, 
        READING_POST =2, 
        WAITING_TO_FINALIZE =3,
        WRITING_DATA =4,
        ABORTING = 5,
    };


    STATE_CONNECTIONS                    _state;        


    ParsedHttpRequest           _parser;

    StrTargetBuffer         _bodyDetail;
    std::string             _headerDetail;

    int                    CloseStateWriter(Time_Clock &currentTime);

public:
    virtual ~HttpConnection(void);
    const Socket_Address & GetMyAddress(void);
    virtual bool BuildPage( BufferedWriter_Growable &_writer, ParsedHttpRequest  &parser) = 0;
    HttpConnection(SOCKET sck,Socket_Address &inaddr) ;

    CloseState             ProcessMessage(char * message,Time_Clock &currentTime);
    int                    DoReadHeader(char * message, int buffersize,Time_Clock &currentTime);    
    int                    DoReadBody(char * message, int buffersize,Time_Clock &currentTime);  
    int                    ReadIt(char * message, int buffersize,Time_Clock &currentTime);
    void                   Reset();

    virtual CloseState      TryAndFinalize()  {  _state = WRITING_DATA;  ;return ConnectionDoNotClose; };

    std::string             GetFullmessage() { return _headerDetail + _bodyDetail; };
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_TCP::init_type();
    register_type(_type_handle, "HttpConnection",
                  Socket_TCP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#endif  // HttpConnection_H 



