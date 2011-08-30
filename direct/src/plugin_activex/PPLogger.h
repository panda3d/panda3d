// Filename: PPLogger.h
// Created by:  atrestman (14Sept09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <iostream>
#include <fstream>

#define P3D_DEFAULT_PLUGIN_LOG_FILENAME "p3dActiveXlog.log"

class PPLogger
{
public:
    PPLogger( );
    virtual ~PPLogger( );

    void Open( const std::string &rootDir );
    static std::ofstream& Log( ) { return m_logfile; }

protected:
  static bool m_isOpen;
  static std::ofstream m_logfile;
};

#define nout PPLogger::Log( )

