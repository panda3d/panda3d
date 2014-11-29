/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREELEMENTINSTANCERGENERIC_H
#define ROCKETCOREELEMENTINSTANCERGENERIC_H

#include <Rocket/Core/ElementInstancer.h>

namespace Rocket {
namespace Core {

/**
	Generic Instancer that creates a plain old Element

	This instancer is used for most elements and is by default
	registered as the "*" fallback handler.

	@author Lloyd Weehuizen
 */

template <typename T>
class ElementInstancerGeneric : public ElementInstancer
{
public:
	virtual ~ElementInstancerGeneric();
	
	/// Instances an element given the tag name and attributes
	/// @param tag Name of the element to instance
	/// @param attributes vector of name value pairs
	virtual Element* InstanceElement(Element* parent, const String& tag, const XMLAttributes& attributes);

	/// Releases the given element
	/// @param element to release
	virtual void ReleaseElement(Element* element);

	/// Release the instancer
	virtual void Release();
};

#include <Rocket/Core/ElementInstancerGeneric.inl>

}
}

#endif
