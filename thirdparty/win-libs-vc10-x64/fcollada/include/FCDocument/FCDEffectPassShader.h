/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectPassShader.h
	This file contains the FCDEffectPassShader and the FCDEffectPassBind classes.
*/

#ifndef _FCD_EFFECT_PASS_SHADER_H_
#define _FCD_EFFECT_PASS_SHADER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDocument;
class FCDEffectCode;

/**
	A COLLADA shader binding.

	Binds an external symbol to a COLLADA effect parameter, by reference.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectPassBind : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

public:
	/** Constructor.
		@param document The document that owns this binding. */
	FCDEffectPassBind(FCDocument* document);

	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, reference, FC("Parameter Reference")); /**< A COLLADA effect parameter reference. */
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, symbol, FC("Shader Symbol")); /**< An external symbol, used within the shader code. */
};

/**
	A COLLADA shader.

	The shader abstraction level in ColladaFX is contained within the effect passes.
	There are two types of shaders: vertex shaders and fragment/pixel shaders.
	A COLLADA shader contains a list of bindings to attach the effect parameters to the
	shader input parameters.

	The shader object also contains the compiler information necessary to build
	the shader: its code, the compiler target and the compiler options.
*/
class FCOLLADA_EXPORT FCDEffectPassShader : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDEffectPass* parent;
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, name, FC("Name"));
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, compilerTarget, FC("Compiler Target"));
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, compilerOptions, FC("Compiler Options"));
	DeclareParameterContainer(FCDEffectPassBind, bindings, FC("Shader Bindings"));
	DeclareParameterPtr(FCDEffectCode, code, FC("Shader Code"));
	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isFragment, FC("Is Fragment Shader"));

public:
	/** Constructor: do not use directly. Instead, use the FCDEffectPass::AddShader,
		FCDEffectPass::AddVertexShader or FCDEffectPass::AddFragmentShader functions.
		@param parent The effect pass that contains this shader. */
	FCDEffectPassShader(FCDocument* document, FCDEffectPass* parent);

	/** Destructor. */
	virtual ~FCDEffectPassShader();

	/** Retrieves the effect pass that contains this shader.
		@return The effect pass. */
	inline FCDEffectPass* GetParent() { return parent; }
	inline const FCDEffectPass* GetParent() const { return parent; } /**< See above. */

	/** Sets this shader as affecting vertices.
		This sets the stage of the shader to the vertex pipeline. */
	inline void AffectsVertices() { isFragment = false; SetDirtyFlag(); }

	/** Sets this shader as affecting fragments/pixels.
		This sets the stage of the shader to the fragment/pixel pipeline. */
	inline void AffectsFragments() { isFragment = true; SetDirtyFlag(); }

	/** Retrieves whether this shader affects fragments/pixels.
		@return Whether this shader affects fragments/pixels. */
	inline bool IsFragmentShader() const { return isFragment; }

	/** Retrieves whether this shader affects vertices.
		@return Whether this shader affects vertices. */
	inline bool IsVertexShader() const { return !isFragment; }

	/** Retrieves the list of bindings for this shader.
		@return The list of bindings. */
	DEPRECATED(3.05A, GetBindingCount and GetBinding(index)) void GetBindings() const {}

	/** Retrieves the number of bindings for this shader.
		@return The number of bindings. */
	inline size_t GetBindingCount() const { return bindings.size(); }
	
	/** Retrieves a binding contained in this shader.
		@param index The index of the binding.
		@return The binding. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDEffectPassBind* GetBinding(size_t index) { FUAssert(index < GetBindingCount(), return NULL); return bindings.at(index); }
	inline const FCDEffectPassBind* GetBinding(size_t index) const { FUAssert(index < GetBindingCount(), return NULL); return bindings.at(index); } /**< See above. */

	/** Retrieves a binding for a given COLLADA reference.
		@param reference The reference of the parameter binding.
		@return The binding. This pointer will be NULL if
			the parameter is not bound in this shader. */
	const FCDEffectPassBind* FindBindingReference(const char* reference) const;
	inline FCDEffectPassBind* FindBindingReference(const char* reference) { return const_cast<FCDEffectPassBind*>(const_cast<const FCDEffectPassShader*>(this)->FindBindingReference(reference)); } /**< See above. */
	
	/** Retrieves a binding for a given FX symbol.
		@param symbol The symbol of the parameter binding.
		@return The binding. This pointer will be NULL if
			the parameter is not bound in this shader. */
	const FCDEffectPassBind* FindBindingSymbol(const char* symbol) const;
	inline FCDEffectPassBind* FindBindingSymbol(const char* symbol) { return const_cast<FCDEffectPassBind*>(const_cast<const FCDEffectPassShader*>(this)->FindBindingSymbol(symbol)); } /**< See above. */

	/** Adds a new binding to this shader.
		@return The new binding. */
	FCDEffectPassBind* AddBinding();

	/** Releases a binding contained within this shader.
		@param binding The binding to release. */
	DEPRECATED(3.05A, binding->Release()) void ReleaseBinding(FCDEffectPassBind* binding) { SAFE_RELEASE(binding); }

	/** Retrieves the compiler target information.
		The validity of this string depends on the type of the profile that contains this shader.
		@return The compiler target information string. */
	inline const fstring& GetCompilerTarget() const { return compilerTarget; }

	/** Sets the compiler target information string.
		The validity of this string depends on the type of the profile that contains this shader.
		@param _compilerTarget The compiler target information. */
	inline void SetCompilerTarget(const fchar* _compilerTarget) { compilerTarget = _compilerTarget; SetDirtyFlag(); }

	/** Retrieves the compiler option string.
		The validity of this string depends on the type of the profile that contains this shader.
		@return The compiler option string. */
	inline const fstring& GetCompilerOptions() const { return compilerOptions; }

	/** Sets the compiler option string.
		The validity of this string depends on the type of the profile that contains this shader.
		@param _compilerOptions The compiler option string. */
	inline void SetCompilerOptions(const fchar* _compilerOptions) { compilerOptions = _compilerOptions; SetDirtyFlag(); }

	/** Retrieves the sub-id of the shader.
		@return The sub-id. */
	inline const fm::string& GetName() const { return name; }

	/** Sets the sub-id of the shader.
		@param _name The sub-id. */
	inline void SetName(const char* _name) { name = _name; SetDirtyFlag(); }

	/** Retrieves the code inclusion that contains the code for this shader.
		@return The code inclusion. This pointer will be NULL if this shader
			is not yet attached to any code. */
	inline FCDEffectCode* GetCode() { return code; }
	inline const FCDEffectCode* GetCode() const { return code; } /**< See above. */

	/** Sets the code inclusion that contains the code for this shader.
		@param _code The code inclusion. This pointer will be NULL to detach
			 a shader from its code. */
	inline void SetCode(FCDEffectCode* _code) { code = _code; SetDirtyFlag(); }

	/** Clones this shader.
		@param clone The cloned shader. If this pointer is NULL,
			a new shader is created and you will need to release this new shader.
		@return The cloned shader. */
	FCDEffectPassShader* Clone(FCDEffectPassShader* clone) const;
};

#endif // _FCD_EFFECT_PASS_SHADER_H_
