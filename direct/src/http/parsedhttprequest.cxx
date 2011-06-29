#pragma warning(disable : 4789)
#pragma warning(disable : 4786)

#include "parsedhttprequest.h"

////////////////////////////////////////////////////////////////////
inline std::string & trimleft_inplace(std::string & s)
{
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    return s;
}


////////////////////////////////////////////////////////////////////
inline std::string & trimright_inplace(std::string & s)
{
    size_t idx = s.find_last_not_of(" \t\n\r");

    if (std::string::npos == idx)
    {
        s.erase();
    }
    else
    {
        char c  = s.at(idx);
        s.erase(idx, std::string::npos);    
        s.append(1, c);
    }

    return s;
}
////////////////////////////////////////////////////////////////////
inline std::string & trim_inplace(std::string & s)
{
    trimleft_inplace(s);
    trimright_inplace(s);
    return s;
}

inline std::string  trim_tonew(const std::string &in)
{
    std::string ss = in;
    return trim_inplace(ss);    
}




std::string ParsedHttpRequest::deCanonicalize(std::string &inval)
{
    std::string work("");
    unsigned int x=0;
    while (x < inval.size())
    {
        switch(inval[x])
        {
        case('+'):
            work+=' ';
            x++;
            break;
            
        case('%'):
            if(x+2 < inval.size())
            {
                x++;
                char aa[5];
                char * end;
                aa[0] = inval[x++];
                aa[1] = inval[x++];
                aa[2] = '\0';
                char    c = ( char ) strtoul(aa,&end,16);
                work+=c;
            }
            else
                x+=3;
            break;
            
        default:
            work+=inval[x++]; 
            break;
        }           
    }
    return work;
}

size_t  ParsedHttpRequest::PullCR(std::string &src, std::string &dst)
{
    size_t offset = src.find(' ');
    if(offset >= 0 )
    {
        dst = src.substr(0,offset);
        src = src.substr(offset+1);
    }
    return offset;
}


void ParsedHttpRequest::clear(void)
{
    _RequestType = "";
    _parameters.clear();
}

const std::string * ParsedHttpRequest::GetOption(const std::string & query)
{
    std::map<std::string,std::string>::iterator ii;
    ii = _parameters.find(query);
    if(ii == _parameters.end())
        return NULL;
    
    return &ii->second;
}


bool ParsedHttpRequest::GetOption(const std::string & query, std::string & out_value)
{
    std::map<std::string,std::string>::iterator ii;
    ii = _parameters.find(query);
    if(ii == _parameters.end())
    {
        out_value   = "";
        return false;
    }
    out_value = ii->second;
    return true;
}

bool ParsedHttpRequest::ParseThis(char * request)
{
    _Raw_Text = request;
//    printf("%s\n\n",request);
    

    std::string work1(_Raw_Text);
    for(size_t pos = work1.find_first_of("\n\r\0") ; pos != std::string::npos ;   pos = work1.find_first_of("\n\r\0") )
    {
        std::string  line1 = work1.substr(0,pos);
        work1 = work1.substr(pos+1);
        if(line1.size() > 2)
        {
//            printf(" Line[%s]\n",line1.c_str());
            size_t i_pos = line1.find(':');
            if(i_pos != std::string::npos && i_pos > 1)
            {
                std::string noune = trim_tonew(line1.substr(0,i_pos));
                std::string verb  = trim_tonew(line1.substr(i_pos+1));

                //printf(" Noune [%s][%s]\n",noune.c_str(),verb.c_str());
                _header_Lines[noune] = verb;

            }
        }
    }

    //
    // Get the url for the request ..
    //
    std::string work(request);
    std::string line1 = work.substr(0,work.find_first_of("\n\r\0"));
    if(line1.size() < 4)
        return false;
    
    if(PullCR(line1,_RequestType) < 3)
        return false;
    
    if(PullCR(line1,_RequestText) < 1)
        return false;
    
    size_t loc = (int)_RequestText.find('?');
    if(loc != std::string::npos)
    {
        _Requestoptions = _RequestText.substr(loc+1);
        _RequestText =  _RequestText.substr(0,loc);
    }
    
    return ProcessOptionString(_Requestoptions);
}

std::string & ParsedHttpRequest::GetRequestURL(void) 
{ 
    return _RequestText; 
};

bool ParsedHttpRequest::ProcessOptionString(std::string str)
{
    size_t loc;
    for(loc = str.find('&'); loc != std::string::npos; loc = str.find('&'))
    {
        std::string valset = str.substr(0,loc);
        str = str.substr(loc+1);
        if(ProcessParamSet(valset) != true)
            return false;
    }
    return ProcessParamSet(str);
};

bool ParsedHttpRequest::ProcessParamSet(std::string &str)
{
    std::string val("");
    size_t loc = str.find('=');
    
    if(loc != std::string::npos)
    {
        val = str.substr(loc+1);
        str = str.substr(0,loc);    
        
        std::string ind1 = deCanonicalize(str);
        _parameters[ind1] = deCanonicalize(val);
    }
    return true;
}



