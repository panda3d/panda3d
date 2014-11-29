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
	@file FCDMaterial.h
	This file contains the FCDMaterail class and the FCDMaterialTechniqueHint structure.
*/

#ifndef _FCD_MATERIAL_H_
#define _FCD_MATERIAL_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDEffect;
class FCDEffectParameter;
class FCDEntityReference;

/**
	A technique usage hint for a material.
	This structure contains two strings to help applications
	choose a technique within the material's instantiated effect
	according to their application platform.
*/
class FCOLLADA_EXPORT FCDMaterialTechniqueHint
{
public:
	fstring platform; /**< A platform semantic. COLLADA defines no platform semantics. */
	fm::string technique; /**< The sid for the technique to choose for the platform. */
};

/** A dynamically-sized list of material platform-technique hints. */
typedef fm::vector<FCDMaterialTechniqueHint> FCDMaterialTechniqueHintList; 

/**
	A COLLADA material.

	A COLLADA material is one of many abstraction level that defines how
	to render mesh polygon sets. It instantiates an effect and may
	overrides some of the effect parameters with its own values.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDMaterial : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	bool ownsEffect;
	DeclareParameterPtr(FCDEntityReference, effect, FC("Effect"));
	DeclareParameterContainer(FCDEffectParameter, parameters, FC("Effect Parameters"))
	FCDMaterialTechniqueHintList techniqueHints;

public:
	/** Constructor: do not use directly.
		Instead, use the FCDLibrary::AddEntity function.
		@param document The COLLADA document that owns the material. */
	FCDMaterial(FCDocument* document);

	/** Destructor. */
	virtual ~FCDMaterial();

	/** Retrieves the entity type for this class. This function is part
		of the FCDEntity class interface.
		@return The entity type: MATERIAL. */
	virtual Type GetType() const { return FCDEntity::MATERIAL; }

	/** Retrieves the effect instantiated for this material.
		The parameters of the effect may be overwritten by this material.
		@return The instantiated effect. This pointer will be NULL if the material has no rendering. */
	FCDEffect* GetEffect() { return const_cast<FCDEffect*>(const_cast<const FCDMaterial*>(this)->GetEffect()); }
	const FCDEffect* GetEffect() const; /**< See above. */

	/** Retrieves the effect reference for this material.
		Using the FCDEffectReference object for the effect is only useful when
		dealing manually with external references.
		@return The entity reference for the effect. */
	FCDEntityReference* GetEffectReference() { return effect; }
	const FCDEntityReference* GetEffectReference() const { return effect; }

	/** Sets the effect instantiated for this material.
		@param _effect The effect instantiated for this material. */
	void SetEffect(FCDEffect* _effect);

	/** Retrieves the list of the material platform-technique hints.
		@return The list of material platform-technique hints. */
	FCDMaterialTechniqueHintList& GetTechniqueHints() { return techniqueHints; }
	const FCDMaterialTechniqueHintList& GetTechniqueHints() const { return techniqueHints; } /**< See above. */
	
	/** Retrieves the number of local effect parameters
		@return The number of local effect parameters. */
	inline size_t GetEffectParameterCount() const { return parameters.size(); }

	/** Retrieves a given local effect parameter.
		@param index An index.
		@return The local effect parameter at the given index. */
	inline FCDEffectParameter* GetEffectParameter(size_t index) { FUAssert(index < parameters.size(), return NULL); return parameters.at(index); }
	inline const FCDEffectParameter* GetEffectParameter(size_t index) const { FUAssert(index < parameters.size(), return NULL); return parameters.at(index); }

	/** Adds a local effect parameter to the local list.
		@see FCDEffectParameter::Type
		@param type The value type of the effect parameter to create.
		@return The new local effect parameter. */
	FCDEffectParameter* AddEffectParameter(uint32 type);

	/** Clones the material object.
		Everything is cloned, including the effect parameters.
		@param clone The material clone. If this pointer is NULL, a new material object
			will be created and you will need to release the returned pointer.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** [INTERNAL] Flattens the material, pushing all the effect parameter overrides
		into the effect parameter generators and moving all the parameters to the 
		effect technique level of abstraction. To flatten the material, use the
		FCDMaterialInstance::FlattenMaterial function. */
	DEPRECATED(3.05A, not recommended) void Flatten() {}
};

#endif // _FCD_MATERIAL_H_
