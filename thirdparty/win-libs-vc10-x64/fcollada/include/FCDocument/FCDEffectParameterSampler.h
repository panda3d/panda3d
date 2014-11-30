/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectParameterSampler.h
	This file contains the FCDEffectParameterSampler class.
*/

#ifndef _FCD_EFFECT_PARAMETER_SAMPLER_H_
#define _FCD_EFFECT_PARAMETER_SAMPLER_H_

#ifndef _FCD_EFFECT_PARAMETER_H_
#include "FCDocument/FCDEffectParameter.h"
#endif // _FCD_EFFECT_PARAMETER_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDEffectPass;
class FCDEffectParameterSurface;

/**
	A COLLADA sampler effect parameter.
	A sampler parameter provides the extra texturing information necessary
	to correctly sample a surface parameter.
	There are four types of samplers supported: 1D, 2D, 3D and cube.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectParameterSampler : public FCDEffectParameter
{
public:
	/** The type of sampling to execute. */
	enum SamplerType
	{
		SAMPLER1D, /** 1D sampling. */
		SAMPLER2D, /** 2D sampling. */
		SAMPLER3D, /** 3D sampling. */
		SAMPLERCUBE /** Cube-map sampling. */
	};

private:
	DeclareObjectType(FCDEffectParameter);

	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, samplerType, FC("Sampler Type")); // SamplerType
	DeclareParameterPtr(FCDEffectParameterSurface, surface, FC("Surface"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, wrap_s, FC("Wrap Mode S")); // FUDaeTextureWrapMode::WrapMode
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, wrap_t, FC("Wrap Mode T"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, wrap_p, FC("Wrap Mode P"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, minFilter, FC("Min Filter")); // FUDaeTextureFilterFunction::FilterFunction
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, magFilter, FC("Mag Filter"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, mipFilter, FC("Mip Filter"));

public:
	/** Constructor: do not use directly.
		Instead, use the appropriate AddEffectParameter function.
		@param document The COLLADA document that owns the effect parameter. */
	FCDEffectParameterSampler(FCDocument* document);

	/** Destructor. */
	virtual ~FCDEffectParameterSampler();

	/** Retrieves the type of effect parameter class.
		@return The parameter class type: SAMPLER. */
	virtual Type GetType() const { return SAMPLER; }

	/** Retrieves the parameter for the surface to sample.
		@return The surface parameter. This pointer will be NULL if the sampler is
			not yet linked to any surface.. */
	FCDEffectParameterSurface* GetSurface() { return surface; }
	const FCDEffectParameterSurface* GetSurface() const { return surface; } /**< See above. */

	/** Sets the surface parameter for the surface to sample.
		@param surface The surface parameter. This pointer may be NULL
			to unlink the sampler. */
	void SetSurface(FCDEffectParameterSurface* surface);

	/** Retrieves the type of sampling to do.
		@return The sampling type. */
	SamplerType GetSamplerType() const { return (SamplerType) *samplerType; }

	/** Sets the type of sampling to do.
		@param type The sampling type. */
	void SetSamplerType(SamplerType type) { samplerType = type; SetDirtyFlag(); }

	/** Retrieves the wrap mode (in dimension S, T or P) of the sampler.
		@return The wrap mode.*/
	FUDaeTextureWrapMode::WrapMode GetWrapS() const { return (FUDaeTextureWrapMode::WrapMode) *wrap_s; }
	FUDaeTextureWrapMode::WrapMode GetWrapT() const { return (FUDaeTextureWrapMode::WrapMode) *wrap_t; } /**< See above.*/
	FUDaeTextureWrapMode::WrapMode GetWrapP() const { return (FUDaeTextureWrapMode::WrapMode) *wrap_p; } /**< See above.*/

	/** Sets the wrap mode (in dimension S, T or P) of the sampler.
		@param mode The wrap mode.*/
	void SetWrapS(FUDaeTextureWrapMode::WrapMode mode) { wrap_s = mode; SetDirtyFlag(); }
	void SetWrapT(FUDaeTextureWrapMode::WrapMode mode) { wrap_t = mode; SetDirtyFlag(); } /**< See above.*/
	void SetWrapP(FUDaeTextureWrapMode::WrapMode mode) { wrap_p = mode; SetDirtyFlag(); } /**< See above.*/

	/** Retrieves the appropriate filter function (minification, magnification or mip map filtering)
		of the sampler.
		@return The filter function.*/
	FUDaeTextureFilterFunction::FilterFunction GetMinFilter() const { return (FUDaeTextureFilterFunction::FilterFunction) *minFilter; }
	FUDaeTextureFilterFunction::FilterFunction GetMagFilter() const { return (FUDaeTextureFilterFunction::FilterFunction) *magFilter; } /**< See above.*/
	FUDaeTextureFilterFunction::FilterFunction GetMipFilter() const { return (FUDaeTextureFilterFunction::FilterFunction) *mipFilter; } /**< See above.*/

	/** Sets the appropriate filter function  (minification, magnification or mip map filtering)
		of the sampler.
		@param func The filter function.*/
	void SetMinFilter(FUDaeTextureFilterFunction::FilterFunction func) { minFilter = func; SetDirtyFlag(); }
	void SetMagFilter(FUDaeTextureFilterFunction::FilterFunction func) { magFilter = func; SetDirtyFlag(); } /**< See above.*/
	void SetMipFilter(FUDaeTextureFilterFunction::FilterFunction func) { mipFilter = func; SetDirtyFlag(); } /**< See above.*/

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

#endif // _FCD_EFFECT_PARAMETER_SAMPLER_H_
