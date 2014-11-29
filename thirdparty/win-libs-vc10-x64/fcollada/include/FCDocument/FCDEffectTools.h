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
	@file FCDEffectTools.h
	This file contains FCDEffectTools namespace.
*/

#ifndef _FCD_EFFECT_TOOLS_H_
#define _FCD_EFFECT_TOOLS_H_

#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

/** Contains functions to post-process parameters into a list from a specific instance of an effect.
	@ingroup FCDEffect */

class FCDMaterialInstance;
class FCDEffectStandard;
class FCDEffectTechnique;
class FCDGeometryInstance;

#if defined(WIN32)
template <class T, int Q> class FCOLLADA_EXPORT FCDEffectParameterAnimatableT; /**< Trick Doxygen. */
#elif defined(LINUX) || defined(__APPLE__)
template <class T, int Q> class FCDEffectParameterAnimatableT; /**< Trick Doxygen. */
#endif // LINUX
typedef FCDEffectParameterAnimatableT<FMVector4, 1> FCDEffectParameterColor4; /**< A RGBA color effect parameter. */
typedef FCDEffectParameterAnimatableT<float, 0> FCDEffectParameterFloat; /**< A floating-point value effect parameter. */

typedef fm::pvector<FCDEffectParameter> FCDEffectParameterList; /**< A dynamically-sized array of effect parameters. */

namespace FCDEffectTools 
{
	/** Synchronizes the standard effect parameters associated with 
		a material instance. Does nothing if there is no profile_COMMON.
		@param instance The specific material instance to be synchronized. */
	FCOLLADA_EXPORT void SynchronizeAnimatedParams(FCDGeometryInstance* geometryInstance, FCDMaterialInstance* materialInstance);

	/** {INTERNAL] Synchronizes a standard effect's float parameter from the three
		lists that can affect it with newparam's and setparam's.
		@param semantic The FCDEffectStandard semantic for the specific parameter.
		@param param One of the FCDEffectStandard color parameters.
		@param materialList The parameters list from a material instance.
		@param effectList The parameters list from the effect.
		@param profileList The parameters list from the effect profile. Should be COMMON. */
	FCOLLADA_EXPORT void LinkAnimatedParamCommonFloat(const fm::string& semantic, FCDEffectParameterFloat* param, FCDGeometryInstance* geometry, FCDMaterial* material, FCDEffect* effect, FCDEffectProfile* profile);

	/** [INTERNAL] Synchronizes a standard effect's vector parameter from the three
		lists that can affect it with newparam's and setparam's.
		@param semantic The FCDEffectStandard semantic for the specific parameter.
		@param param One of the FCDEffectStandard color parameters.
		@param materialList The parameters list from a material instance.
		@param effectList The parameters list from the effect.
		@param profileList The parameters list from the effect profile. Should be COMMON. */
	FCOLLADA_EXPORT void LinkAnimatedParamCommonVector(const fm::string& semantic, FCDEffectParameterColor4* param, FCDGeometryInstance* geometry, FCDMaterial* material, FCDEffect* effect, FCDEffectProfile* profile);

	/** document.... */
	FCOLLADA_EXPORT FCDParameterAnimatableColor4* GetAnimatedColor(FCDGeometryInstance* geometryInstance, FCDMaterial* material, const fm::string& semantic, bool* isFloat3);

	/** document... */
	FCOLLADA_EXPORT FCDParameterAnimatableFloat* GetAnimatedFloat(FCDGeometryInstance* geometryInstance, FCDMaterial* material, const fm::string& semantic);

	/** document... */
	FCOLLADA_EXPORT FMVector4* GetDefaultColor(FCDMaterial* material, const fm::string& semantic, bool* isFloat3);

	/** document... */
	FCOLLADA_EXPORT float* GetDefaultFloat(FCDMaterial* material, const fm::string& semantic);

	/** Retrieves a common effect parameter. Looks for the common effect parameter with the correct
		semantic, in order to bind or override its value.
		This function searches through the given level of abstractions and the ones below.
		@todo Some iterator structure to replace all these functions.
		@param materialInstance The material instance being considered.
		@param semantic The effect parameter semantic to match.
		@param localOnly Whether to search below this level of abstraction.
		@return The first effect parameter that matches the semantic.
			This pointer will be NULL if no effect parameter matches the given semantic. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDMaterialInstance* materialInstance, const char* semantic, bool localOnly = false);
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDMaterialInstance* materialInstance, const char* semantic, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDMaterialInstance*>(materialInstance), semantic, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDGeometryInstance* geometryInstance, const char* semantic); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDGeometryInstance* geometryInstance, const char* semantic) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDGeometryInstance*>(geometryInstance), semantic)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDMaterial* material, const char* semantic, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDMaterial* material, const char* semantic, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDMaterial*>(material), semantic, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDEffect* effect, const char* semantic, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDEffect* effect, const char* semantic, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDEffect*>(effect), semantic, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDEffectProfile* profile, const char* semantic, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDEffectProfile* profile, const char* semantic, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDEffectProfile*>(profile), semantic, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterBySemantic(const FCDEffectTechnique* technique, const char* semantic, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterBySemantic(FCDEffectTechnique* technique, const char* semantic, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterBySemantic(const_cast<const FCDEffectTechnique*>(technique), semantic, localOnly)); } /**< See above. */

	/** Retrieves a common effect parameter. Looks for the common effect parameter with the correct
		reference, in order to bind or override its value.
		This function searches through the given level of abstractions and the ones below.
		@todo Some iterator structure to replace all these functions.
		@param materialInstance The material instance being considered.
		@param reference The effect parameter reference to match.
		@return The first effect parameter that matches the reference.
			This pointer will be NULL if no effect parameter matches the given reference. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDMaterialInstance* materialInstance, const char* reference, bool localOnly = false);
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDMaterialInstance* materialInstance, const char* reference, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDMaterialInstance*>(materialInstance), reference, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDGeometryInstance* geometryInstance, const char* reference); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDGeometryInstance* geometryInstance, const char* reference) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDGeometryInstance*>(geometryInstance), reference)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDMaterial* material, const char* reference, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDMaterial* material, const char* reference, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDMaterial*>(material), reference, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDEffect* effect, const char* reference, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDEffect* effect, const char* reference, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDEffect*>(effect), reference, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDEffectProfile* profile, const char* reference, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDEffectProfile* profile, const char* reference, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDEffectProfile*>(profile), reference, localOnly)); } /**< See above. */
	FCOLLADA_EXPORT const FCDEffectParameter* FindEffectParameterByReference(const FCDEffectTechnique* technique, const char* reference, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline FCDEffectParameter* FindEffectParameterByReference(FCDEffectTechnique* technique, const char* reference, bool localOnly = false) { return const_cast<FCDEffectParameter*>(FindEffectParameterByReference(const_cast<const FCDEffectTechnique*>(technique), reference, localOnly)); } /**< See above. */

	/** Retrieves a subset of the common effect parameter list.
		Look for the effect parameter generators with the correct semantic.
		This function searches through the geometry instance and the level of abstractions below.
		@todo Some iterator structure to replace all these functions.
		@param materialInstance The material instance being considered.
		@param semantic The effect parameter semantic to match.
		@param parameters The list of parameters to fill in. This list is not cleared. */
	FCOLLADA_EXPORT void FindEffectParametersBySemantic(const FCDMaterialInstance* materialInstance, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false); 
	FCOLLADA_EXPORT inline void FindEffectParametersBySemantic(FCDMaterialInstance* materialInstance, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersBySemantic(const_cast<const FCDMaterialInstance*>(materialInstance), semantic, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersBySemantic(const FCDMaterial* material, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersBySemantic(FCDMaterial* material, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersBySemantic(const_cast<const FCDMaterial*>(material), semantic, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersBySemantic(const FCDEffect* effect, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersBySemantic(FCDEffect* effect, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersBySemantic(const_cast<const FCDEffect*>(effect), semantic, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersBySemantic(const FCDEffectProfile* profile, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersBySemantic(FCDEffectProfile* profile, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersBySemantic(const_cast<const FCDEffectProfile*>(profile), semantic, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersBySemantic(const FCDEffectTechnique* technique, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersBySemantic(FCDEffectTechnique* technique, const char* semantic, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersBySemantic(const_cast<const FCDEffectTechnique*>(technique), semantic, parameters, localOnly); } /**< See above. */

	/** Retrieves a subset of the common effect parameter list.
		Look for the effect parameter generators with the correct reference.
		This function searches through the geometry instance and the level of abstractions below.
		@todo Some iterator structure to replace all these functions.
		@param materialInstance The material instance being considered.
		@param reference The effect parameter reference to match. In the case of effect
			parameter generators, the reference is replaced by the sub-id.
		@param parameters The list of parameters to fill in. This list is not cleared. */
	FCOLLADA_EXPORT void FindEffectParametersByReference(const FCDMaterialInstance* materialInstance, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false); 
	FCOLLADA_EXPORT inline void FindEffectParametersByReference(FCDMaterialInstance* materialInstance, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersByReference(const_cast<const FCDMaterialInstance*>(materialInstance), reference, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersByReference(const FCDMaterial* material, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersByReference(FCDMaterial* material, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersByReference(const_cast<const FCDMaterial*>(material), reference, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersByReference(const FCDEffect* effect, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersByReference(FCDEffect* effect, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersByReference(const_cast<const FCDEffect*>(effect), reference, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersByReference(const FCDEffectProfile* profile, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersByReference(FCDEffectProfile* profile, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersByReference(const_cast<const FCDEffectProfile*>(profile), reference, parameters, localOnly); } /**< See above. */
	FCOLLADA_EXPORT void FindEffectParametersByReference(const FCDEffectTechnique* technique, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false); /**< See above. */
	FCOLLADA_EXPORT inline void FindEffectParametersByReference(FCDEffectTechnique* technique, const char* reference, FCDEffectParameterList& parameters, bool localOnly = false) { FindEffectParametersByReference(const_cast<const FCDEffectTechnique*>(technique), reference, parameters, localOnly); } /**< See above. */
};

#endif //_FCD_EFFECT_TOOLS_H_
