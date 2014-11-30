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
	@file FCDEffectStandard.h
	This file contains the FCDEffectStandard class.
*/

#ifndef _FCD_MATERIAL_STANDARD_H_
#define _FCD_MATERIAL_STANDARD_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FCD_EFFECT_PROFILE_H_
#include "FCDocument/FCDEffectProfile.h"
#endif // _FCD_EFFECT_PROFILE_H_
#ifndef _FCD_EFFECT_PARAMETER_H_
#include "FCDocument/FCDEffectParameter.h"
#endif // _FCD_EFFECT_PARAMETER_H_

class FCDocument;
class FCDEffect;
class FCDTexture;
class FCDEffectParameter;

#if defined(WIN32)
template <class T, int Q> class FCOLLADA_EXPORT FCDEffectParameterAnimatableT; /**< Trick Doxygen. */
#elif defined(LINUX) || defined(__APPLE__)
template <class T, int Q> class FCDEffectParameterAnimatableT; /**< Trick Doxygen. */
#endif // LINUX
typedef FCDEffectParameterAnimatableT<FMVector4, 1> FCDEffectParameterColor4; /**< A RGBA color effect parameter. */
typedef FCDEffectParameterAnimatableT<float, 0> FCDEffectParameterFloat; /**< A floating-point value effect parameter. */

/**
	A COMMON profile effect description.
	
	The COMMON effect profile holds the information necessary
	to render your polygon sets using the well-defined lighting models.

	COLLADA supports four lighting models: constant, Lambert, Phong and Blinn.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectStandard : public FCDEffectProfile
{
public:
	/** The list of the lighting models supported by the COMMON profile of COLLADA. */
	enum LightingType
	{
		/** The constant lighting model.
			This lighting model uses the emissive color everywhere, without
			any complex lighting calculations. It also uses the translucency
			factor and the translucency color, by multiplying them together
			and applying them to your standard alpha channel according to the
			final lighting color.*/
		CONSTANT, 

		/** The Lambert lighting model.
			This lighting model improves on the constant lighting model by
			using the dot-product between the normalized light vectors and the
			polygon normals to determine how much light should affect each polygon.
			This value is multiplied to the diffuse color and (1 + the ambient color). */
		LAMBERT,

		/** The Phong lighting model.
			This lighting model improves on the Lambert lighting model by
			calculating how much light is reflected by the polygons into the viewer's eye.
			For this calculation, the shininess, the specular color and the reflectivity is used. */
		PHONG,

		/** The Blinn lighting model.
			This lighting model improves on the Lambert lighting model by
			calculating how much light is reflected by the polygons into the viewer's eye.
			For this calculation, the shininess, the specular color and the reflectivity is used. */
		BLINN,

		/** Not a valid lighting model. */
		UNKNOWN
	};

	/** The list of transparency modes supported by the COMMON profile of COLLADA. */
	enum TransparencyMode
	{
		/** Takes the transparency information from the color's alpha channel, where the
			value 1.0 is opaque. */
		A_ONE,

		/** Takes the transparency information from the color's red, green, and blue channels,
			where the value 0.0 is opaque, with each channel modulated independently. */
		RGB_ZERO
	};

private:
	DeclareObjectType(FCDEffectProfile);

	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, type, FC("Lighting Type")); // LightingType;

	// Emission Channel
	DeclareParameterContainer(FCDTexture, emissionTextures, FC("Emission Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, emissionColor, FC("Emission Color"));
	DeclareParameterRef(FCDEffectParameterFloat, emissionFactor, FC("Emission Factor"));
	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isEmissionFactor, FC("Is Emission Factor"));

	// Reflectivity Channel
	DeclareParameterContainer(FCDTexture, reflectivityTextures, FC("Reflectivity Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, reflectivityColor, FC("Reflectivity Color"));
	DeclareParameterRef(FCDEffectParameterFloat, reflectivityFactor, FC("Reflectivity Factor"));
	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isReflective, FC("Is Reflective"));

	// Refractivity Channel
	DeclareParameterContainer(FCDTexture, refractionTextures, FC("Refraction Textures"));
	DeclareParameterRef(FCDEffectParameterFloat, indexOfRefraction, FC("Index of Refraction"));
	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isRefractive, FC("Is Refractive"));

	// Transparency Channel
	DeclareParameterContainer(FCDTexture, translucencyTextures, FC("Translucency Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, translucencyColor, FC("Translucency Color"));
	DeclareParameterRef(FCDEffectParameterFloat, translucencyFactor, FC("Translucency Factor"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, transparencyMode, FC("Translucency Mode")); // TransparencyMode

	// Diffuse Channel
	DeclareParameterContainer(FCDTexture, diffuseTextures, FC("Diffuse Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, diffuseColor, FC("Diffuse Color"));

	// Ambient Channel
	DeclareParameterContainer(FCDTexture, ambientTextures, FC("Ambient Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, ambientColor, FC("Ambient Color"));

	// Specular Channel
	DeclareParameterContainer(FCDTexture, specularTextures, FC("Specular Color Textures"));
	DeclareParameterRef(FCDEffectParameterColor4, specularColor, FC("Specular Color"));
	DeclareParameterContainer(FCDTexture, specularFactorTextures, FC("Specular Factor Textures"));
	DeclareParameterRef(FCDEffectParameterFloat, specularFactor, FC("Specular Factor"));

	// Specular Exponent Channel
	DeclareParameterContainer(FCDTexture, shininessTextures, FC("Shininess Textures"));
	DeclareParameterRef(FCDEffectParameterFloat, shininess, FC("Shininess Factor"));

	// Extra Channels
	DeclareParameterContainer(FCDTexture, bumpTextures, FC("Bump Maps"));
	DeclareParameterContainer(FCDTexture, displacementTextures, FC("Displacement Maps")); // Max-only
	DeclareParameterContainer(FCDTexture, filterTextures, FC("Filter Maps")); // Max-only

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffect::AddProfile function
		with the FUDaeProfileType::COMMON parameter.
		@param document The FCollada document that owns this effect standard.
		@param parent The effect that contains this profile. */
	FCDEffectStandard(FCDocument* document, FCDEffect* parent);

	/** Destructor. */
	virtual ~FCDEffectStandard();

	/** Retrieves the lighting model to be used for this profile.
		@return The lighting model. */
	inline LightingType GetLightingType() const { return (LightingType) *type; }

	/** Sets the lighting model to be used for this profile.
		Note that which parameters are exported depends on the lighting model.
		@param _type The lighting model. */
	inline void SetLightingType(LightingType _type) { type = _type; SetDirtyFlag(); }

	/** Retrieves the profile type for this effect.
		This function is a part of the FCDEffectProfile interface and allows you
		to up-cast an effect profile pointer safely to this class.
		@return The profile type: COMMON. */
	virtual FUDaeProfileType::Type GetType() const { return FUDaeProfileType::COMMON; }

	/** Retrieves the list of textures belonging to a specific channel.
		@param bucket A texture channel index. This index should match one
			of the values in the FUDaeTextureChannel enum.
		@return The list of textures for this channel. */
	inline FCDTexture** GetTextureBucket(uint32 bucket) { return const_cast<FCDTexture**>(const_cast<const FCDEffectStandard*>(this)->GetTextureBucket(bucket)); }
	const FCDTexture** GetTextureBucket(uint32 bucket) const; /**< See above. */

	/** Retrieves the number of textures belonging to a specific channel.
		@param bucket A texture channel index. This index should match one
			of the values in the FUDaeTextureChannel enum.
		@return The number of textures in that channel. */
	size_t GetTextureCount(uint32 bucket) const;

	/** Retrieves a texture
		@param bucket A texture channel index. This index should match one
			of the values in the FUDaeTextureChannel enum.
		@param index The index of a texture within this channel.
		@return The texture. This pointer will be NULL if either the bucket or the index is out-of-bounds. */
	inline FCDTexture* GetTexture(uint32 bucket, size_t index) { FUAssert(index < GetTextureCount(bucket), return NULL); return GetTextureBucket(bucket)[index]; }
	inline const FCDTexture* GetTexture(uint32 bucket, size_t index) const { FUAssert(index < GetTextureCount(bucket), return NULL); return GetTextureBucket(bucket)[index]; } /**< See above. */

	/** Adds a texture to a specific channel.
		@param bucket A texture channel index. This index should match one
			of the values in the FUDaeTextureChannel enum.
		@return The new texture. This pointer will be NULL if the bucket is out-of-bounds. */
	FCDTexture* AddTexture(uint32 bucket);

	/** Releases a texture contained within this effect profile.
		@param texture The texture to release. */
	DEPRECATED(3.05A, texture->Release()) void ReleaseTexture(FCDTexture* texture) { ((FUObject*) texture)->Release(); }

	/** Retrieves a parameter based on the semantic.
		@param semantic The semantic that names the param.
		@param isFloat Returned float indicating whether the parameter is a float or a vector.
		@return param The returned parameter. 
		Use isFloat to cast to FCDEffectParameterFloat or FCDEffectParameterColor4. */
	FCDEffectParameter* GetParam(const fm::string& semantic, bool* isFloat);

//
//	Translucency Color (vector)
//

	/** Retrieves the base translucency color.
		This value must be multiplied with the translucency factor
		to get the real translucency color. Use the RGB channels
		in case of a RGB_ZERO transparency mode, and the A channel in case
		of a A_ONE transparency mode
		This value is used in all lighting models.
		@return The base translucency color. */
	inline FMVector4& GetTranslucencyColor() { return translucencyColor->GetValue(); }
	inline const FMVector4& GetTranslucencyColor() const { return translucencyColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the translucency color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the translucency color. */
	inline FCDEffectParameterColor4* GetTranslucencyColorParam() { return translucencyColor; }
	inline const FCDEffectParameterColor4* GetTranslucencyColorParam() const { return translucencyColor; } /**< See above. */

	/** Sets the base translucency color.
		@param color The base translucency color including the alpha channel. */
	inline void SetTranslucencyColor(const FMVector4& color) { translucencyColor->SetValue(color); SetDirtyFlag(); }

//
// Translucency Factor (float)
//

	/** Retrieves the translucency factor.
		This value must be multiplied with the translucency color
		to get the real translucency color.
		This value is used in all lighting models.
		@return The translucency factor. */
	inline float& GetTranslucencyFactor() { return translucencyFactor->GetValue(); }
	inline const float& GetTranslucencyFactor() const { return translucencyFactor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the translucency factor.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the translucency factor. */
	inline FCDEffectParameterFloat* GetTranslucencyFactorParam() { return translucencyFactor; }
	inline const FCDEffectParameterFloat* GetTranslucencyFactorParam() const { return translucencyFactor; } /**< See above. */

	/** Sets the translucency factor.
		@param factor The translucency factor. */
	inline void SetTranslucencyFactor(float factor) { translucencyFactor->SetValue(factor); SetDirtyFlag(); }

//
// Transparency Mode (bool)
//

	/** Retrieves the transparency mode.
		@return The transparency mode. */
	inline TransparencyMode GetTransparencyMode() const { return (TransparencyMode) *transparencyMode; }

	/** Sets the transparency mode.
		@param mode The transparency mode. */
	inline void SetTransparencyMode(TransparencyMode mode) { transparencyMode = mode; SetDirtyFlag(); }

	/** Retrieves the flat opacity.
		This is a calculated value and will not take into consideration any animations
		that affect either the base translucency color or the translucency factor.
		This value can be used in all lighting models.
		@return The flat opacity. */
	float GetOpacity() const;

// 
// Emission Color (vector)
//

	/** Retrieves the base emission/self-illumination color.
		This value must be multiplied with the emission factor to get the real emission color.
		This value is used in all lighting models.
		@return The base emission color. */
	inline FMVector4& GetEmissionColor() { return emissionColor->GetValue(); }
	inline const FMVector4& GetEmissionColor() const { return emissionColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the emission color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the emission color. */
	inline FCDEffectParameterColor4* GetEmissionColorParam() { return emissionColor; }
	inline const FCDEffectParameterColor4* GetEmissionColorParam() const { return emissionColor; } /**< See above. */

	/** Sets the base emission/self-illumination color.
		@param color The base emission color. */
	inline void SetEmissionColor(const FMVector4& color) { emissionColor->SetValue(color); SetDirtyFlag(); }

//
// Emission Factor (float)
//

    /** Retrieves the emission/self-illumination factor.
		This value must be multiplied with the base emission color to get the real emission color.
		@return The emission factor. */
	inline float& GetEmissionFactor() { return emissionFactor->GetValue(); }
	inline const float& GetEmissionFactor() const { return emissionFactor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the emission/self-illumination factor.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the emission/self-illumination factor. */
	inline FCDEffectParameterFloat* GetEmissionFactorParam() { return emissionFactor; }
	inline const FCDEffectParameterFloat* GetEmissionFactorParam() const { return emissionFactor; } /**< See above. */

	/** Sets the emission/self-illumination factor.
		@param factor The emission factor. */
	inline void SetEmissionFactor(float factor) { emissionFactor->SetValue(factor); SetDirtyFlag(); }

//
// IsEmission Factor (bool)
//

	/** Retrieves whether the emission factor was used, rather than the emission color.
		This value is used in conjunction with 3dsMax, in which the self-illumination color
		and the self-illumination factor are mutually exclusive.
		@return Whether the emission factor is to be used. */
	inline bool IsEmissionFactor() const { return isEmissionFactor; }

	/** Sets whether the emission factor is to be used, rather than the emission color.
		This value is used in conjunction with 3dsMax, in which the self-illumination color
		and the self-illumination factor are mutually exclusive.
		@param useFactor Whether the emission factor should be used. */
	inline void SetIsEmissionFactor(bool useFactor) { isEmissionFactor = useFactor; SetDirtyFlag(); }

//
// Diffuse Color (vector)
// 

	/** Retrieves the diffuse color.
		This value is used in the Lambert lighting model.
		@return The diffuse color. */
	inline FMVector4& GetDiffuseColor() { return diffuseColor->GetValue(); }
	inline const FMVector4& GetDiffuseColor() const { return diffuseColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the diffuse color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the diffuse color. */
	inline FCDEffectParameterColor4* GetDiffuseColorParam() { return diffuseColor; }
	inline const FCDEffectParameterColor4* GetDiffuseColorParam() const { return diffuseColor; } /**< See above. */

	/** Sets the diffuse color.
		@param color The diffuse color. */
	inline void SetDiffuseColor(const FMVector4& color) { diffuseColor->SetValue(color); SetDirtyFlag(); }

//
// Ambient Color (vector)
//

	/** Retrieves the ambient color.
		This value is used in the Lambert lighting model.
		@return The ambient color. */
	inline FMVector4& GetAmbientColor() { return ambientColor->GetValue(); }
	inline const FMVector4& GetAmbientColor() const { return ambientColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the ambient color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the ambient color. */
	inline FCDEffectParameterColor4* GetAmbientColorParam() { return ambientColor; }
	inline const FCDEffectParameterColor4* GetAmbientColorParam() const { return ambientColor; } /**< See above. */

	/** Sets the ambient color.
		@param color The ambient color. */
	inline void SetAmbientColor(const FMVector4& color) { ambientColor->SetValue(color); SetDirtyFlag(); }

//
// Specular Color (vector)
//

	/** Retrieves the base specular color.
		This value must be multiplied with the specular factor
		to get the real specular color.
		This value is used in the Phong and Blinn lighting models.
		@return The specular color. */
	inline FMVector4& GetSpecularColor() { return specularColor->GetValue(); }
	inline const FMVector4& GetSpecularColor() const { return specularColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the specular color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the specular color. */
	inline FCDEffectParameterColor4* GetSpecularColorParam() { return specularColor; }
	inline const FCDEffectParameterColor4* GetSpecularColorParam() const { return specularColor; } /**< See above. */

	/** Sets the specular color.
		@param color The specular color. */
	inline void SetSpecularColor(const FMVector4& color) { specularColor->SetValue(color); SetDirtyFlag(); }

//
// Specular Factor (float)
//

	/** Retrieves the specular factor.
		This value must be multiplied with the base specular color
		to get the real specular color.
		This value is used in the Phong and Blinn lighting models.
		@return The specular factor. */
	inline float& GetSpecularFactor() { return specularFactor->GetValue(); }
	inline const float& GetSpecularFactor() const { return specularFactor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the specular factor.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the specular factor. */
	inline FCDEffectParameterFloat* GetSpecularFactorParam() { return specularFactor; }
	inline const FCDEffectParameterFloat* GetSpecularFactorParam() const { return specularFactor; } /**< See above. */

	/** Sets the specular factor.
		@param factor The specular factor. */
	inline void SetSpecularFactor(float factor) { specularFactor->SetValue(factor); SetDirtyFlag(); }

//
// Shininess (float)
//

	/** Retrieves the specular shininess.
		This value represents the exponent to which you must raise
		the dot-product between the view vector and reflected light vectors:
		as such, it is usually a number greater than 1.
		This value is used in the Phong and Blinn lighting models.
		@return The specular shininess. */
	inline float& GetShininess() { return shininess->GetValue(); }
	inline const float& GetShininess() const { return shininess->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the specular shininess.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the specular shininess. */
	inline FCDEffectParameterFloat* GetShininessParam() { return shininess; }
	inline const FCDEffectParameterFloat* GetShininessParam() const { return shininess; } /**< See above. */

	/** Sets the specular shininess.
		This value represents the exponent to which you must raise
		the dot-product between the view vector and reflected light vectors:
		as such, it is usually a number greater than 1.
		@param _shininess The specular shininess. */
	inline void SetShininess(float _shininess) { shininess->SetValue(_shininess); SetDirtyFlag(); }

//
// IsReflective (bool)
//

	/**	Retrieves the reflectivity state.
		@return True if the effect has reflectivity information.*/
	inline bool IsReflective() const { return isReflective; }

	/**	Sets the reflectivity state.
		@param r The effect's new reflectivity state.*/
	inline void SetReflective(bool r) { isReflective = r; }

//
// Reflectivity Color (vector)
//

	/** Retrieves the base reflectivity color.
		This value must be multiplied to the reflectivity factor to
		get the real reflectivity color.
		This value is used in the Phong and Blinn lighting models.
		@return The base reflectivity color. */
    inline FMVector4& GetReflectivityColor() { return reflectivityColor->GetValue(); }
    inline const FMVector4& GetReflectivityColor() const { return reflectivityColor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the reflectivity color.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple color. 
		@return The parameter that contains the reflectivity color. */
	inline FCDEffectParameterColor4* GetReflectivityColorParam() { return reflectivityColor; }
	inline const FCDEffectParameterColor4* GetReflectivityColorParam() const { return reflectivityColor; } /**< See above. */

	/** Sets the base reflectivity color.
		@param color The base reflectivity color. */
	inline void SetReflectivityColor(const FMVector4& color) { reflectivityColor->SetValue(color); isReflective = true; SetDirtyFlag(); }

// 
// Reflectivity Factor (float)
//

	/** Retrieves the reflectivity factor.
		This value must be multiplied to the base reflectivity color
		to get the real reflectivity color.
		This value is used in the Phong and Blinn lighting models.
		@return The reflectivity factor. */
	inline float& GetReflectivityFactor() { return reflectivityFactor->GetValue(); }
	inline const float& GetReflectivityFactor() const { return reflectivityFactor->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the reflectivity factor.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the reflectivity factor. */
	inline FCDEffectParameterFloat* GetReflectivityFactorParam() { return reflectivityFactor; }
	inline const FCDEffectParameterFloat* GetReflectivityFactorParam() const { return reflectivityFactor; } /**< See above. */

	/** Sets the reflectivity factor.
		@param factor The reflectivity factor. */
	inline void SetReflectivityFactor(float factor) { reflectivityFactor->SetValue(factor); isReflective = true; SetDirtyFlag(); }

	/** Retrieves the flat reflectivity.
		This is a calculated value and will not take into consideration any animations
		that affect either the base reflectivity color or the reflectivity factor.
		This value can be used in the Phong and Blinn lighting models.
		@return The flat reflectivity. */
	float GetReflectivity() const;

//
// IsRefractive (bool)
//

	/**	Retrieves the refractive state.
		@return True if the effect has refractive information.*/
	inline bool IsRefractive() const { return isRefractive; }

	/**	Sets the refractive state.
		@param r The effect's new refractive state.*/
	inline void SetRefractive(bool r) { isRefractive = r; }

//
// IndexOfRefraction (float)
//

	/** Retrieves the index of refraction.
		The index of refraction defaults to 1.0f.
		@return The index of refraction. */
	inline float& GetIndexOfRefraction() { return indexOfRefraction->GetValue(); }
	inline const float& GetIndexOfRefraction() const { return indexOfRefraction->GetValue(); } /**< See above. */

	/** Retrieves the parameter pointer that contains the index of refraction.
		Use this function to modify the value, the semantic, reference or annotations of the parameter.
		Note: in order to be linked with the profile_COMMON's parameter list, the parameter needs
		to have a valid semantic and reference. Otherwise it is treated as a simple float. 
		@return The parameter that contains the index of refraction. */
	inline FCDEffectParameterFloat* GetIndexOfRefractionParam() { return indexOfRefraction; }
	inline const FCDEffectParameterFloat* GetIndexOfRefractionParam() const { return indexOfRefraction; } /**< See above. */

	/** Sets the index of refraction.
		@param index The new index of refraction. */
	inline void SetIndexOfRefraction(float index) { indexOfRefraction->SetValue(index); isRefractive = true; SetDirtyFlag(); }

//
//	Miscellaneous
//

	/** Clones the COMMON profile effect and its parameters.
		@param clone The cloned profile.
			If this pointer is NULL, a new COMMON profile is created and
			you will need to release this pointer.
		@return The cloned COMMON profile. */
	virtual FCDEffectProfile* Clone(FCDEffectProfile* clone = NULL) const;

	/** [INTERNAL] Flattens the profile.
		Does nothing on the common profile. */
	DEPRECATED(3.05A, not recommended) void Flatten() {}

	/** Adds an extra attribute to the given profile
		@param profile The profile in which to insert the attribute.
		@param key The attribute's key
		@param value The attribute's value */
	void AddExtraAttribute(const char *profile, const char *key, const fchar *value);

	/** Get the extra attribute value corresponding to the given profile and key values
		@param profile The profile to look for.
		@param key The attribute key to look for. */
	const fchar* GetExtraAttribute(const char *profile, const char *key) const;

public:
	static const fm::string EmissionColorSemantic;
	static const fm::string EmissionFactorSemantic;
	static const fm::string ReflectivityColorSemantic;
	static const fm::string ReflectivityFactorSemantic;
	static const fm::string IndexOfRefractionSemantic;
	static const fm::string TranslucencyColorSemantic;
	static const fm::string TranslucencyFactorSemantic;
	static const fm::string DiffuseColorSemantic;
	static const fm::string AmbientColorSemantic;
	static const fm::string SpecularColorSemantic;
	static const fm::string SpecularFactorSemantic;
	static const fm::string ShininessSemantic;

};

#endif //_FCD_MATERIAL_STANDARD_H_

