#ifndef   StrTargetBuffer_h_
#define   StrTargetBuffer_h_

#include <string>

class  StrTargetBuffer : public std::string 
{
    size_t     _target_size;
public:
    StrTargetBuffer() : std::string(), _target_size(0)
    {
    }

    size_t  left_to_fill() 
    { 
        if(_target_size < size())
            return 0;

        return _target_size - size();
    };

    void SetDataSize(size_t  target)
    {
        _target_size = target;
    }

    size_t  GetTargetSize() { return _target_size; };
};

#endif   // StrTargetBuffer_h_

