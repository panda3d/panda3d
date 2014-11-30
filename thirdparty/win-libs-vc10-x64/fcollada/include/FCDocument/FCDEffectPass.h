/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectPass.h
	This file contains the FCDEffectPass class.
*/

#ifndef _FCD_EFFECT_PASS_H_
#define _FCD_EFFECT_PASS_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDEffectTechnique;
class FCDEffectParameter;
class FCDEffectPassShader;
class FCDEffectPassState;

/**
	A COLLADA effect pass.

	The effect pass contains a list of effect shaders. While they
	may be missing, it does not make sense for the effect pass to
	contain more than two shaders: a vertex shader and a fragment/pixel shader.

	For this reason, we provide the GetVertexShader and the GetFragmentShader
	which we expect will be used for most applications, rather than looking
	through the list of shader objects.
	
	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectPass : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDEffectTechnique* parent;
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, name, FC("Name"));
	DeclareParameterContainer(FCDEffectPassShader, shaders, FC("Shaders"));
	DeclareParameterContainer(FCDEffectPassState, states, FC("Render States"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffectTechnique::AddPass function.
		@param document The FCollada document that owns this pass.
		@param parent The effect technique that contains this effect pass. */
	FCDEffectPass(FCDocument* document, FCDEffectTechnique* parent);

	/** Destructor. */
	virtual ~FCDEffectPass();

	/** Retrieves the effect techniques which contains this effect pass.
		@return The parent technique. */
	inline FCDEffectTechnique* GetParent() { return parent; }
	inline const FCDEffectTechnique* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the COLLADA id of the parent effect.
		This function is mostly useful as a shortcut for debugging and reporting.
		@return The COLLADA id of the parent effect. */
	DEPRECATED(3.05A, GetParent()->GetParent()->GetParent()->GetDaeId()) const fm::string& GetDaeId() const { return emptyString; }

	/** Retrieves the sub-id of the effect pass.
		This sub-id is optional.
		@return The sub-id. */
	inline const fstring& GetPassName() const { return name; }

	/** Sets the optional sub-id for the effect pass.
		This sub-id is optional.
		@param _name The sub-id. */
	inline void SetPassName(const fchar* _name) { name = _name; SetDirtyFlag(); }

	/** Retrieves the number of shaders contained within the effect pass.
		@return The number of shaders. */
	inline size_t GetShaderCount() const { return shaders.size(); }

	/** Retrieves a specific shader.
		@param index The index of the shader.
		@return The shader. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDEffectPassShader* GetShader(size_t index) { FUAssert(index < GetShaderCount(), return NULL); return shaders.at(index); }
	inline const FCDEffectPassShader* GetShader(size_t index) const { FUAssert(index < GetShaderCount(), return NULL); return shaders.at(index); } /**< See above. */

	/** Adds a new shader to the pass.
		@return The new shader. */
	FCDEffectPassShader* AddShader();

	/** Releases a shader contained within the pass.
		@param shader The shader to release. */
	DEPRECATED(3.05A, shader->Release()) void ReleaseShader(FCDEffectPassShader* shader) { ((FCDObject*)shader)->Release(); }

	/** Retrieves the vertex shader for this effect pass.
		@return The vertex shader. This pointer will be NULL if no
			shader within the pass affects vertices. */
	inline FCDEffectPassShader* GetVertexShader() { return const_cast<FCDEffectPassShader*>(const_cast<const FCDEffectPass*>(this)->GetVertexShader()); }
	const FCDEffectPassShader* GetVertexShader() const; /**< See above. */

	/** Retrieves the fragment shader for this effect pass.
		@return The fragment shader. This pointer will be NULL if no
			shader within the pass affects pixels/fragments. */
	inline FCDEffectPassShader* GetFragmentShader() { return const_cast<FCDEffectPassShader*>(const_cast<const FCDEffectPass*>(this)->GetFragmentShader()); }
	const FCDEffectPassShader* GetFragmentShader() const; /**< See above. */

	/** Adds a new vertex shader to the pass.
		If a vertex shader already exists within the pass, it will be released.
		@return The new vertex shader. */
	FCDEffectPassShader* AddVertexShader();

	/** Adds a new fragment shader to the pass.
		If a fragment shader already exists within the pass, it will be released.
		@return The new fragment shader. */
	FCDEffectPassShader* AddFragmentShader();

	/** Retrieves the container of the render states for the pass.
		@return The render state list. */
	DEPRECATED(3.05A, GetRenderStateCount and GetRenderState(index)) void GetRenderStates() const {}

	/** Retrieves the number of render states defined for the pass.
		@return The render state count. */
	inline size_t GetRenderStateCount() const { return states.size(); }

	/** Retrieves a specific render state defined for the pass.
		@param index The index of the render state.
		@return The render state at the given index. */
	inline FCDEffectPassState* GetRenderState(size_t index) { FUAssert(index < states.size(), return NULL); return states.at(index); }
	inline const FCDEffectPassState* GetRenderState(size_t index) const { FUAssert(index < states.size(), return NULL); return states.at(index); } /**< See above. */

	/** Adds a new render state to the effect pass.
		Render states automatically get sorted by type.
		@param type The type of the render state to add.
			If a render state of this type already exists within the effect pass,
			it will be returned.
		@return A render state of the given type. */
	FCDEffectPassState* AddRenderState(FUDaePassState::State type);

	/** Retrieves a specific render state defined for the pass.
		@param type The type of the render state to retrieve.
		@return The render state with the given type. This pointer will be NULL
			if no render state has been defined for the given type. */
	inline FCDEffectPassState* FindRenderState(FUDaePassState::State type) { return const_cast<FCDEffectPassState*>(const_cast<const FCDEffectPass*>(this)->FindRenderState(type)); }
	const FCDEffectPassState* FindRenderState(FUDaePassState::State type) const; /**< See above. */

	/** Clones the effect pass and shaders.
		@param clone The cloned pass.
			If this pointer is NULL, a new pass is created and
			you will need to release this new pass.
		@return The cloned pass. */
	FCDEffectPass* Clone(FCDEffectPass* clone = NULL) const;
};

#endif
