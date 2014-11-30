/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectParameter.h
	This file contains the FCDEffectParameter interface and the simpler of its derivate classes:
	FCDEffectParameterString, FCDEffectParameterFloat, FCDEffectParameterVector...
*/

#ifndef _FCD_EFFECT_PARAMETER_H_
#define _FCD_EFFECT_PARAMETER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDocument;
class FCDEffectParameterAnnotation;

/**
	A COLLADA effect parameter.

	This interface class is used to define all the valid
	ColladaFX parameter types. There are many types of
	parameters: integers, booleans, floating-point
	values, 2D, 3D and 4D vectors of floating-point values,
	matrices, strings, surfaces and their samplers.

	A COLLADA effect parameter may generate a new
	effect parameter, in which case it will declare a semantic
	and a reference: to represent it within the COLLADA document.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectParameter : public FCDObject
{
public:
	/** The value type of the effect parameter. */
	enum Type
	{
		SAMPLER, /**< A sampler effect parameter. Points towards a surface parameter and adds extra texturing parameters. */
		INTEGER, /**< A single integer effect parameter. */
		BOOLEAN, /**< A single boolean effect parameter. */
		FLOAT, /**< A single floating-pointer value effect parameter. */
		FLOAT2, /**< A 2D vector of floating-pointer values. */
		FLOAT3, /**< A 3D vector of floating-pointer values. */
		VECTOR, /**< A 4D vector of floating-pointer values. */
		MATRIX, /**< A 4x4 matrix. */
		STRING, /**< A string effect parameter. */
		SURFACE /**< A surface effect parameter. Contains a COLLADA image pointer. */
	};

	/** The usage type of the effect parameter. */
	enum ParamType
	{
		GENERATOR, /**< This should be the type used for a 'newparam' element. 
						Valid only at the levels of effect, profile common or technique common.*/
		MODIFIER, /**< This should be the type used for a 'setparam' element.
					   Valid only at the instance effect level. */
		ANIMATOR, /**< This should be the type used for a 'param' element that is to be animated. 
					   Valid only at the bind_material level.*/
		REFERENCER, /**< This should be the type used for a 'param' element used in any of the common techniques.
						 The 'param' needs to have a reference that is the same as the generator used above.*/
		CONSTANT /**< This should be the type used for a parameter that is either a color or a float in 
					  any of the common techniques. This type isn't linked to any parameter elsewhere.*/
	};

private:
	DeclareObjectType(FCDObject);

	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, paramType, FC("Parameter Type")); // ParamType 
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, reference, FC("Identifier"));
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, semantic, FC("Semantic")); // this is a COLLADA Semantic, not a Cg semantic
	DeclareParameterContainer(FCDEffectParameterAnnotation, annotations, FC("Annotations"));
	
public:
	/** Constructor: do not use directly.
		Instead, use the appropriate AddEffectParameter function.
		@param document The COLLADA document that owns the effect parameter. */
	FCDEffectParameter(FCDocument* document);

	/** Destructor. */
	virtual ~FCDEffectParameter();

	/** Retrieves the type of effect parameter class.
		@return The type of the effect parameter class.*/
	virtual Type GetType() const = 0;

	/** Retrieves the type of parameter.
		The parameter can be a generator (newparam), a modifier (setparam) 
		or an animated param (animator).
		@return The type of the parameter. */
	inline ParamType GetParamType() const { return (ParamType) *paramType; }

	/** Retrieves the reference for this effect parameter.
		In the case of generators, the reference string contains the sub-id.
		@return The reference. */
	inline const fm::string& GetReference() const { return reference; }

	/** Sets the reference for the effect parameter.
		In the case of generators, the reference string contains the sub-id.
		@param _reference The reference. */
	void SetReference(const char* _reference);

	/** Retrieves the semantic for this effect parameter.
		@return The semantic. */
	inline const fm::string& GetSemantic() const { return semantic; }

	/** Sets the semantic for this effect parameter.
		@param _semantic The semantic. */
	void SetSemantic(const char* _semantic) { semantic = _semantic; SetDirtyFlag(); } 

	/** Retrieves whether this effect parameter is a parameter generator.
		A ColladaFX parameter must be generated to be modified or bound at
		higher abstraction levels.
		@return Whether this is a generator. */
	inline bool IsGenerator() const { return paramType == GENERATOR; }

	/** Sets this effect parameter as a generator. */
	inline void SetGenerator() { paramType = GENERATOR; SetDirtyFlag(); }

	/** Retrieves whether this effect parameter is a parameter modifier.
		A ColladaFX parameter must be generated to be modified or bound at
		higher abstraction levels.
		@return Whether this is a modifier. */
	inline bool IsModifier() const { return paramType == MODIFIER; }

	/** Sets this effect parameter as a modified. */
	inline void SetModifier() { paramType = MODIFIER; SetDirtyFlag(); }

	/** Retrieves whether this effect is an animated parameter.
		A ColladaFX parameter must be generated to be modified or bound at
		higher abstraction levels.
		@return Whether this is an animator. */
	inline bool IsAnimator() const { return paramType == ANIMATOR; }

	/** Sets this effect parameter as animated. */
	inline void SetAnimator() { paramType = ANIMATOR; SetDirtyFlag(); }

	/** Retrieves whether this effect is a referenced parameter.
		This type of parameter must have a valid reference equal
		to that of the generator paramater in order to be properly linked.
		@return Whether this is a referencer. */
	inline bool IsReferencer() const { return paramType == REFERENCER; }

	/** Sets this effect parameter as referencer. */
	inline void SetReferencer() { paramType = REFERENCER; SetDirtyFlag(); }

		/** Retrieves whether this effect is a constant parameter.
		This type of parameter belongs to the technique and isn't
		referenced anywhere. It should have an empty reference.
		@return Whether this is a constant parameter. */
	inline bool IsConstant() const { return paramType == CONSTANT; }

	/** Sets this effect parameter as constant. */
	inline void SetConstant() { paramType = CONSTANT; SetDirtyFlag(); }

	/**	Retrieves the list of annotations for this parameter.
		@return The list of annotations. */
	inline const FCDEffectParameterAnnotation** GetAnnotations() const { return annotations.begin(); } /**< See above. */
	
	/** Retrieves the number of annotations for this parameter.
		@return The number of annotations. */
	inline size_t GetAnnotationCount() const { return annotations.size(); }

	/** Retrieves an annotation of this parameter.
		@param index The index of the annotation.
		@return The annotation for the given index. This pointer will be NULL
			if the index is out-of-bounds. */
	inline FCDEffectParameterAnnotation* GetAnnotation(size_t index) { FUAssert(index < GetAnnotationCount(), return NULL); return annotations.at(index); }
	inline const FCDEffectParameterAnnotation* GetAnnotation(size_t index) const { FUAssert(index < GetAnnotationCount(), return NULL); return annotations.at(index); } /**< See above. */

	/** Adds a blank annotation to this parameter.
		@return The blank annotation. */
	FCDEffectParameterAnnotation* AddAnnotation();

	/** Adds an annotation to this parameter.
		@param name The name of the annotation.
		@param type The type of the annotation.
		@param value The value of the annotation. */
	void AddAnnotation(const fchar* name, FCDEffectParameter::Type type, const fchar* value);
	inline void AddAnnotation(const fstring& name, FCDEffectParameter::Type type, const fchar* value) { AddAnnotation(name.c_str(), type, value); } /**< See above. */
	inline void AddAnnotation(const fchar* name, FCDEffectParameter::Type type, const fstring& value) { AddAnnotation(name, type, value.c_str()); } /**< See above. */
	inline void AddAnnotation(const fstring& name, FCDEffectParameter::Type type, const fstring& value) { AddAnnotation(name.c_str(), type, value.c_str()); } /**< See above. */
	template <class T> inline void AddAnnotation(const fchar* name, FCDEffectParameter::Type type, const T& value) { FUStringBuilder builder; builder.set(value); AddAnnotation(name, type, builder.ToCharPtr()); } /**< See above. */
	template <class T> inline void AddAnnotation(const fstring& name, FCDEffectParameter::Type type, const T& value) { FUStringBuilder builder; builder.set(value); AddAnnotation(name.c_str(), type, builder.ToCharPtr()); } /**< See above. */

	/** Releases an annotation of this parameter.
		@param annotation The annotation to release. */
	DEPRECATED(3.05A, annotation->Release) inline void ReleaseAnnotation(FCDEffectParameterAnnotation* annotation) { ((FCDObject*) annotation)->Release(); }

	/** Compares this parameter's value with another
		@param parameter The given parameter to compare with.
		@return true if the values are equal */
	virtual bool IsValueEqual(FCDEffectParameter *parameter) = 0;

	/** Creates a full copy of the effect parameter.
		@param clone The cloned effect parameter. If this pointer is NULL,
			a new effect parameter will be created and you
			will need to delete this pointer.
		@return The cloned effect parameter. */
	virtual FCDEffectParameter* Clone(FCDEffectParameter* clone = NULL) const;

	/** [INTERNAL] Overwrites the target parameter with this parameter.
		This function is used during the flattening of materials.
		@param target The target parameter to overwrite. */
	virtual void Overwrite(FCDEffectParameter* target);
};

/**
	A COLLADA non-animatable effect parameter template.
	@ingroup FCDEffect
*/
template <class PrimitiveType>
class FCOLLADA_EXPORT FCDEffectParameterT : public FCDEffectParameter
{
private:
	DeclareObjectType(FCDEffectParameter);
	DeclareParameter(PrimitiveType, FUParameterQualifiers::SIMPLE, value, FC("Value"));

public:
	/** Constructor: do not use directly.
		Instead, use the appropriate AddEffectParameter function.
		@param document The COLLADA document that owns the effect parameter. */
	FCDEffectParameterT(FCDocument* document);

	/** Destructor. */
	virtual ~FCDEffectParameterT();

	/** Retrieves the type of effect parameter class.
		@return The parameter class type. */
	virtual Type GetType() const;

	/** Retrieves the value of the effect parameter.
		@return The integer value. */
	inline const PrimitiveType& GetValue() const { return *value; }

	/** Sets the integer value of the effect parameter.
		@param _value The integer value. */
	inline void SetValue(const PrimitiveType& _value) { value = _value; SetDirtyFlag(); }

	/** Compares this parameter's value with another
		@param parameter The given parameter to compare with.
		@return true if the values are equal */
	virtual bool IsValueEqual(FCDEffectParameter *parameter);

	/** Creates a full copy of the effect parameter.
		@param clone The cloned effect parameter. If this pointer is NULL,
			a new effect parameter will be created and you
			will need to delete this pointer.
		@return The cloned effect parameter. */
	virtual FCDEffectParameter* Clone(FCDEffectParameter* clone = NULL) const;

	/** [INTERNAL] Overwrites the target parameter with this parameter.
		This function is used during the flattening of materials.
		@param target The target parameter to overwrite. */
	virtual void Overwrite(FCDEffectParameter* target);
};

typedef FCDEffectParameterT<int32> FCDEffectParameterInt; /**< An integer effect parameter. */
typedef FCDEffectParameterT<bool> FCDEffectParameterBool; /**< A boolean effect parameter. */
typedef FCDEffectParameterT<fm::string> FCDEffectParameterString; /**< A string effect parameter. */

/**
	A COLLADA non-animatable effect parameter template.
	@ingroup FCDEffect
*/
template <class PrimitiveType, int Qualifiers>
class FCOLLADA_EXPORT FCDEffectParameterAnimatableT : public FCDEffectParameter
{
public:
	/** The supported types of float-point values. */
	enum FloatType
	{
		FLOAT, /** A full-fledged floating-point value. This is the default. */
		HALF /** Probably implies a 16-bit floating-point value. */
	};

private:
	DeclareObjectType(FCDEffectParameter);
	FloatType floatType;
	DeclareParameterAnimatable(PrimitiveType, Qualifiers, value, FC("Value"));

public:
	/** Constructor: do not use directly.
		Instead, use the appropriate AddEffectParameter function.
		@param document The COLLADA document that owns the effect parameter. */
	FCDEffectParameterAnimatableT(FCDocument* document);

	/** Destructor. */
	virtual ~FCDEffectParameterAnimatableT();

	/** Retrieves the type of effect parameter class.
		@return The parameter class type. */
	virtual Type GetType() const;

	/** Retrieves the type of floating-point value held by this effect parameter.
		@return The type of floating-point value. */
	inline FloatType GetFloatType() const { return floatType; }

	/** Sets the type of floating-point value held by this effect parameter.
		@param type The type of floating-point value. */
	inline void SetFloatType(FloatType type) { floatType = type; SetDirtyFlag(); }

	/** Retrieves the value of the effect parameter.
		@return The integer value. */
	inline FCDParameterAnimatableT<PrimitiveType, Qualifiers>& GetValue() { return value; }
	inline const FCDParameterAnimatableT<PrimitiveType, Qualifiers>& GetValue() const { return value; } /**< See above. */

	/** Sets the integer value of the effect parameter.
		@param _value The integer value. */
	inline void SetValue(const PrimitiveType& _value) { value = _value; SetDirtyFlag(); }

	/** Compares this parameter's value with another
		@param parameter The given parameter to compare with.
		@return true if the values are equal */
	virtual bool IsValueEqual(FCDEffectParameter *parameter);

	/** Creates a full copy of the effect parameter.
		@param clone The cloned effect parameter. If this pointer is NULL,
			a new effect parameter will be created and you
			will need to delete this pointer.
		@return The cloned effect parameter. */
	virtual FCDEffectParameter* Clone(FCDEffectParameter* clone = NULL) const;

	/** [INTERNAL] Overwrites the target parameter with this parameter.
		This function is used during the flattening of materials.
		@param target The target parameter to overwrite. */
	virtual void Overwrite(FCDEffectParameter* target);
};

typedef FCDEffectParameterAnimatableT<float, FUParameterQualifiers::SIMPLE> FCDEffectParameterFloat; /**< An effect parameter containing one floating point value. */
typedef FCDEffectParameterAnimatableT<FMVector2, FUParameterQualifiers::SIMPLE> FCDEffectParameterFloat2; /**< An effect parameter containing a 2D floating point vector. */
typedef FCDEffectParameterAnimatableT<FMVector3, FUParameterQualifiers::VECTOR> FCDEffectParameterFloat3; /**< An effect parameter containing a 3D floating point vector. */
typedef FCDEffectParameterAnimatableT<FMVector3, FUParameterQualifiers::COLOR> FCDEffectParameterColor3; /**< An effect parameter containing a 3D color value. */
typedef FCDEffectParameterAnimatableT<FMVector4, FUParameterQualifiers::VECTOR> FCDEffectParameterVector; /**< An effect parameter containing a 4D floating point vector. */
typedef FCDEffectParameterAnimatableT<FMVector4, FUParameterQualifiers::COLOR> FCDEffectParameterColor4; /**< An effect parameter containing a 4D color value. */
typedef FCDEffectParameterAnimatableT<FMMatrix44, FUParameterQualifiers::SIMPLE> FCDEffectParameterMatrix; /**< An effect parameter containing a 4x4 matrix. */

/**
	A ColladaFX annotation.

	ColladaFX annotations are used mainly to describe the user
	interface necessary to modify a parameter. Common annotations
	are "UIMin", "UIMax" and "UIWidget".
*/
class FCOLLADA_EXPORT FCDEffectParameterAnnotation : public FUParameterizable
{
private:
	DeclareObjectType(FUParameterizable);

public:
    /** Constructor. Do not use directly.
        Instead, use FCDEffectParameter::AddAnnotation. */
	FCDEffectParameterAnnotation();
    /** Destructor. */
	virtual ~FCDEffectParameterAnnotation();

	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, name, FC("Name")); /**< The annotation name. */
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, type, FC("Value Type")); /**< The annotation value type (FCDEffectParameter::Type). */
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, value, FC("Value")); /**< The annotation value. */
};

#if defined(__APPLE__) || defined(LINUX)
#include <FCDocument/FCDEffectParameter.hpp>
#endif

#endif // _FCD_EFFECT_PARAMETER_H_

