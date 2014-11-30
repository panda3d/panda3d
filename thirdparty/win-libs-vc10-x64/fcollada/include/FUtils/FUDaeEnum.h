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

#ifndef _FU_DAE_ENUM_H_
#define _FU_DAE_ENUM_H_

#undef NEVER // 3dsMax defines NEVER in the global namespace.
#undef TRANSPARENT // Win32: GDI defines TRANSPARENT in the global namespace
#undef RELATIVE // Win32: GDI defines RELATIVE in the global namespace
#undef IN
#undef OUT
#undef DIFFERENCE

/** Contains the animation curve interpolation types and their conversion functions. */
namespace FUDaeInterpolation
{
	/** An animation curve interpolation type. */
	enum Interpolation
	{
		STEP = 0, /**< No interpolation. 
					Uses the output value of the previous key until the next key time is reached. */
		LINEAR, /**< Linear interpolation. Uses the average slope between the previous key and the next key. */
		BEZIER, /**< Bezier interpolation. Uses two 2D control points for each segment, wrongly called in-tangent and out-tangent in COLLADA. */
		TCB, /**< TCB interpolation. Uses Tension, Continuity and Bias values to generate Hermite tangents.
				This interpolation type is not standard COLLADA. */

		UNKNOWN, /**< An unknown interpolation type. */
		DEFAULT = STEP,
	};

	/** Converts the COLLADA interpolation type string into an interpolation type.
		@param value The COLLADA interpolation type string.
		@return The interpolation type. */
	FCOLLADA_EXPORT Interpolation FromString(const fm::string& value);

	/** Converts the interpolation type into its COLLADA type string.
		@param value The interpolation type.
		@return The COLLADA interpolation type string. */
	FCOLLADA_EXPORT const char* ToString(const Interpolation& value);
};

/** A dynamically-sized array of animation curve interpolation types. */
typedef fm::vector<FUDaeInterpolation::Interpolation> FUDaeInterpolationList;

/** Contains the spline segment interpolation types and their conversion functions. */
namespace FUDaeSplineType
{
	/** A spline segment interpolation type. */
	enum Type
	{
		LINEAR = 0, /**< Linear interpolation. */
		BEZIER, /**< Bezier interpolation. */
		NURBS, /**< NURBS interpolation. This is the default interpolation type. */

		UNKNOWN, /**< An unknown interpolation type. */
		DEFAULT = NURBS,
	};

	/** Converts a COLLADA spline interpolation type string into a spline interpolation type.
		@param value The COLLADA spline interpolation type string.
		@return The interpolation type. */
	FCOLLADA_EXPORT Type FromString(const fm::string& value);

	/** Converts the spline interpolation type into its COLLADA type string.
		@param value The spline interpolation type.
		@return The COLLADA spline interpolation type string. */
	FCOLLADA_EXPORT const char* ToString(const Type& value);
};

/** Contains the spline form types and their conversion functions. */
namespace FUDaeSplineForm
{
	/** A spline form. */
	enum Form
	{
		OPEN = 0, /**< The first and last spline vertices are not connected.
						This is the default spline form. */
		CLOSED, /**< The first and last spline vertices are connected. */

		UNKNOWN, /**< An unknown spline form. */
		DEFAULT = OPEN,
	};

	/** Converts a COLLADA spline form string into a spline form.
		@param value The COLLADA spline form string.
		@return The spline form. */
	FCOLLADA_EXPORT Form FromString(const fm::string& value);

	/** Converts a spline form into its COLLADA spline form string.
		@param value The spline form.
		@return The COLLADA spline form string. */
	FCOLLADA_EXPORT const char* ToString(const Form& value);
};


/** Contains the texture channels and their conversion functions. */
namespace FUDaeTextureChannel
{
	/** A texture channel.
		Used by standard effects to assign textures to channels.
		Multi-texturing is done by assigning more than one texture per slot.
		Defaults to diffuse texture slot. */
	enum Channel
	{
		AMBIENT = 0, /**< The texels will be modulated with the ambient light colors. */
		BUMP, /**< The texels will re-orient the geometric normals. */
		DIFFUSE, /**< The texels will be modulated with the non-ambient light colors. */
		DISPLACEMENT, /**< The texels will displace the pixel positions. */
		EMISSION, /**< The texels will be added to the final color directly. */
		FILTER, /**< Max-specific. */
		REFLECTION, /**< The texels will modify the pixel reflection factor. */
		REFRACTION, /**< The texels will modify the pixel refraction factor. */
		SHININESS, /**< The texels will modify the specular shininess of the pixel. */
		SPECULAR, /**< The texels will be modulated with the specular light colors. */
		SPECULAR_LEVEL, /**< The texels will be modulated with the specular light colors. */
		TRANSPARENT, /**< The texels will be modify the final color alpha. */

		COUNT, /**< The number of texture channels. */
		UNKNOWN, /**< An unknown texture channel. */
		DEFAULT = DIFFUSE,
	};

	/** Converts the FCollada texture channel string into a texture channel.
		@param value The FCollada texture channel string.
		@return The texture channel. */
	FCOLLADA_EXPORT Channel FromString(const fm::string& value);
};

/** Contains the texture wrap modes.*/
namespace FUDaeTextureWrapMode
{
	enum WrapMode
	{
		NONE = 0,
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,

		UNKNOWN,
		DEFAULT = WRAP,
	};

	/** Converts the COLLADA wrap mode string into a wrap mode.
		@param value The COLLADA wrap mode string.
		@return The wrap mode. */
	FCOLLADA_EXPORT WrapMode FromString(const char* value);
	inline WrapMode FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the wrap mode into its COLLADA wrap mode string.
		@param wrap The wrap mode.
		@return The COLLADA wrap mode string. */
	FCOLLADA_EXPORT const char* ToString(WrapMode wrap);
};

/** Contains the texture filter functions.*/
namespace FUDaeTextureFilterFunction
{
	enum FilterFunction
	{
		NONE = 0,
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR,

		UNKNOWN,
		DEFAULT = NONE,
	};

	/** Converts the COLLADA filter function string into a filter function.
		@param value The COLLADA filter function string.
		@return The filter function. */
	FCOLLADA_EXPORT FilterFunction FromString(const char* value);
	inline FilterFunction FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the filter function into its COLLADA filter function string.
		@param function The filter function.
		@return The COLLADA filter function string. */
	FCOLLADA_EXPORT const char* ToString(FilterFunction function);
};

/** Contains the morphing method for morph controllers and their conversion functions. */
namespace FUDaeMorphMethod
{
	/** A morphing method.

		Whether the vertex position is relative or absolute is irrelevant,
		as long as you use the correct weight generation function:
		NORMALIZED: base_weight = 1.0f - SUM(weight[t])
		RELATIVE: base_weight = 1.0f, and position[k] = SUM(weight[t][k] * position[t][k]). */
	enum Method
	{
		NORMALIZED = 0, /**< Implies that the morph targets all have absolute vertex positions. */
		RELATIVE, /**< Implies that the morph targets have relative vertex positions. */

		UNKNOWN, /**< An unknown morphing method. */
		DEFAULT = NORMALIZED,
	};

	/** Converts the COLLADA morph method string into a morph method.
		@param value The COLLADA morph method string.
		@return The morph method. */
	FCOLLADA_EXPORT Method FromString(const char* value);

	/** Converts the morph method into its COLLADA morph method string.
		@param method The morph method.
		@return The COLLADA morph method string. */
	FCOLLADA_EXPORT const char* ToString(Method method);
	inline Method FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */
};

/** Contains the infinity types and their conversion functions. */ 
namespace FUDaeInfinity
{
	/** An infinity type.
		They determine what happens when evaluating an animation curve outside of its bounds. */
	enum Infinity
	{
		CONSTANT = 0, /**< Uses the output value of the closest animation key. This is the default infinity type. */
		LINEAR, /**< Takes the distance between the closest animation key input value and the evaluation time.
					Multiplies this distance against the instant slope at the closest animation key and offsets the
					result with the closest animation key output value. */
		CYCLE, /**< Iteratively removes or adds the animation curve time length to the evaluation time until it is
					within the animation curve time interval and evaluates it. */
		CYCLE_RELATIVE, /**< Iteratively removes or adds the animation curve time length to the evaluation time until it is
							within the animation curve time interval and evaluates it. Adds to the evaluation output the
							number of iteration done multiplied by the difference between the animation curve
							start and end key outputs. */
		OSCILLATE, /**< Iteratively removes or adds the animation curve time length to the evaluation time until it is
						within the animation curve time interval. If the number of iterations done is even, evaluate the
						new evaluation time, otherwise evaluate (animation curve time length - evaluation time). */

		UNKNOWN, /**< An unknown infinity type. */
		DEFAULT = CONSTANT
	};

	/** Converts the FCollada infinity type string into a infinity type.
		@param value The FCollada infinity type string.
		@return The infinity type. */
	FCOLLADA_EXPORT Infinity FromString(const char* value);
	
	/** Converts the infinity type into its FCollada infinity type string.
		@param infinity The infinity type.
		@return The infinity type string. */
	FCOLLADA_EXPORT const char* ToString(Infinity infinity);
	inline Infinity FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */
};

/** Contains the multi-texture blend modes and their conversion functions. */
namespace FUDaeBlendMode
{
	/** A multi-texture blend mode. */
	enum Mode
	{
		NONE, /**< No blending. This is the default blend mode. */
		OVER, /**< Replaces the previous texture with the current one. */
		IN, /**< ?. */
		OUT, /**< ?. */
		ADD, /**< Adds the color values from the previous texture to the current one. */
		SUBTRACT, /**< Substracts the color values from the current texture to the previous one. */
		MULTIPLY, /**< Modulates the previous texture with the current one. */
		DIFFERENCE, /**< Substracts the color values from the previous texture to the current one. */
		LIGHTEN, /**< ?. */
		DARKEN, /**< ?. */
		SATURATE, /**< ?. */
		DESATURATE, /**< ?. */
		ILLUMINATE, /**< ?. */

		UNKNOWN, /**< An unknown blend mode. */
		DEFAULT = NONE,
	};

	/** Converts the FCollada multi-texture mode string into a blend mode.
		@param value The FCollada multi-texture mode string.
		@return The blend mode. */
	FCOLLADA_EXPORT Mode FromString(const char* value);

	/** Converts the blend mode into its FCollada multi-texture mode string.
		@param mode The multi-texture blend mode.
		@return The FCollada mode string. */
	FCOLLADA_EXPORT const char* ToString(Mode mode);
	inline Mode FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */
}

/** Contains the geometry source data types and their conversion functions. */
namespace FUDaeGeometryInput
{
	/** The geometry source data types. */
	enum Semantic
	{
		POSITION = 0, /**< Used for position data. */
		VERTEX, /**< Used to input the vertex sources within a polygon set. */
		NORMAL, /**< Used for surface normal vectors. */
		GEOTANGENT, /**< Used for surface tangent vectors. */
		GEOBINORMAL, /**< Used for surface binormal vectors. */
		TEXCOORD, /**< Used for texture coordinates. */
		TEXTANGENT, /**< Used for texture-aligned surface tangent vectors. */
		TEXBINORMAL, /**< Used for texture-aligned surface binormal vectors. */
		UV, /**< Used for generic mapping parameters. Do not confuse this with TEXCOORD. */
		COLOR, /**< Used for vertex colors. */
		EXTRA, /**< Used for application-specific vertex data. This is Maya-specific to support blind data. */

		POINT_SIZE,	/**< Used to define the size of the Billboard generated by a given point.
						This is a type::POINTS specific type.  If the size is 0, a single pixel pt is rendered. */
		POINT_ROTATION, /**< Used to give a billboard orientation.  This is a type::POINTS 
							specific type, and is not compulsory */
		UNKNOWN = -1, /**< An unknown data source. */
	};
	typedef fm::vector<Semantic> SemanticList; /**< A dynamically-sized array of geometry source data types. */

	/** Converts the COLLADA geometry input string to a geometry source data type.
		@param value The COLLADA geometry input string.
		@return The geometry source data type. */
	FCOLLADA_EXPORT Semantic FromString(const char* value);

	/** Converts the geometry source data type into a COLLADA geometry input string.
		@param semantic The geometry source data type.
		@return The COLLADA geometry input string. */
    FCOLLADA_EXPORT const char* ToString(Semantic semantic);
	inline Semantic FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */
}

/** The types of effect profiles. */
namespace FUDaeProfileType
{
	/** Enumerates the types of effect profiles. */
	enum Type
	{
		CG, /**< The CG profile. */
		HLSL, /**< The HLSL profile for DirectX. */
		GLSL, /**< The GLSL profile for OpenGL. */
		GLES, /**< The GLES profile for OpenGL ES. */
		COMMON, /**< The common profile which encapsulates the standard lighting equations: Lambert, Phong, Blinn. */

		UNKNOWN /**< Not a valid profile type. */
	};

	/** Converts the COLLADA profile name string to a profile type.
		Examples of COLLADA profile name strings are 'profile_CG' and 'profile_COMMON'.
		@param value The profile name string.
		@return The profile type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the profile type to its COLLADA profile name string.
		Examples of COLLADA profile name strings are 'profile_CG' and 'profile_COMMON'.
		@param type The profile type.
		@return The name string for the profile type. */
	FCOLLADA_EXPORT const char* ToString(Type type);
}

/** The function types for the effect pass render states. */
namespace FUDaePassStateFunction
{
	/** Enumerates the COLLADA render state function types.
		The usage of this enumeration type depends on the render state.
		For the alpha-test and the depth-test, this function determines
		when a given pixel will be kept when compared against the
		color/depth buffer's current alpha/depth value. */
	enum Function
	{
		NEVER = 0x0200, /**< Never kept. */
		LESS = 0x0201, /**< The pixel is kept when its value is lesser to the current value. */
		EQUAL = 0x0202, /**< The pixel is kept when its value is equal to the current value. */
		LESS_EQUAL = 0x0203, /**< The pixel is kept when its value is lesser or equal to the current value. */
		GREATER = 0x0204, /**< The pixel is kept when its value is greater to the current value. */
		NOT_EQUAL = 0x0205, /**< The pixel is kept when its value is not equal to the current value. */
		GREATER_EQUAL = 0x0206, /**< The pixel is kept when its value is greater or equal to the current value. */
		ALWAYS = 0x0207, /** Always kept. */

		INVALID /**< An invalid function. */
	};

	/** Converts the COLLADA render state function string to the render state function type.
		@param value The render state function string.
		@return The render state function type. */
	FCOLLADA_EXPORT Function FromString(const char* value);
	inline Function FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the render state function type to its COLLADA render state function string.
		@param fn The render state function type.
		@return The render state function string. */
	FCOLLADA_EXPORT const char* ToString(Function fn);
};

/** The stencil operation types for the effect pass render states. */
namespace FUDaePassStateStencilOperation
{
	/** Enumerates the COLLADA stencil operation types. */
	enum Operation
	{
		KEEP = 0x1E00, /**< Keep the current stencil value. */
		ZERO = 0x0000, /**< Set the stencil value to zero. */
		REPLACE = 0x1E01, /**< Replace the stencil value with the new value. */
		INCREMENT = 0x1E02, /**< Increment the stencil value, stop at maximum value. */
		DECREMENT = 0x1E03, /**< Decrement the stencil value, stop at zero. */
		INVERT = 0x1E0A, /**< Invert the stencil value. On a 8-bit stencil buffer,
							  this implies the following: StencilValue = 255 - StencilValue. */
		INCREMENT_WRAP = 0x8507, /**< Increment the stencil value and if the maximum value is reached, set to zero. */
		DECREMENT_WRAP = 0x8508, /**< Decrement the stencil value and if zero is reached, set to the maximum value. */

		INVALID /**< An invalid stencil operation. */
	};

	/** Converts the COLLADA render state stencil operation string to the stencil operation type.
		@param value The render state stencil operation string.
		@return The stencil operation type. */
	FCOLLADA_EXPORT Operation FromString(const char* value);
	inline Operation FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the stencil operation type to its COLLADA render state function string.
		@param op The stencil operation type.
		@return The render state stencil operation string. */
	FCOLLADA_EXPORT const char* ToString(Operation op);
};

/** The render state blend types for effect passes. */
namespace FUDaePassStateBlendType
{
	/** Enumerates the COLLADA render state blend types.
		These are applied to either the destination or the source color.

		The most common blend type is (SOURCE_ALPHA, ONE_MINUS_SOURCE_ALPHA)
		where the first blend type is applied on the source color and the
		second blend type is applied on the destination color to give the
		following equation: SrcColor * SrcAlpha + DestColor * (1.0 - SrcAlpha). */
	enum Type
	{
		ZERO = 0x0000, /**< The color is not used. */
		ONE = 0x0001, /**< The color is taken as is. */
		SOURCE_COLOR = 0x0300, /**< The color is multiplied with the source color. */
		ONE_MINUS_SOURCE_COLOR = 0x0301, /**< The color is multiplied with the inverse of the source color. */
		DESTINATION_COLOR = 0x0306, /**< The color is multiplied with the destination color. */
		ONE_MINUS_DESTINATION_COLOR = 0x0307, /**< The color is multiplied with the inverse of the destination color. */
		SOURCE_ALPHA = 0x0302, /**< The color is multiplied with the source alpha. */
		ONE_MINUS_SOURCE_ALPHA = 0x0303, /**< The color is multiplied with the inverse of the source alpha. */
		DESTINATION_ALPHA = 0x0304, /**< The color is multiplied with the destination alpha. */
		ONE_MINUS_DESTINATION_ALPHA = 0x0305, /**< The color is multiplied with the inverse of the destination alpha. */
		CONSTANT_COLOR = 0x8001, /**< The color is multiplied with a constant color value. */
		ONE_MINUS_CONSTANT_COLOR = 0x8002, /**< The color is multiplied with the inverse of a constant color value. */
		CONSTANT_ALPHA = 0x8003, /**< The color is multiplied with a constant alpha value. */
		ONE_MINUS_CONSTANT_ALPHA = 0x8004, /**< The color is multiplied with the inverse of a constant alpha value. */
		SOURCE_ALPHA_SATURATE = 0x0308, /**< The color is multiplied with the saturation of the source alpha value. */

		INVALID /**< An invalid blend color type. */
	};

	/** Converts the COLLADA render state blend type string to the blend type.
		@param value The render state blend type string.
		@return The blend type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the blend type to its COLLADA render state blend type string.
		@param type The blend type.
		@return The render state blend type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render state face types. */
namespace FUDaePassStateFaceType
{
	/** The render state face types.
		The usage of this enumerated type depends on
		the specific render state that uses it. */
	enum Type
	{
		FRONT = 0x0404, /**< Front faces. */
		BACK = 0x0405, /**< Back faces. */
		FRONT_AND_BACK = 0x0408, /**< Front and bank faces. */

		INVALID /**< An invalid face type. */
	};

	/** Converts the COLLADA render state face type string to the blend type.
		@param value The render state face type string.
		@return The face type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the face type to its COLLADA render state face type string.
		@param type The face type.
		@return The render state face type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render state blend equation types. */
namespace FUDaePassStateBlendEquation
{
	/** A render state blend equation formula. */
	enum Equation
	{
		ADD = 0x8006, /**< Will add the destination color with the source color. */
		SUBTRACT = 0x800A, /**< Will substract the source color to the destination color. */
		REVERSE_SUBTRACT = 0x800B, /**< Will substract the destination color to the source color. */
		MIN = 0x8007, /**< Will take the smaller of the destination color and the source color components. */
		MAX = 0x8008, /**< Will take the larger of the destination color and the source color components. */

		INVALID /**< An unknown blend equation. */
	};

	/** Converts the COLLADA render state face type string to the blend equation type.
		@param value The render state blend equation type string.
		@return The face type. */
	FCOLLADA_EXPORT Equation FromString(const char* value);
	inline Equation FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the blend equation type to its COLLADA render state blend equation type string.
		@param equation The blend equation type.
		@return The render state blend equation type string. */
	FCOLLADA_EXPORT const char* ToString(Equation equation);
};

/** The render state material types. */
namespace FUDaePassStateMaterialType
{
	/** The applicable material type. */
	enum Type
	{
		EMISSION = 0x1600, /**< An emissive material. */
		AMBIENT = 0x1200, /**< An ambient material. */
		DIFFUSE = 0x1201, /**< A diffuse material. */
		SPECULAR = 0x1202, /**< A specular material. */
		AMBIENT_AND_DIFFUSE = 0x1602, /**< An ambient and diffuse material. */

		INVALID /**< An invalid material type. */
	};

	/** Converts the COLLADA render state material type string to the material type.
		@param value The render state material type string.
		@return The material type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the material type to its COLLADA render state material type string.
		@param type The material type.
		@return The render state material type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render state fog types. */
namespace FUDaePassStateFogType
{
	/** The fog formula. */
	enum Type
	{
		LINEAR = 0x2601, /**< Linear fog. */
		EXP = 0x0800, /**< Exponential fog. */
		EXP2 = 0x0801, /**< Exponential fog. */

		INVALID /**< An invalid fog type. */
	};

	/** Converts the COLLADA render state fog type string to the fog type.
		@param value The render state fog type string.
		@return The fog type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the fog type to its COLLADA render state fog type string.
		@param type The fog type.
		@return The render state fog type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
}

/** The render state fog coordinate source types. */
namespace FUDaePassStateFogCoordinateType
{
	/** A fog coordinate type. */
	enum Type
	{
		FOG_COORDINATE = 0x8451, /**< Use fog coordinate values. */
		FRAGMENT_DEPTH = 0x8452, /**< Use the depth value of the fragment. */

		INVALID /**< An invalid fog coordinate type. */
	};

	/** Converts the COLLADA render state fog coordinate source type string to the fog coordinate source type.
		@param value The render state fog coordinate source type string.
		@return The fog coordinate source type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the fog coordinate source type to its COLLADA render state fog coordinate source type string.
		@param type The fog coordinate source type.
		@return The render state fog coordinate source type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render state front face types. */
namespace FUDaePassStateFrontFaceType
{
	/** The winding direction that determines the front surface for a polygon. */
	enum Type
	{
		CLOCKWISE = 0x0900, /**< Clockwise winding. */
		COUNTER_CLOCKWISE = 0x0901, /**< Counter-clockwise winding. */

		INVALID /**< An invalid winding. */
	};

	/** Converts the COLLADA render state front-face type string to the front-face type.
		@param value The render state front-face type string.
		@return The front-face type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the front-face type to its COLLADA render state front-face type string.
		@param type The front-face type.
		@return The render state front-face type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render state logic operations. */
namespace FUDaePassStateLogicOperation
{
	/** A render state logical operation.
		The usage of the operation depends on the render state. */
	enum Operation
	{
		CLEAR = 0x1500, /**< dest_value = 0. */
		AND = 0x1501, /**< dest_value = dest_value & source_value. */
		AND_REVERSE = 0x1502, /**< ? */
		COPY = 0x1503, /**< dest_value = source_value. */
		AND_INVERTED = 0x1504, /**< ? */
		NOOP = 0x1505, /**< No operation. */
		XOR = 0x1506, /**< dest_value = dest_value ^ source_value. */
		OR = 0x1507, /**< dest_value = dest_value | source_value. */
		NOR = 0x1508, /**< dest_value = dest_value | ~(source_value). */
		EQUIV = 0x1509, /**< dest_value = dest_value | source_value. */
		INVERT = 0x150A, /**< dest_value = ~(dest_value). */
		OR_REVERSE = 0x150B, /**< ? */
		COPY_INVERTED = 0x150C, /**< ? */
		NAND = 0x150E, /**< dest_value = dest_value & ~(source_value). */
		SET = 0x150F, /**< ? */

		INVALID /**< An invalid logical operation. */
	};

	/** Converts the COLLADA render state logic operation type string to the logic operation.
		@param value The render state logic operation type string.
		@return The logic operation. */
	FCOLLADA_EXPORT Operation FromString(const char* value);
	inline Operation FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the logic operation to its COLLADA render state logic operation type string.
		@param op The logic operation.
		@return The render state logic operation type string. */
	FCOLLADA_EXPORT const char* ToString(Operation op);
};

/** The render state polygon modes. */
namespace FUDaePassStatePolygonMode
{
	/** The rasterizing mode for polygons. */
	enum Mode
	{
		POINT = 0x1B00, /**< Rasterize only the polygon vertices. */
		LINE = 0x1B01, /**< Rasterize only the polygon edges. */
		FILL = 0x1B02, /**< Rasterize the full polygon. */

		INVALID /**< An invalid rasterizing mode. */
	};

	/** Converts the COLLADA render state polygon mode type string to the polygon mode.
		@param value The render state polygon mode type string.
		@return The polygon mode. */
	FCOLLADA_EXPORT Mode FromString(const char* value);
	inline Mode FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the polygon mode to its COLLADA render state polygon mode type string.
		@param mode The polygon mode.
		@return The render state polygon mode type string. */
	FCOLLADA_EXPORT const char* ToString(Mode mode);
};

/** The render state shading model. */
namespace FUDaePassStateShadeModel
{
	/** The shading model using when rasterizing. */
	enum Model
	{
		FLAT = 0x1D00, /** Flat shading. */
		SMOOTH = 0x1D01, /** Gouraud shading. */

		INVALID /**< An invalid shading model. */
	};

	/** Converts the COLLADA render state shading model type string to the shading model.
		@param value The render state shading model type string.
		@return The polygon mode. */
	FCOLLADA_EXPORT Model FromString(const char* value);
	inline Model FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the shading model to its COLLADA render state shading model type string.
		@param model The shading model.
		@return The render state shading model type string. */
	FCOLLADA_EXPORT const char* ToString(Model model);
};

/** The render state light model color control types. */
namespace FUDaePassStateLightModelColorControlType
{
	/** The type of colors used within the light model. */
	enum Type
	{
		SINGLE_COLOR = 0x81F9, /**< Only one color should be used, for both the diffuse and specular lighting. */
		SEPARATE_SPECULAR_COLOR = 0x81FA, /** Two separate colors should be used for diffuse and specular lighting. */

		INVALID /**< An invalid light model color control type. */
	};

	/** Converts the COLLADA render state light model color control type string to the light model color control type.
		@param value The render state light model color control type string.
		@return The light model color control type. */
	FCOLLADA_EXPORT Type FromString(const char* value);
	inline Type FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the light model color control type to its COLLADA render state light model color control type string.
		@param type The light model color control type.
		@return The render state light model color control type string. */
	FCOLLADA_EXPORT const char* ToString(Type type);
};

/** The render states for effect passes. */
namespace FUDaePassState
{
	/** Enumerates the COLLADA render states for effect passes.
		Each state description states the structure allocated in FCollada for this state.
		For example, ALPHA_FUNC describes: { FUDaePassStateFunction function = ALWAYS, float alphaComparison = 0.0f }.
		This implies a 8-byte structure with
		1) FUDaePassStateFunction::Function at offset 0; it defaults to the ALWAYS enumerated type.
		2) The alpha comparison value at offset 4; it defaults to 0.0f.
		All enumerated types are 4 byte in length. */
	enum State
	{
		ALPHA_FUNC = 0, /**< { FUDaePassStateFunction function = ALWAYS, float alphaComparison = 0.0f } */
		BLEND_FUNC, /**< { FUDaePassStateBlendType sourceBlend = ONE, FUDaePassStateBlendType destinationBlend = ZERO } */
		BLEND_FUNC_SEPARATE, /**< { FUDaePassStateBlendType sourceColorBlend = ONE, FUDaePassStateBlendType destinationColorBlend = ZERO, FUDaePassStateBlendType sourceAlphaBlend = ONE, FUDaePassStateBlendType destinationAlphaBlend = ZERO } */
		BLEND_EQUATION, /**< { FUDaePassStateBlendEquation blendEquation = ADD } */
		BLEND_EQUATION_SEPARATE, /**< { FUDaePassStateBlendEquation colorEquation = ADD, FUDaePassStateBlendEquation alphaEquation = ADD } */
		COLOR_MATERIAL, /**< { FUDaePassStateFaceType whichFaces = FRONT_AND_BACK, FUDaePassStateMaterialType material = AMBIENT_AND_DIFFUSE } */
		CULL_FACE, /**< { FUDaePassStateFaceType culledFaces = BACK } */
		DEPTH_FUNC, /**< { FUDaePassStateFunction depthAcceptFunction = ALWAYS } */
		FOG_MODE, /**< { FUDaePassStateFogType fogType = EXP } */
		FOG_COORD_SRC, /**< { FUDaePassStateFogCoordinateType type = FOG_COORDINATE } */

		FRONT_FACE = 10, /**< { FUDaePassStateFrontFaceType frontFaceSide = COUNTER_CLOCKWISE } */
		LIGHT_MODEL_COLOR_CONTROL, /**< { FUDaePassStateLightModelColorControlType controlType = SINGLE_COLOR } */
		LOGIC_OP, /**< { FUDaePassStateLogicOperation operation = COPY } */
		POLYGON_MODE, /**< { FUDaePassStateFaceType whichFaces = FRONT_AND_BACK, FUDaePassStatePolygonMode renderMode = FILL } */
		SHADE_MODEL, /**< { FUDaePassStateShadeModel model = SMOOTH } */
		STENCIL_FUNC, /**< { FUDaePassStateFunction acceptFunction = ALWAYS, uint8 referenceValue = 0, uint8 mask = 0xFF } */
		STENCIL_OP, /**< { FUDaePassStateStencilOperation failOperation = KEEP, FUDaePassStateStencilOperation depthFailOperation = KEEP, FUDaePassStateStencilOperation depthPassOperation = KEEP } */
		STENCIL_FUNC_SEPARATE, /**< { FUDaePassStateFunction frontFacesAcceptFunction = ALWAYS, FUDaePassStateFunction backFacesAcceptFunction = ALWAYS, uint8 referenceValue = 0, uint8 mask = 0xFF } */
		STENCIL_OP_SEPARATE, /**< { FUDaePassStateFaceType whichFaces = FRONT_AND_BACK, FUDaePassStateStencilOperation failOperation = KEEP, FUDaePassStateStencilOperation depthFailOperation = KEEP, FUDaePassStateStencilOperation depthPassOperation = KEEP } */
		STENCIL_MASK_SEPARATE, /**< { FUDaePassStateFaceType whichFaces = FRONT_AND_BACK, uint8 mask = 0xFF } */

		LIGHT_ENABLE = 20, /**< { uint8 lightIndex = 0, bool enabled = false } */
		LIGHT_AMBIENT, /**< { uint8 lightIndex = 0, FMVector4 ambientColor = [0,0,0,1] } */
		LIGHT_DIFFUSE, /**< { uint8 lightIndex = 0, FMVector4 diffuseColor = [0,0,0,0] } */
		LIGHT_SPECULAR, /**< { uint8 lightIndex = 0, FMVector4 specularColor = [0,0,0,0] } */
		LIGHT_POSITION, /**< { uint8 lightIndex = 0, FMVector4 position = [0,0,1,0] } */
		LIGHT_CONSTANT_ATTENUATION, /**< { uint8 lightIndex = 0, float constantAttenuation = 1.0f } */
		LIGHT_LINEAR_ATTENUATION, /**< { uint8 lightIndex = 0, float linearAttenuation = 0.0f } */
		LIGHT_QUADRATIC_ATTENUATION, /**< { uint8 lightIndex = 0, float quadraticAttenuation = 0.0f } */
		LIGHT_SPOT_CUTOFF, /**< { uint8 lightIndex = 0, float cutoff = 180.0f } */
		LIGHT_SPOT_DIRECTION, /**< { uint8 lightIndex = 0, FMVector4 direction = [0,0,-1] } */
		LIGHT_SPOT_EXPONENT = 30, /**< { uint8 lightIndex = 0, float exponent = 0.0f } */

		TEXTURE1D = 31, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */ 
		TEXTURE2D, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */
		TEXTURE3D, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */
		TEXTURECUBE, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */
		TEXTURERECT, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */
		TEXTUREDEPTH, /**< { uint8 textureIndex = 0, uint32 textureId = 0 } */
		TEXTURE1D_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTURE2D_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTURE3D_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTURECUBE_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTURERECT_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTUREDEPTH_ENABLE, /**< { uint8 textureIndex = 0, bool enabled = false } */
		TEXTURE_ENV_COLOR, /**< { uint8 textureIndex = 0, FMVector4 environmentColor = [0,0,0,0] } */
		TEXTURE_ENV_MODE, /**< { uint8 textureIndex = 0, char environmentMode[255] = "" } */

		CLIP_PLANE = 45, /**< { uint8 planeIndex = 0, FMVector4 planeValue = [0,0,0,0] } */
		CLIP_PLANE_ENABLE, /**< { uint8 planeIndex = 0, bool enabled = false } */
		BLEND_COLOR, /**< { FMVector4 blendColor = [0,0,0,0] } */
		CLEAR_COLOR, /**< { FMVector4 clearColor = [0,0,0,0] } */
		CLEAR_STENCIL, /**< { uint32 clearStencilValue = 0 } */
		CLEAR_DEPTH, /**< { float clearDepthValue = 1.0f } */
		COLOR_MASK, /**< { bool redWriteMask = true, bool greenWriteMask = true, bool blueWriteMask = true, bool alphaWriteMask = true } */
		DEPTH_BOUNDS, /**< { FMVector2 depthBounds = [0,1] } */
		DEPTH_MASK, /**< { bool depthWriteMask = true } */
		DEPTH_RANGE, /**< { float minimumDepthValue = 0.0f, float maximumDepthValue = 1.0f } */

		FOG_DENSITY = 55, /**< { float fogDensity = 1.0f } */
		FOG_START, /**< { float fogStartDepthValue = 0.0f } */
		FOG_END, /**< { float fogEndDepthValue = 1.0f } */
		FOG_COLOR, /**< { FMVector4 fogColor = [0,0,0,0] } */
		LIGHT_MODEL_AMBIENT, /**< { FMVector4 ambientColor = [0.2,0.2,0.2,1] } */
		LIGHTING_ENABLE, /**< { bool enabled = false } */
		LINE_STIPPLE, /**< { uint16 lineStippleStart = 1, uint16 lineStippleEnd = 0xFF } */
		LINE_WIDTH, /**< { float lineWidth = 1.0f } */

		MATERIAL_AMBIENT = 63, /**< { FMVector4 ambientColor = [0.2,0.2,0.2,1] } */
		MATERIAL_DIFFUSE, /**< { FMVector4 diffuseColor = [0.8,0.8,0.8,1] } */
		MATERIAL_EMISSION, /**< { FMVector4 emissionColor = [0,0,0,1] } */
		MATERIAL_SHININESS, /**< { float shininessOrSpecularLevel = 0.0f } */
		MATERIAL_SPECULAR, /**< { FMVector4 specularColor = [0,0,0,1] } */
		MODEL_VIEW_MATRIX, /**< { FMMatrix44 modelViewMatrix = FMMatrix44::Identity } */
		POINT_DISTANCE_ATTENUATION, /**< { FMVector3 attenuation = [1,0,0] } */
		POINT_FADE_THRESHOLD_SIZE, /**< { float threshold = 1.0f } */
		POINT_SIZE, /**< { float size = 1.0f } */
		POINT_SIZE_MIN, /**< { float minimum = 0.0f } */
		POINT_SIZE_MAX, /**< { float maximum = 1.0f } */

		POLYGON_OFFSET = 74, /**< { float factor = 0.0f, float units = 0.0f } */
		PROJECTION_MATRIX, /**< { FMMatrix44 projectionMatrix = FMMatrix44::Identity } */
		SCISSOR, /**< { FMVector4 scissor = [0,0,0,0] } */
		STENCIL_MASK, /**< { uint32 mask = 0xFFFFFFFF } */
		ALPHA_TEST_ENABLE, /**< { bool enabled = false } */
		AUTO_NORMAL_ENABLE, /**< { bool enabled  = false } */
		BLEND_ENABLE, /**< { bool enabled  = false } */
		COLOR_LOGIC_OP_ENABLE, /**< { bool enabled  = false } */
		COLOR_MATERIAL_ENABLE, /**< { bool enabled  = true } */
		CULL_FACE_ENABLE, /**< { bool enabled  = false } */

		DEPTH_BOUNDS_ENABLE = 84, /**< { bool enabled  = false } */
		DEPTH_CLAMP_ENABLE, /**< { bool enabled  = false } */
		DEPTH_TEST_ENABLE, /**< { bool enabled  = false } */
		DITHER_ENABLE, /**< { bool enabled  = false } */
		FOG_ENABLE, /**< { bool enabled  = false } */
		LIGHT_MODEL_LOCAL_VIEWER_ENABLE, /**< { bool enabled  = false } */
		LIGHT_MODEL_TWO_SIDE_ENABLE, /**< { bool enabled  = false } */
		LINE_SMOOTH_ENABLE, /**< { bool enabled  = false } */
		LINE_STIPPLE_ENABLE, /**< { bool enabled = false } */
		LOGIC_OP_ENABLE, /**< { bool enabled  = false } */
		MULTISAMPLE_ENABLE, /**< { bool enabled  = false } */
		
		NORMALIZE_ENABLE = 95, /**< { bool enabled  = false } */
		POINT_SMOOTH_ENABLE, /**< { bool enabled  = false } */
		POLYGON_OFFSET_FILL_ENABLE, /**< { bool enabled  = false } */
		POLYGON_OFFSET_LINE_ENABLE, /**< { bool enabled  = false } */
		POLYGON_OFFSET_POINT_ENABLE, /**< { bool enabled  = false } */
		POLYGON_SMOOTH_ENABLE, /**< { bool enabled  = false } */
		POLYGON_STIPPLE_ENABLE, /**< { bool enabled  = false } */
		RESCALE_NORMAL_ENABLE, /**< { bool enabled  = false } */

		SAMPLE_ALPHA_TO_COVERAGE_ENABLE = 103, /**< { bool enabled  = false } */
		SAMPLE_ALPHA_TO_ONE_ENABLE, /**< { bool enabled  = false } */
		SAMPLE_COVERAGE_ENABLE, /**< { bool enabled  = false } */
		SCISSOR_TEST_ENABLE, /**< { bool enabled  = false } */
		STENCIL_TEST_ENABLE, /**< { bool enabled  = false } */

		COUNT = 108, /**< The number of supported render states. */
		INVALID /**< An invalid render state. */
	};

	/** Converts the COLLADA render state name string to the render state enumeration type.
		@param value The render state name string.
		@return The render state enumeration type. */
	FCOLLADA_EXPORT State FromString(const char* value);
	inline State FromString(const fm::string& value) { return FromString(value.c_str()); } /**< See above. */

	/** Converts the render state enumeration type to its COLLADA render state name string.
		@param state The render state enumeration type.
		@return The render state name string. */
	FCOLLADA_EXPORT const char* ToString(State state);
};

/** Character-based definition of the various accessors TODO */
namespace FUDaeAccessor
{
	/** Common accessor type string arrays.
		These are NULL-terminated and can be used with the AddAccessor function. */
	extern const char* XY[3]; /**< Use for tangents and other 2D values. */
	extern const char* XYZW[5]; /**< Use for vector and position sources. */
	extern const char* RGBA[5]; /**< Use for color sources. */
	extern const char* STPQ[5]; /**< Use for texture coordinate sources. */
}

#endif // _FU_DAE_ENUM_H_

