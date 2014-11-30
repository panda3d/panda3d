/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUParameterizable.h
	This file contains the FUParameterizable class.
*/

#ifndef _FU_PARAMETERIZABLE_H_
#define _FU_PARAMETERIZABLE_H_

#ifndef _FU_TRACKER_H_
#include "FUtils/FUTracker.h"
#endif // _FU_TRACKER_H_

class FUParameter;

/** Use the following macros to declare flags. This will create functions
	to get, set and clear the flag, and a public static variable: 
	FLAG_(flag_name) to represent the flag mask.
	@param flag_name The unique name of the flag.
	@param local_position The local offset for this flag. */
#define DeclareFlag(flag_name, local_position) \
	static const uint32 FLAG_##flag_name = (1 << (Parent::nextAvailableBit + local_position)); \
	inline void Set##flag_name##Flag(bool value) { flags &= ~FLAG_##flag_name; flags |= FLAG_##flag_name * value; } \
	inline void Set##flag_name##Flag() { flags |= FLAG_##flag_name; } \
	inline void Reset##flag_name##Flag() { flags &= ~FLAG_##flag_name; } \
	inline bool Get##flag_name##Flag() const { return (flags & FLAG_##flag_name) != 0; } 

/** Use the following macro to set the number of
	local flags declared by this class.
	@param count The number of local flags declared by this class. */
#define DeclareFlagCount(count) \
protected: \
	static const uint32 nextAvailableBit = Parent::nextAvailableBit + count;


/**
	A trackable object which contains parameters.
	Having a generic object that contains a generic parameter structure
	has many applications to improve user interfaces coding and maintenance,
	including simple edition, undo/redo, animation authoring.
	@see FUParameter, FCDParameterAnimatable
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUParameterizable : public FUTrackable
{
private:
	DeclareObjectType(FUTrackable);

protected:
	/** A generic flag container. Contains notifications of object state. */
	uint32 flags;

public:
	DeclareFlag(Dirty, 0); /**< [EXPERIMENTAL] Generic Dirty Flag. Should be removed in future versions. */
	DeclareFlag(ValueChanged, 1); /**< [EXPERIMENTAL] The value of a member has been changed.
								        Examples: 1. the diffuse color changes from black to red.
										2. The translation vector changes from {0,3,2} to {5,2,1}. */
	DeclareFlag(StructureChanged, 2); /**< [EXPERIMENTAL] The structure of this object has changed.
										Examples: 1. The effect of a material is changed.
										2. A new transform is added to the scene node.
										3. An effect parameter is removed from the effect's technique. */
	DeclareFlagCount(3); /**< This class declares three flags. */

public:
	/** Constructor. */
	FUParameterizable();

	/** Destructor. */
	virtual ~FUParameterizable();

	/** Changes the value of one or more flags to a given value.
		@param f The mask of the flag(s) to set.
		@param value The value to give these flags. */
	inline void SetFlag(uint32 f, bool value) { flags &= ~f; flags |= f * value; }

	/** Sets the value of one or more flags.
		When a flag is set, its value is 1.
		@param f The mask of the flag(s) to set. */
	inline void SetFlag(uint32 f) { flags |= f; }

	/** Retrieves the value of one or more flags.
		@param f The mask of the flag(s) to retrieve.
		@return The value of the requested flags. */
	inline uint32 TestFlag(uint32 f) const { return flags & f; }
	
	/** Resets the value of one or more flags.
		When a flag is reset, its value is 0.
		@param f The mask of the flag(s) to reset. */
	inline void ClearFlag(uint32 f) { flags &= ~f; }

	/** Value change notification. */
	virtual void SetValueChange() {}
};

#endif // _FU_PARAMETERIZABLE_H_
