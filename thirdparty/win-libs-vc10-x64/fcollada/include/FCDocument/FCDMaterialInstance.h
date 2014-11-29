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
	@file FCDMaterialInstance.h
	This file contains the FCDMaterialInstance and the FCDMaterialInstanceBind classes.
*/

#ifndef _FCD_MATERIAL_BIND_H_
#define	_FCD_MATERIAL_BIND_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDGeometryPolygons;

/**
	A ColladaFX per-instance binding.

	This structure is used by the effect to bind its parameters to scene graph values.
	Two common examples of this is to bind effect parameters to the position and orientation of the camera.

	@ingroup FCDEffect
*/
class FCDMaterialInstanceBind : public FUParameterizable
{
private:
	DeclareObjectType(FUParameterizable);

public:
	/** The token used to identify the effect parameter to modify. */
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, semantic, FC("Semantic"));

	/** The fully-qualified target of the COLLADA element whose value should be bound to the effect parameter. */
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, target, FC("Target"));

	/** Constructor. */
	FCDMaterialInstanceBind();

	/** Destructor. */
	virtual ~FCDMaterialInstanceBind();
};

/**
	A ColladaFX per-instance vertex input binding.

	This structure is used by the shader to bind its varying inputs
	to actual data streams from the polygons sets. A common example of this
	is to identify which texture coordinate set to use for a particular TEXCOORDx stream.

	@ingroup FCDEffect
*/
class FCDMaterialInstanceBindVertexInput : public FUParameterizable
{
private:
	DeclareObjectType(FUParameterizable);

public:
	/** The token used to identify the effect parameter or varying shader input. */
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, semantic, FC("Bind Semantic"));

	/** The geometry source type of the data source to bind to the effect parameter.
		@see FUDaeGeometryInput::Semantic. */
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, inputSemantic, FC("Input Semantic"));

	/** The set value of the data source to bind to the effect parameter.
		A semantic/set pair should always be unique within a polygons set. */
	DeclareParameter(int32, FUParameterQualifiers::SIMPLE, inputSet, FC("Input Set"));

	/** Constructor. */
	FCDMaterialInstanceBindVertexInput();

	/** Destructor. */
	virtual ~FCDMaterialInstanceBindVertexInput();

	/** Retrieves the input semantic for this vertex-input binding.
		@return The input semantic. */
	FUDaeGeometryInput::Semantic GetInputSemantic() const { return (FUDaeGeometryInput::Semantic) *inputSemantic; }
};

/**
	A COLLADA material instance.
	A material instance is used to given polygon sets with a COLLADA material entity.
	It is also used to bind data sources with the inputs of an effect.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDMaterialInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	FCDEntityInstance* parent;

	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, semantic, FC("Material Token"));
	DeclareParameterContainer(FCDMaterialInstanceBind, bindings, FC("Effect Bindings"));
	DeclareParameterContainer(FCDMaterialInstanceBindVertexInput, vertexBindings, FC("Vertex Input Bindings"));

public:
	/** Constructor.
		@param parent The entity instance that owns this material instance. */
	FCDMaterialInstance(FCDocument* document, FCDEntityInstance* parent);

	/** Destructor. */
	virtual ~FCDMaterialInstance();

	/** Retrieves the type of entity instance.
		@deprecated Use the following code, instead:
			entityInstance->HasType(FCDMaterialInstance::GetClassType()).
		@return The type of entity instance. */
	DEPRECATED(3.05A, HasType(FCDMaterialInstance::GetClassType())) virtual Type GetType() const { return MATERIAL; }

	/** Retrieves the parent instance.
		@return The parent instance. */
	inline FCDEntityInstance* GetParent() { return parent; }
	inline const FCDEntityInstance* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the symbolic name used to link the polygons sets with the material of this instance.
		@return The symbolic name of the material instance. */
	inline const fstring& GetSemantic() const { return semantic; }

	/** Sets the symbolic name used to link polygons sets with this instance.
		This name should always match against the symbolic name assigned to a polygons set.
		@see FCDGeometryPolygons.
		@param _semantic The new symbolic name of this instance. */
	inline void SetSemantic(const fchar* _semantic) { semantic = _semantic; SetDirtyFlag(); }
	inline void SetSemantic(const fstring& _semantic) { semantic = _semantic; SetDirtyFlag(); } /**< See above. */

	/** Retrieves the material entity used by this instance.
		@return The instanced material. */
	inline FCDMaterial* GetMaterial() { return (FCDMaterial*) GetEntity(); }
	inline const FCDMaterial* GetMaterial() const { return (const FCDMaterial*) GetEntity(); } /**< See above. */

	/** Sets the material entity used by this instance.
		@param _material The new instanced material. */
	inline void SetMaterial(FCDMaterial* _material) { SetEntity((FCDEntity*) _material); }

	/** Retrieves the parameter bindings of the instance.
		Each material instance may re-define any number of material or effect parameters.
		@return The parameter bindings. */
	DEPRECATED(3.05A, GetBindingCount and GetBinding(index)) inline void GetBindings() const {}

	/** Retrieves a given binding.
		@param semantic The binding's semantic.
		@return The found binding, or NULL if there is no such binding.*/
	const FCDMaterialInstanceBind* FindBinding(const char* semantic);

	/** Retrieves the number of parameter bindings for this instance.
		@return The number of parameter bindings. */
	inline size_t GetBindingCount() const { return bindings.size(); }

	/** Retrieves a parameter binding.
		@param index The index of the parameter binding.
		@return The parameter binding at the given index. */
	inline FCDMaterialInstanceBind* GetBinding(size_t index) { FUAssert(index < bindings.size(), return NULL); return bindings.at(index); }
	inline const FCDMaterialInstanceBind* GetBinding(size_t index) const { FUAssert(index < bindings.size(), return NULL); return bindings.at(index); } /**< See above. */

	/** Retrieves the geometry target that this instance affects.
		Note that this function uses the parent geometry instance and searches for the polygon set.
		Therefore, the application should buffer the retrieved pointer. */
	FCDObject* GetGeometryTarget();

	/** Adds a new parameter binding.
		@return An empty parameter binding. */
	FCDMaterialInstanceBind* AddBinding();
	
	/** Adds a new parameter binding.
		@param semantic The token that identifies the material or effect parameter.
		@param target The fully-qualified target pointer to the bind scene graph object.
		@return The parameter binding. */
	FCDMaterialInstanceBind* AddBinding(const char* semantic, const char* target);

	/** Removes a parameter binding.
		@param index The index of the parameter binding to remove. */
	void RemoveBinding(size_t index);

	/** Retrieves the vertex input bindings.
		@return The vertex input bindings. */
	DEPRECATED(3.05A, GetVertexInputBindingCount and GetVertexInputBinding(index)) inline void GetVertexInputBindings() {}

	/** Retrieves the number of vertex input bindings.
		@return The number of vertex input bindings. */
	inline size_t GetVertexInputBindingCount() const { return vertexBindings.size(); }

	/** Retrieves a vertex input binding.
		@param index The index of the vertex input binding.
		@return The vertex input binding at the given index. */
	inline FCDMaterialInstanceBindVertexInput* GetVertexInputBinding(size_t index) { FUAssert(index < vertexBindings.size(), return NULL); return vertexBindings.at(index); }
	inline const FCDMaterialInstanceBindVertexInput* GetVertexInputBinding(size_t index) const { FUAssert(index < vertexBindings.size(), return NULL); return vertexBindings.at(index); } /**< See above. */

	/** Retrieves a given vertex input binding.
		This is useful when trying to match textures with the texture coordinate sets.
		@param semantic A given vertex input binding semantic.
		@return The vertex input binding information structure. This pointer will be
			NULL if the given semantic is not bound within the material instance. */
	inline FCDMaterialInstanceBindVertexInput* FindVertexInputBinding(const char* semantic) { return const_cast<FCDMaterialInstanceBindVertexInput*>(const_cast<const FCDMaterialInstance*>(this)->FindVertexInputBinding(semantic)); }
	const FCDMaterialInstanceBindVertexInput* FindVertexInputBinding(const char* semantic) const; /**< See above. */

	/** Adds a new vertex input binding.
		@return An empty vertex input binding. */
	FCDMaterialInstanceBindVertexInput* AddVertexInputBinding();

	/** Adds a new vertex input binding.
		Together, the data source type and set should always define a unique data source.
		@param semantic The token identify the shader varying input.
		@param inputSemantic The polygons set data source type to bind.
		@param inputSet The polygons set data source set to bind.
		@return The new vertex input binding. */
	FCDMaterialInstanceBindVertexInput* AddVertexInputBinding(const char* semantic, FUDaeGeometryInput::Semantic inputSemantic, int32 inputSet);


	/** Creates a flattened version of the instantiated material. This is the
		preferred way to generate viewer materials from a COLLADA document.
		@return The flattened version of the instantiated material. You
			will need to delete this pointer manually. This pointer will
			be NULL when there is no material attached to this instance. */
	DEPRECATED(3.05A, not recommended) FCDMaterial* FlattenMaterial() { return NULL; }

	/** Clones the material instance.
		@param clone The material instance to become the clone.
		@return The cloned material instance. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;
};

#endif // _FCD_MATERIAL_BIND_H_
