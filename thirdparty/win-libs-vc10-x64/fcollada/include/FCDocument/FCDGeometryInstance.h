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
	@file FCDGeometryInstance.h
	This file contains the FCDGeometryInstance class.
*/
#ifndef _FCD_GEOMETRY_ENTITY_H_
#define _FCD_GEOMETRY_ENTITY_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;
class FCDMaterial;
class FCDMaterialInstance;
class FCDGeometryPolygons;
class FCDEffectParameter;

/**
	A COLLADA geometry instance.
	It is during the instantiation of geometries that the mesh polygons
	are attached to actual materials.
*/
class FCOLLADA_EXPORT FCDGeometryInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	DeclareParameterContainer(FCDMaterialInstance, materials, FC("Materials Bound"));
	DeclareParameterContainer(FCDEffectParameter, parameters, FC("Effect Parameters"));

	friend class FCDEntityInstanceFactory;

protected:
	/** Constructor.
		@param document The FCollada document that owns this instance.
		@param parent The visual scene node that contains this instance. This pointer will be NULL for
			instances that are not directly under a visual scene node.
		@param entityType The type of entity to instantiate. */
	FCDGeometryInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type entityType = FCDEntity::GEOMETRY);

public:
	/** Destructor. */
	virtual ~FCDGeometryInstance();

	/** Retrieves the entity instance class type.
		This is used to determine the up-class for the entity instance object.
		@deprecated Instead use: FCDEntityInstance::HasType(FCDGeometryInstance::GetClassType())
		@return The class type: GEOMETRY. */
	virtual Type GetType() const { return GEOMETRY; }

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

	/** Retrieves a material instance bound to the given material semantic.
		@param semantic A material semantic.
		@return The material instance bound to the given material semantic.
			This pointer will be NULL if the material semantic has no material
			instance binding to it. */
	inline FCDMaterialInstance* FindMaterialInstance(const fchar* semantic) { return const_cast<FCDMaterialInstance*>(const_cast<const FCDGeometryInstance*>(this)->FindMaterialInstance(semantic)); }
	inline FCDMaterialInstance* FindMaterialInstance(const fstring& semantic) { return FindMaterialInstance(semantic.c_str()); } /**< See above. */
	const FCDMaterialInstance* FindMaterialInstance(const fchar* semantic) const; /**< See above. */
	inline const FCDMaterialInstance* FindMaterialInstance(const fstring& semantic) const { return FindMaterialInstance(semantic.c_str()); } /**< See above. */

	/** Retrieves the number of material instances.
		@return The number of material instances. */
	inline size_t GetMaterialInstanceCount() const { return materials.size(); }

	/** Retrieves a material instance.
		@param index The index of the material instance.
		@return The material instance at the given index. */
	inline FCDMaterialInstance* GetMaterialInstance(size_t index) { FUAssert(index < materials.size(), return NULL); return materials.at(index); }
	inline const FCDMaterialInstance* GetMaterialInstance(size_t index) const { FUAssert(index < materials.size(), return NULL); return materials.at(index); } /**< See above. */

	/** Retrieves the material instances.
		@return The list of material instances. */
	DEPRECATED(3.05A, GetMaterialInstance) inline FCDMaterialInstance** GetMaterialInstances() { return const_cast<FCDMaterialInstance**>(materials.begin()); }
	DEPRECATED(3.05A, GetMaterialInstance) inline const FCDMaterialInstance** GetMaterialInstances() const { return materials.begin(); } /**< See above. */

	/** Adds an empty material instance to the geometry.
		This new material instance will be unbound.
		@return The empty material instance. */
	FCDMaterialInstance* AddMaterialInstance();

	/** Binds a material with a polygons set for this geometry instance.
		No verification is done to ensure that the polygons set is not
		already bound to another material.
		@param material A material.
		@param polygons A polygons set that belongs to the instanced geometry.
		@return The new material instance. */
	FCDMaterialInstance* AddMaterialInstance(FCDMaterial* material, FCDGeometryPolygons* polygons);


	/** Binds a material to a material semantic token.
		No verification is done to ensure that the material semantic token
		is used within the instanced geometry or that a material is not already bound
		to this token.
		@param material A material.
		@param semantic A material semantic token.
		@return The new material instance. */
	FCDMaterialInstance* AddMaterialInstance(FCDMaterial* material, const fchar* semantic);
	inline FCDMaterialInstance* AddMaterialInstance(FCDMaterial* material, const fstring& semantic) { return AddMaterialInstance(material, semantic.c_str()); } /**< See above. */

	/** Clones the geometry instance.
		@param clone The geometry instance to become the clone. If this pointer
			is NULL, a new geometry instance will be created.
		@return The cloned geometry instance. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent. This method corrects the sub ids if there are conflicts.
		@param parentStringMap The string map from the parent of this instance in which the sub ids must be unique. */
	virtual void CleanSubId(FUSUniqueStringMap* parentStringMap);
};

#endif // _FCD_GEOMETRY_ENTITY_H_
