// Filename: include_access.cxx
// Created by:  drose (21May02)
// 
////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_ACCESS_H
#define INCLUDE_ACCESS_H

// This file includes whatever is necessary to define the access()
// function.

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32_VC
#include <io.h>      // Windows requires this for access()
#define access _access
#define F_OK 00
#define W_OK 02
#define R_OK 04
#endif  // WIN32_VC

#endif  // INCLUDE_ACCESS_H

