// Filename: bitMask32Transition.h
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BITMASK32TRANSITION_H
#define BITMASK32TRANSITION_H

#include <pandabase.h>

#include "bitMaskTransition.h"
#include "bitMaskAttribute.h"

#include <bitMask.h>


////////////////////////////////////////////////////////////////////
// 	 Class : BitMask32Transition
// Description : This is just an instantation of BitMaskTransition
//               using BitMask32, the most common bitmask type.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, BitMaskTransition<BitMask32>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, BitMaskAttribute<BitMask32>);

typedef BitMaskTransition<BitMask32> BitMask32Transition;
typedef BitMaskAttribute<BitMask32> BitMask32Attribute;

#ifdef __GNUC__
#pragma interface
#endif

#endif


