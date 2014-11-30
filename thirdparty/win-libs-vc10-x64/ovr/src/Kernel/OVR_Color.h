/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Color.h
Content     :   Contains color struct.
Created     :   February 7, 2013
Notes       : 

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/
#ifndef OVR_Color_h
#define OVR_Color_h

#include "OVR_Types.h"

namespace OVR {

struct Color
{
    UByte R,G,B,A;

    Color() {}

    // Constructs color by channel. Alpha is set to 0xFF (fully visible)
    // if not specified.
    Color(unsigned char r,unsigned char g,unsigned char b, unsigned char a = 0xFF)
        : R(r), G(g), B(b), A(a) { }

    // 0xAARRGGBB - Common HTML color Hex layout
    Color(unsigned c)
        : R((unsigned char)(c>>16)), G((unsigned char)(c>>8)),
        B((unsigned char)c), A((unsigned char)(c>>24)) { }

    bool operator==(const Color& b) const
    {
        return R == b.R && G == b.G && B == b.B && A == b.A;
    }

    void  GetRGBA(float *r, float *g, float *b, float* a) const
    {
        *r = R / 255.0f;
        *g = G / 255.0f;
        *b = B / 255.0f;
        *a = A / 255.0f;
    }
};

}

#endif
