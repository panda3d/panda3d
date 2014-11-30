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
	@file FCDTexture.h
	This file contains the FCDTexture class.
*/

#ifndef _FCD_TEXTURE_H_
#define _FCD_TEXTURE_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDEffectParameter;
class FCDEffectParameterSampler;
class FCDEffectStandard;
class FCDImage;

#if defined(WIN32)
template <class T> class FCOLLADA_EXPORT FCDEffectParameterT; /**< Trick Doxygen. */
#elif defined(LINUX) || defined(__APPLE__)
template <class T> class FCDEffectParameterT; /**< Trick Doxygen. */
#endif // LINUX
typedef FCDEffectParameterT<int32> FCDEffectParameterInt; /**< An integer effect parameter. */

/**
	A COLLADA texture.
	
	Textures are used by the COMMON profile materials.
	As per the COLLADA 1.4 specification, a texture is
	used to match some texture coordinates with a surface sampler, on a given
	texturing channel.

	Therefore: textures hold the extra information necessary to place an image correctly onto
	polygon sets. This extra information includes the texturing coordinate transformations and
	the blend mode.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDTexture : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDEffectStandard* parent;
	DeclareParameterPtr(FCDEffectParameterSampler, sampler, FC("Sampler")); // Points to the surface, which points to the image.
	DeclareParameterRef(FCDEffectParameterInt, set, FC("Set")); // Always preset, this parameter hold the map channel/uv set index
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

public:
	/** Constructor. Do not use directly.
		Instead, use the FCDEffectStandard::AddTexture function.
		@param document The COLLADA document that owns this texture.
		@param parent The standard effect that contains this texture. */
	FCDTexture(FCDocument* document, FCDEffectStandard* parent = NULL);

	/** Destructor. */
	virtual ~FCDTexture();

	/** Access the parent standard effect or this texture.
		@return The parent effect.*/
	FCDEffectStandard* GetParent() const { return parent; }

	/** Retrieves the image information for this texture.
		@return The image. This pointer will be NULL if this texture is not yet
			tied to a valid image. */
	inline FCDImage* GetImage() { return const_cast<FCDImage*>(const_cast<const FCDTexture*>(this)->GetImage()); }
	const FCDImage* GetImage() const; /**< See above. */

	/** Set the image information for this texture.
		This is a shortcut that generates the sampler/surface parameters
		to access the given image.
		@param image The image information. This pointer may be
			NULL to disconnect an image. */
	void SetImage(FCDImage* image);

	/** Retrieves the surface sampler for this texture.
		@return The sampler. In the non-const method: the sampler
			will be created if it is currently missing and the parent is available. */
	FCDEffectParameterSampler* GetSampler();
	inline const FCDEffectParameterSampler* GetSampler() const { return sampler; } /**< See above. */

	/** Sets the targeted sampler.
		@param _sampler The new sampler. */
	inline void SetSampler(FCDEffectParameterSampler* _sampler) { sampler = _sampler; }

	/** Determines whether this texture targets a sampler.
		@return Whether the texture targets a sampler. */
	inline bool HasSampler() { return sampler != NULL; }

	/** Retrieves the texture coordinate set to use with this texture.
		This information is duplicated from the material instance abstraction level.
		@return The effect parameter containing the set. */
	inline FCDEffectParameterInt* GetSet() { return set; }
	inline const FCDEffectParameterInt* GetSet() const { return set; } /**< See above. */

	/** Retrieves the extra information tied to this texture.
		@return The extra tree. */
	inline FCDExtra* GetExtra() { return extra; }
	inline const FCDExtra* GetExtra() const { return extra; } /**< See above. */

	/** Clones the texture.
		@param clone The cloned texture. If this pointer is NULL,
			a new texture is created and you will need to release this new texture.
		@return The cloned texture. This pointer will never be NULL. */
	virtual FCDTexture* Clone(FCDTexture* clone = NULL) const;
};

#endif // _FCD_TEXTURE_H_
