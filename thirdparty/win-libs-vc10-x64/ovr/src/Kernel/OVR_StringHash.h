/************************************************************************************

PublicHeader:   None
Filename    :   OVR_StringHash.h
Content     :   String hash table used when optional case-insensitive
                lookup is required.
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_StringHash_h
#define OVR_StringHash_h

#include "OVR_String.h"
#include "OVR_Hash.h"

namespace OVR {

//-----------------------------------------------------------------------------------
// *** StringHash

// This is a custom string hash table that supports case-insensitive
// searches through special functions such as GetCaseInsensitive, etc.
// This class is used for Flash labels, exports and other case-insensitive tables.

template<class U, class Allocator = ContainerAllocator<U> >
class StringHash : public Hash<String, U, String::NoCaseHashFunctor, Allocator>
{
public:
    typedef U                                                        ValueType;
    typedef StringHash<U, Allocator>                                 SelfType;
    typedef Hash<String, U, String::NoCaseHashFunctor, Allocator>    BaseType;

public:    

    void    operator = (const SelfType& src) { BaseType::operator = (src); }

    bool    GetCaseInsensitive(const String& key, U* pvalue) const
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey, pvalue);
    }
    // Pointer-returning get variety.
    const U* GetCaseInsensitive(const String& key) const   
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }
    U*  GetCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }

    
    typedef typename BaseType::Iterator base_iterator;

    base_iterator    FindCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return BaseType::FindAlt(ikey);
    }

    // Set just uses a find and assigns value if found. The key is not modified;
    // this behavior is identical to Flash string variable assignment.    
    void    SetCaseInsensitive(const String& key, const U& value)
    {
        base_iterator it = FindCaseInsensitive(key);
        if (it != BaseType::End())
        {
            it->Second = value;
        }
        else
        {
            BaseType::Add(key, value);
        }
    } 
};

} // OVR 

#endif
