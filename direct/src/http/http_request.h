#ifndef  Http_Request_H_
#define  Http_Request_H_

class Http_Request;
extern std::set< Http_Request * >                       Global_WebRequests_pendingNotify;
 
class Http_Request : public HttpConnection
{
public:
    Http_Request(SOCKET sck,Socket_Address &inaddr) : HttpConnection(sck,inaddr) 
    {
    };

    ~Http_Request() 
    {
        Global_WebRequests_pendingNotify.erase(this);
    };
    bool BuildPage( BufferedWriter_Growable &_writer, ParsedHttpRequest  &parser)
    {
        Global_WebRequests_pendingNotify.insert((Http_Request *)this);

        _state = WAITING_TO_FINALIZE;
        _Timer.ResetAll(Time_Clock::GetCurrentTime(),Time_Span(9999999,0));
        return true;
    };

    CloseState      TryAndFinalize()  
    {
        return  ConnectionDoNotClose;
    };

PUBLISHED:

    std::string GetRequestType() 
    {
        return _parser.GetRequestType();
    }

    std::string GetRawRequest()
    {
        return _parser.GetRawRequest();
    }

    std::string GetRequestURL()
    {
        return _parser.GetRequestURL();
    }

    std::string GetSourceAddress() 
    {
        return _MyAddress.get_ip_port();
    }

    void   AppendToResponse(const std::string &in)
    {
        _writer+=in;
    }


    void SendThisResponse(const std::string &in)
    {
        _writer+=in;
        Finish();
    }

    void Finish()  
    {    
        _Timer.ResetAll(Time_Clock::GetCurrentTime(),Time_Span(10,0));
        _state  =  WRITING_DATA; 
    };
    void Abort()   
    {
        _state =   ABORTING; 
    };


    bool HasOption(std::string in)
    {
        const std::string * answer = _parser.GetOption(in);
        if(answer != NULL)
       return true;
        return false;
    }

    const char * GetOption(std::string in)
    {
        const std::string * answer = _parser.GetOption(in);
        if(answer != NULL)
       return answer->c_str();
        return "";
    }
    std::string   GetRequestOptionString()
    {
        return _parser.GetRequestOptionString();
    }

   static bool HttpManager_Initialize( unsigned short port);
   static Http_Request * HttpManager_GetARequest();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    HttpConnection::init_type();
    register_type(_type_handle, "Http_Request",
                  HttpConnection::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};






#endif  // Http_Request_H_
