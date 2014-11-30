/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_PLUGIN_H_
#define _FU_PLUGIN_H_

#ifndef _FU_TRACKER_H_
#include "FUtils/FUTracker.h"
#endif // _FU_TRACKER_H_

/**
	A generic plug-in interface structure.
	Used by FCollada within the FCPlugin structure.
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUPlugin : public FUTrackable
{
private:
	DeclareObjectType(FUTrackable);

protected:
	/** Destructor.
		This destructor is protected as no application should
		release the memory of a plug-in directly. Instead, use
		the FUObject::Release function. */
	virtual ~FUPlugin() {}

public:
	/** Retrieves the name of the plug-in.
		@return The name of the plug-in. */
	virtual const char* GetPluginName() const = 0;

	/** Retrieves the version of the plug-in.
		For now, no version string formatting is suggested for this integer.
		It is unused and exists for future expansion only.
		@return The version number of the plug-in. */
	virtual uint32 GetPluginVersion() const = 0;
};

#endif // _FU_PLUGIN_H_
