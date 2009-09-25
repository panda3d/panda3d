// P3DActiveXPropPage.cpp : Implementation of the CP3DActiveXPropPage property page class.

// Filename: P3DActiveXPropPage.cpp
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

#include "stdafx.h"
#include "P3DActiveX.h"
#include "P3DActiveXPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CP3DActiveXPropPage, COlePropertyPage)



// Message map

BEGIN_MESSAGE_MAP(CP3DActiveXPropPage, COlePropertyPage)
END_MESSAGE_MAP()



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CP3DActiveXPropPage, "P3DACTIVEX.P3DActiveXPropPage.1",
    0xd0111370, 0xe705, 0x485e, 0x96, 0xfd, 0xa2, 0xef, 0xb1, 0x71, 0x26, 0x9a)



// CP3DActiveXPropPage::CP3DActiveXPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CP3DActiveXPropPage

BOOL CP3DActiveXPropPage::CP3DActiveXPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_P3DACTIVEX_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}



// CP3DActiveXPropPage::CP3DActiveXPropPage - Constructor

CP3DActiveXPropPage::CP3DActiveXPropPage() :
    COlePropertyPage(IDD, IDS_P3DACTIVEX_PPG_CAPTION)
{
}



// CP3DActiveXPropPage::DoDataExchange - Moves data between page and properties

void CP3DActiveXPropPage::DoDataExchange(CDataExchange* pDX)
{
    DDP_PostProcessing(pDX);
}



// CP3DActiveXPropPage message handlers
