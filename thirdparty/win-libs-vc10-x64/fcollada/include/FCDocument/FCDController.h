/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDController.h
	This file contains the FCDController class.
*/

#ifndef _FCD_CONTROLLER_H_
#define _FCD_CONTROLLER_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDGeometry;
class FCDSkinController;
class FCDMorphController;

/**
	A generic COLLADA controller.
	A COLLADA controller is used to influence a mesh.
	COLLADA defines two types of controller:
	skins (FCDSkinController) and morphers (FCDMorphController).

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDController : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	DeclareParameterRef(FCDSkinController, skinController, FC("Skin"));
	DeclareParameterRef(FCDMorphController, morphController, FC("Morpher"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDLibrary::AddEntity function.
		@param document The COLLADA document that owns the controller. */
	FCDController(FCDocument* document);

	/** Destructor. */
	virtual ~FCDController();

	/** Retrieves the entity class type.
		This function is a part of the FCDEntity interface.
		@return The entity class type: CONTROLLER. */
	virtual Type GetType() const { return CONTROLLER; };

	/** Retrieves whether this controller is a skin controller.
		@return Whether this controller is a skin controller. */
	bool IsSkin() const { return skinController != NULL; }

	/** Retrieves whether this controller is a morph controller.
		@return Whether this controller is a morph controller. */
	bool IsMorph() const { return morphController != NULL; }

	/** Sets the type of this controller to a skin controller.
		This function will release any previously created morpher or skin.
		@return The new skin controller. */
	FCDSkinController* CreateSkinController();

	/** Sets the type of this controller to a morph controller.
		This function will release any previously created morpher or skin.
		@return The new morph controller. */
	FCDMorphController* CreateMorphController();

	/** Retrieves the skin controller.
		This pointer is only valid for skins. To verify that this is a skin,
		check the HasSkinController function.
		@return The skin controller. This pointer will be NULL, if the controller
			is not a skin. */
	inline FCDSkinController* GetSkinController() { return skinController; }
	inline const FCDSkinController* GetSkinController() const { return skinController; } /**< See above. */

	/** Retrieves the morph controller.
		This pointer is only valid for skins. To verify that this is a morpher,
		check the HasMorphController function.
		@return The morph controller. This pointer will be NULL, if the controller
			is not a morpher. */
	inline FCDMorphController* GetMorphController() { return morphController; }
	inline const FCDMorphController* GetMorphController() const { return morphController; } /**< See above. */

	/** Retrieves the base target entity for this controller.
		The base target entity may be another controller or a geometry entity.
		To change the base target, use the FCDMorphController::SetBaseTarget
		or the FCDSkinController::SetTarget functions.
		@return The base target entity. This pointer will be NULL
			if no base target is defined. */
	FCDEntity* GetBaseTarget();
	const FCDEntity* GetBaseTarget() const; /**< See above. */

	/** Retrieves the base target geometry for this controller.
		Controllers can be chained together. This function allows
		you to retrieve the base target geometry, if there is one.
		@return The base target geometry. This pointer will be NULL
			if no base target is defined or if the base target entity
			is not a geometry. */
	inline FCDGeometry* GetBaseGeometry() { return const_cast<FCDGeometry*>(const_cast<const FCDController*>(this)->GetBaseGeometry()); }
	const FCDGeometry* GetBaseGeometry() const; /**< See above. */

	/** Retrieves the lowest controller on this stack.
		Controllers can be chained together. This function allows
		you to retrieve the controller assigned to the base target geometry.
		@return The base controller. This pointer will be NULL
			if no base target is defined or if the base target entity
			is not a geometry. */
	inline FCDController* GetBaseGeometryController() 	{ return const_cast<FCDController*>(const_cast<const FCDController*>(this)->GetBaseGeometryController()); }
	const FCDController* GetBaseGeometryController() const; /**< See above. */
};

#endif // _FCD_CONTROLLER_H_

