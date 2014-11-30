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
	@file FCDLight.h
	This file contains the FCDLight class.
*/

#ifndef _FCD_LIGHT_H_
#define _FCD_LIGHT_H_

#ifndef _FCD_TARGETED_ENTITY_H_
#include "FCDocument/FCDTargetedEntity.h"
#endif // _FCD_TARGETED_ENTITY_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDocument;
class FCDSceneNode;
class FCDAnimated;

/**
	A COLLADA light.
	Based on the FCDTargetedEntity class to supported aimed lights.
	COLLADA defines four types of native lights: point, spot, ambient and directional.
	These four types are fully handled by this class: make sure to check the type flag
	as well as which information to expect for each light type.

	A COLLADA ambient light has a global color, which should be added to
	all other lighting on all geometry.

	A COLLADA directional light has a global color, which should be multiplied
	to the cosine of the angle between the normal vector of a triangle
	and the direction of the light. Note that the direction will be calculated
	from the transforms, for each instance, and is not provided by this class.

	A COLLADA point light has a color which attenuates as the distance increases
	between the light position and the vertex being shaded. Note that the position
	will be calculated from the transforms, for each instance,
	and is not provided by this class.

	A COLLADA spot light is a point light which lights only the objects that
	appear within a specific angle, with respect to the direction of the light.
	Note that the position and the direction will be calculated from the
	transforms, for each instance, and is not provided by this class.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDLight : public FCDTargetedEntity
{
public:
	/** The types of lights supported by this class. */
	enum LightType
	{
		POINT, /**< A point light. This is the default type. */
		SPOT, /**< A spot light. */
		AMBIENT, /**< An ambient light. */
		DIRECTIONAL /**< A directional light. */
	};

private:
	DeclareObjectType(FCDTargetedEntity);

	// Common Light parameters
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::COLOR, color, FC("Color"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, intensity, FC("Intensity")); // Non-standard COLLADA
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, lightType, FC("Light Type")); // LightType

	// Point and spot light parameters
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, constantAttenuationFactor, FC("Constant Attenuation Factor"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, linearAttenuationFactor, FC("Linear Attenuation Factor"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, quadracticAttenuationFactor, FC("Quadratic Attenuation Factor"));

	// Spot-specific light parameters
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, fallOffExponent, FC("Fall-off Exponent")); // Likely to be deprecated in future versions.
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, fallOffAngle, FC("Inner Cone Angle"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, outerAngle, FC("Outer Cone Angle")); // Non-standard COLLADA
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, penumbraAngle, FC("Penumbra Angle")); // *** DEPRECATED *** Replaced with the inner/outer angle.
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, dropoff, FC("Drop-off"));

public:
	/** Constructor: do not use directly. Create new lights using the FCDLibrary::AddEntity function.
		@param document The COLLADA document that contains this light entity. */
	FCDLight(FCDocument* document);

	/** Destructor. */
	virtual ~FCDLight();

	/** Retrieves the entity type for this class. This function is part of the FCDEntity interface.
		@return The entity type: LIGHT. */
	virtual Type GetType() const { return LIGHT; }

	/** Checks if the light uses a DCC specific representation
		Currently always returns false.
		@deprecated Instead use: GetExtra()->GetDefaultType()->GetTechniques()
		@return The DCC flag. */
	DEPRECATED(3.05A, GetExtra()->GetDefaultType()->GetTechniques()) virtual bool HasMaxExtras() const { return false; }
	DEPRECATED(3.05A, GetExtra()->GetDefaultType()->GetTechniques()) virtual bool HasMayaExtras() const { return false; } /**< See above. */

	/** [INTERNAL] Set DCC specific flags.
		@deprecated Internal method, should never have been used.
		@param value The new flag.
	*/
	DEPRECATED(3.05A, nothing) void SetHasMaxExtras(bool UNUSED(value)) { }
	DEPRECATED(3.05A, nothing) void SetHasMayaExtras(bool UNUSED(value)) { }

	/** Retrieves the base color for the light. To calculate the light color,
		multiply the base color with the intensity.
		@return The base color for the light. */
	FCDParameterAnimatableColor3& GetColor() { return color; }
	const FCDParameterAnimatableColor3& GetColor() const { return color; } /**< See above. */

	/** Sets the base color for the light. To calculate the light color,
		multiply the base color with the intensity.
		@param col The base color for the light. */
	void SetColor(const FMVector3& col) { color = col; SetDirtyFlag(); }

	/** Sets the base color for the light. To calculate the light color,
		multiply the base color with the intensity.
		@param r The red component for the light color.
		@param g The green component for the light color.
		@param b The blue component for the light color. */
	void SetColor(float r, float g, float b) { color = FMVector3(r, g, b); SetDirtyFlag(); }

	/** Retrieves the intensity of the light. To calculate the light color,
		multiply the base color with the intensity.
		@return The intensity of the light. */
	FCDParameterAnimatableFloat& GetIntensity() { return intensity; }
	const FCDParameterAnimatableFloat& GetIntensity() const { return intensity; } /**< See above. */

	/** Sets the intensity of the light. To calculate the light color,
		multiply the base color with the intensity.
		@param _intensity The intensity of the light. */
	void SetIntensity(float _intensity) { intensity = _intensity; SetDirtyFlag(); }

	/** Retrieves the type of the light.
		Make sure to check the type of light before using the values, as some values
		may not make sense with some types of light.
		@return The light type. */
	LightType GetLightType() const { return (LightType) *lightType; }

	/** Sets the type of the light. The default type of a new light is POINT.
		@param type The light type. */
	void SetLightType(LightType type) { lightType = type; SetDirtyFlag(); }

	/** Retrieves the constant attenuation factor for the light.
		This value is valid only for point and spot lights.
		@return The constant attenuation factor. */
	FCDParameterAnimatableFloat& GetConstantAttenuationFactor() { return constantAttenuationFactor; }
	const FCDParameterAnimatableFloat& GetConstantAttenuationFactor() const { return constantAttenuationFactor; } /**< See above. */

	/** Sets the constant attenuation factor for the light.
		This value is valid only for point and spot lights.
		@param factor The constant attenuation factor. */
	void SetConstantAttenuationFactor(float factor) { constantAttenuationFactor = factor; SetDirtyFlag(); }

	/** Retrieves the linear attenuation factor for the light.
		This value is valid only for point and spot lights.
		@return The linear attenuation factor. */
	FCDParameterAnimatableFloat& GetLinearAttenuationFactor() { return linearAttenuationFactor; }
	const FCDParameterAnimatableFloat& GetLinearAttenuationFactor() const { return linearAttenuationFactor; } /**< See above. */

	/** Sets the linear attenuation factor for the light.
		This value is valid only for point and spot lights.
		@param factor The linear attenuation factor. */
	void SetLinearAttenuationFactor(float factor) { linearAttenuationFactor = factor; SetDirtyFlag(); }

	/** Retrieves the quadratic attenuation factor for the light.
		This value is valid only for point and spot lights.
		@return The quadratic attenuation factor. */
	FCDParameterAnimatableFloat& GetQuadraticAttenuationFactor() { return quadracticAttenuationFactor; }
	const FCDParameterAnimatableFloat& GetQuadraticAttenuationFactor() const { return quadracticAttenuationFactor; } /**< See above. */

	/** Sets the quadratic attenuation factor for the light.
		This value is valid only for point and spot lights.
		@param factor The quadratic attenuation factor. */
	void SetQuadraticAttenuationFactor(float factor) { quadracticAttenuationFactor = factor; SetDirtyFlag(); }

	/** Retrieves the fall-off exponent for the light.
		This value is valid only for spot lights. It determines
		how fast the lighting turns off, with respect to
		angles greater than the fall-off angle. This results in a smooth
		lighting at the spot light's edges.
		
		IMPORTANT NOTE: Neither ColladaMaya or ColladaMax use this value
		as neither Maya or 3dsMax use this technique for soft lighting.

		@return The spot light fall-off exponent. */
	FCDParameterAnimatableFloat& GetFallOffExponent() { return fallOffExponent; }
	const FCDParameterAnimatableFloat& GetFallOffExponent() const { return fallOffExponent; } /**< See above. */

	/** Sets the fall-off exponent for the light.
		@see GetFallOffExponent
		@param exponent The spot light fall-off exponent. */
	void SetFallOffExponent(float exponent) { fallOffExponent = exponent; SetDirtyFlag(); }

	/** Retrieves the fall-off angle for the light.
		This value is valid only for spot lights. It defines
		the cone of the spot light.
		@return The spot light fall-off angle. */
	FCDParameterAnimatableFloat& GetFallOffAngle() { return fallOffAngle; }
	const FCDParameterAnimatableFloat& GetFallOffAngle() const { return fallOffAngle; } /**< See above. */

	/** Sets the fall-off angle for the light.
		@see GetFallOffAngle
		@param angle The spot light fall-off angle. */
	void SetFallOffAngle(float angle) { fallOffAngle = angle; SetDirtyFlag(); }

	/** Retrieves the outer angle for the light.
		This value is valid only for spot lights. This value is used
		by documents exported by ColladaMax and ColladaMaya. This value should always be
		greater than the fall-off angle. It represents the angle at which
		the lighting is black. All lighting between the fall-off angle and
		the outer angle is a linear interpolation between the light color
		and black.
		@return The spot light outer angle. */
	FCDParameterAnimatableFloat& GetOuterAngle() { return outerAngle; }
	const FCDParameterAnimatableFloat& GetOuterAngle() const { return outerAngle; } /**< See above. */

	/** Sets the outer angle for the light.
		@see GetOuterAngle
		@param angle The spot light outer angle. */
	void SetOuterAngle(float angle) { outerAngle = angle; SetDirtyFlag(); }

	/** Retrieves the penumbra angle for the light.
		This value is valid only for spot lights. The value is only used
		by documents exported by ColladaMaya. This value is relative to
		the fall-off angle and may be negative. If this value is positive,
		it determines the outer angle, as described above. If this value
		is negative, the fall-off angle is used as the outer angle and the
		fall-off angle + the penumbra angle is used as the full-lighting
		angle.
		This now actually does nothing but gets the value assigned in SetPenumbraAngle
		@deprecated Instead use: GetOuterAngle and GetFallOffAngle
		@see GetOuterAngle
		@return The spot light penumbra angle. */
	DEPRECATED(3.05A, GetOuterAngle and GetFallOffAngle) float& GetPenumbraAngle() { return penumbraAngle; }
	DEPRECATED(3.05A, GetOuterAngle and GetFallOffAngle) const float& GetPenumbraAngle() const { return penumbraAngle; } /**< See above. */

	/** Sets the penumbra angle for the light.
		This now actually doesn't nothing except sets a variable that you can read back with GetPenumbraAngle.
		@deprecated Instead use: SetOuterAngle and SetFallOffAngle, or FCDLightTools::LoadPenumbra
		@see GetPenumbraAngle
		@param angle The spot light penumbra angle. */
	DEPRECATED(3.05A, SetOuterAngle and SetFallOffAngle) void SetPenumbraAngle(float angle) { penumbraAngle = angle; }
	
	/** Retrieves the drop-off for the light.
		It defines the rate at which a spot light gets dimmer from the center
		of the beam to outside angles.
		@return The drop-off for the light. */
	FCDParameterAnimatableFloat& GetDropoff() { return dropoff; }
	const FCDParameterAnimatableFloat& GetDropoff() const { return dropoff; } /**< See above. */

	/** Sets the drop-off for the light.
		@param factor The drop-off for the light. */
	void SetDropoff(float factor) { dropoff = factor; SetDirtyFlag(); }
};

#endif // _FCD_LIGHT_H_

