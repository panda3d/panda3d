/* ***** BEGIN LICENSE BLOCK ***** 
 * Version: RCSL 1.0/RPSL 1.0 
 *  
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved. 
 *      
 * The contents of this file, and the files included with this file, are 
 * subject to the current version of the RealNetworks Public Source License 
 * Version 1.0 (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the RealNetworks Community Source License Version 1.0 
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl, 
 * in which case the RCSL will apply. You may also obtain the license terms 
 * directly from RealNetworks.  You may not use this file except in 
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks 
 * applicable to this file, the RCSL.  Please see the applicable RPSL or 
 * RCSL for the rights, obligations and limitations governing use of the 
 * contents of the file.  
 *  
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the portions 
 * it created. 
 *  
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * Technology Compatibility Kit Test Suite(s) Location: 
 *    http://www.helixcommunity.org/content/tck 
 * 
 * Contributor(s): 
 *  
 * ***** END LICENSE BLOCK ***** */ 

#ifndef _FIVEMMAP_H_
#define _FIVEMMAP_H_

#include "pandabase.h"
#include "helixDefs.h"

#include <hxtypes.h>

#ifndef _HXTYPES_H_
#error FiveMinuteMap assumes pntypes.h.
#endif 

class FiveMinuteMap {
    const int AllocationSize;

    void**  KeyArray;
    void**  ValueArray;
    int     m_nMapSize;
    int     m_nAllocSize;
    int     m_nCursor;
public:
    FiveMinuteMap()
    : KeyArray(NULL)
    , ValueArray(NULL)
    , m_nMapSize(0)
    , m_nAllocSize(0)
        , m_nCursor(0)
    , AllocationSize(10)
    {};

    ~FiveMinuteMap()
    {
        delete [] KeyArray;
        delete [] ValueArray;
    };

    int  GetCount() {return m_nMapSize;}

    void* GetFirstValue();
    void* GetNextValue();

    BOOL Lookup(void* Key, void*& Value) const;
    void RemoveKey(void* Key);
    void RemoveValue(void* Value);
    void SetAt(void* Key, void* Value);
};

#endif /* _FIVEMMAP_H_ */
