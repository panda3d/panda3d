// Filename: P3DActiveXPropPage.h
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

// P3DActiveXPropPage.h : Declaration of the CP3DActiveXPropPage property page class.
#include "PPPandaObject.h"

// CP3DActiveXPropPage : See P3DActiveXPropPage.cpp for implementation.

class CP3DActiveXPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CP3DActiveXPropPage)
    DECLARE_OLECREATE_EX(CP3DActiveXPropPage)

// Constructor
public:
    CP3DActiveXPropPage();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_P3DACTIVEX };

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    DECLARE_MESSAGE_MAP()
};

