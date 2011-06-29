#include "http_connection.h"

TypeHandle HttpConnection::_type_handle;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
HttpConnection::HttpConnection(SOCKET sck,Socket_Address &inaddr) :
    _Timer(Time_Span(10,0)) ,
    _MyAddress(inaddr),
    _state(READING_HEADER)
{
    SetSocket(sck);
    SetNonBlocking();
    SetReuseAddress();

    _writer.reserve(102400);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
HttpConnection::~HttpConnection(void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Socket_Address & HttpConnection::GetMyAddress(void)
{
    return _MyAddress;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  HttpConnection::DoReadHeader(char * message, int buffersize,Time_Clock &currentTime)
{
    int ans = _Reader.PumpHTTPHeaderRead(message,buffersize,*this);

    if(ans != 0)
    {
        if(ans > 0)
            _headerDetail.assign(message,buffersize);

        return ans;
    }

    if(_Timer.Expired(currentTime) == true)
    {
        return -1;
    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  HttpConnection::DoReadBody(char * message1, int buffersize,Time_Clock &currentTime)
{
    int ans = _Reader.PumpSizeRead(_bodyDetail,*this);

    if(ans != 0)
    {

        return ans;
    }

    if(_Timer.Expired(currentTime) == true)
    {
        return -1;
    }

    // ok lets process this thing..
    _state = WAITING_TO_FINALIZE;

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int HttpConnection::ReadIt(char * message, int buffersize,Time_Clock &currentTime)
{       
    switch (_state)
    {
    case(READING_HEADER):
        return DoReadHeader(message, buffersize,currentTime);
        break;

    case(READING_POST):
        return DoReadBody(message, buffersize,currentTime);
        break;

    case(WAITING_TO_FINALIZE):
        return TryAndFinalize();
        break;

    case(WRITING_DATA):
        return CloseStateWriter(currentTime);
        break;
    default:
        break;

    };
    return ConnectionDoClose;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CloseState HttpConnection::ProcessMessage(char * message,Time_Clock &currentTime)
{
    if(_state == READING_POST )
    {
         _state = WAITING_TO_FINALIZE;
         return ConnectionDoClose;
    }


    if(_parser.ParseThis(message) != true)
    {
        Reset();
        return ConnectionDoClose;
    }
    // if it is a post go into read details mode and 
    // wait to get the post data..
    // we do not support any other methoid today
    if(_parser.GetRequestType() == "POST")
    {
        int context_length = _parser.getContentLength();
        if(context_length > 0)
        {
            //_DoingExtraRead = true;
            _state = READING_POST;
            _bodyDetail.SetDataSize(context_length);
            return ConnectionDoNotClose;
        }
    }

    _state = WAITING_TO_FINALIZE;
  
    _parser.SetBodyText(_bodyDetail);
    _Timer.ResetTime(currentTime);

    if(BuildPage(_writer,_parser) != true)
        return ConnectionDoClose;

    if(_state == WRITING_DATA)
    {
        if(CloseStateWriter(currentTime) <0)
            return ConnectionDoClose;
    }

    return ConnectionDoNotClose;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int HttpConnection::CloseStateWriter(Time_Clock &currentTime)
{
    int fans = _writer.Flush(*this);    // write error
    if(fans < 0)
        return -1;      

    if(_writer.AmountBuffered() <= 0)   // all done
        return -1;      

    if(_Timer.Expired(currentTime) == true) // too long
        return -1;

    return 0;   // keep trying
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HttpConnection::Reset()
{
    _state = ABORTING;
    Close();
    _Timer.ForceToExpired();
}


