// Filename: typedef.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef TYPEDEF_H
#define TYPEDEF_H
//
////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifndef NPOS
  #define NPOS string::npos
#endif

// Declare the namespace std in case it's not already.
namespace std {
};

// Let's always be in the std namespace unless we specify otherwise.
using namespace std;


#endif
