#ifndef __PARSEDHTTPREQUEST_GM_H__
#define __PARSEDHTTPREQUEST_GM_H__

#include "string"
#include "map"
#include "stdlib.h"


class ParsedHttpRequest
{
protected:
    std::string             _Raw_Text;
    std::string             _RequestType;
    std::string             _RequestText;
    std::string             _Requestoptions;
    std::string             _BodyText;


    std::map<std::string,std::string>   _parameters;
    std::map<std::string,std::string>   _header_Lines;

    std::string deCanonicalize(std::string &inval);
    size_t  PullCR(std::string &src, std::string &dst);

public:
    void clear(void);
    const std::string   GetRequestOptionString() { return _Requestoptions; };
    const std::string * GetOption(const std::string & query);
    bool GetOption(const std::string & query, std::string & out_value);
    bool ParseThis(char * request);
    std::string & GetRequestURL(void);
    const std::string & GetRawRequest(void) const { return _Raw_Text; };
    const std::string & GetRequestType(void) const { return _RequestType; };
    bool ProcessOptionString(std::string str);
    bool ProcessParamSet(std::string &str);


    void SetBodyText(const std::string &  text)
    {
        _BodyText = text;
    }

    const std::string & getBodyText() { return _BodyText; };

    int getContentLength()
    {
        int answer = 0;
        std::map<std::string,std::string>::iterator ii = _header_Lines.find("Content-Length");
        if(ii != _header_Lines.end())
            answer = atol(ii->second.c_str());

        return answer;
    };
};

/*
class ParsedHttpResponse
{
    std::string             _Raw_Text;
    std::string             _response_header;
    
    std::map<std::string,std::string>   _header_Lines;


public:

    std::string GetresponseCode()
    {
        std::string answer;

        size_t pos = _response_header.find_first_of(" ");
        if(pos != std::string::npos)
            answer =   support::trim_tonew(_response_header.substr(pos,100));


        pos = answer.find_first_of(" \t\n\r\0");
        if(pos != std::string::npos)
            answer =   support::trim_tonew(answer.substr(0,pos));


        return answer;
    }

    bool ParseThis(const std::string &response)
    {   
        _Raw_Text = response;

        int line_number = 0;
        std::string work1(_Raw_Text);
        for(size_t pos = work1.find_first_of("\n\r\0") ; pos != std::string::npos ;   pos = work1.find_first_of("\n\r\0") )
        {
            std::string  line1 = work1.substr(0,pos);
            work1 = work1.substr(pos+1);
            if(line1.size() > 2)
            {

                if(line_number == 0 && line1.substr(0,4) == "HTTP")
                {
                    // the first line...
                    _response_header = line1;
//                    printf("[%s]\n",line1.c_str());
                }

                size_t i_pos = line1.find(':');
                if(i_pos != std::string::npos && i_pos > 1)
                {
                    std::string noune = support::trim_tonew(line1.substr(0,i_pos));
                    std::string verb  = support::trim_tonew(line1.substr(i_pos+1));
                    _header_Lines[noune] = verb;
                }
                line_number++;
            }
        }


        return !_response_header.empty();
    }

    size_t  PullCR(std::string &src, std::string &dst)
    {
        size_t offset = src.find(' ');
        if(offset >= 0 )
        {
            dst = src.substr(0,offset);
            src = src.substr(offset+1);
        }
        return offset;
    }

    int getContentLength()
    {
        int answer = 0;
        std::map<std::string,std::string>::iterator ii = _header_Lines.find("Content-Length");
        if(ii != _header_Lines.end())
            answer = atol(ii->second.c_str());

        return answer;
    };

};
*/

#endif //__PARSEDHTTPREQUEST_GM_H__

