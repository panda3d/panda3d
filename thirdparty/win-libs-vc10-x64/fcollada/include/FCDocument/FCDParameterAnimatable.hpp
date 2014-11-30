/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_ANIMATED_H_
#include "FCDocument/FCDAnimated.h"
#endif // _FCD_ANIMATED_H_

//
// FCDParameterAnimatableT
//

template <class TYPE, int QUALIFIERS>
FCDParameterAnimatableT<TYPE, QUALIFIERS>::FCDParameterAnimatableT(FUParameterizable* _parent)
:	FCDParameterAnimatable(_parent)
{
}

template <class TYPE, int QUALIFIERS>
FCDParameterAnimatableT<TYPE, QUALIFIERS>::FCDParameterAnimatableT(FUParameterizable* _parent, const TYPE& defaultValue)
:	FCDParameterAnimatable(_parent)
,	value(defaultValue)
{
}

template <class TYPE, int QUALIFIERS>
FCDParameterAnimatableT<TYPE, QUALIFIERS>::~FCDParameterAnimatableT()
{
}

template <class TYPE, int QUALIFIERS>
FCDParameterAnimatableT<TYPE, QUALIFIERS>& FCDParameterAnimatableT<TYPE, QUALIFIERS>::operator= (const TYPE& copy)
{
	value = copy;
	GetParent()->SetValueChange();
	return *this;
}

//
// FCDParameterListAnimatableT
//

template <class TYPE, int QUALIFIERS>
FCDParameterListAnimatableT<TYPE, QUALIFIERS>::FCDParameterListAnimatableT(FUParameterizable* parent)
:	FCDParameterListAnimatable(parent)
{
}

template <class TYPE, int QUALIFIERS>
FCDParameterListAnimatableT<TYPE, QUALIFIERS>::~FCDParameterListAnimatableT()
{
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::set(size_t index, const TYPE& value)
{
	values.at(index) = value;
	GetParent()->SetValueChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::insert(size_t index, const TYPE& value)
{
	values.insert(values.begin() + index, value);
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnInsertion(index, 1);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::insert(size_t index, const TYPE* _values, size_t count)
{
	values.insert(values.begin() + index, _values, count);
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnInsertion(index, count);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::insert(size_t index, size_t count, const TYPE& value)
{
	values.insert(values.begin() + index, count, value);
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnInsertion(index, count);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::erase(size_t index)
{
	values.erase(index);
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnRemoval(index, 1);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::erase(const TYPE& value)
{
	size_t index = find(value);
	if (index < values.size()) erase(index);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::erase(size_t start, size_t end)
{
	values.erase(values.begin() + start, values.begin() + end);
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnRemoval(start, end - start);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::clear()
{
	OnRemoval(0, values.size());
	values.clear();
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::push_back(const TYPE& value)
{
	OnInsertion(values.size(), 1);
	values.push_back(value); 
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::push_front(const TYPE& value) 
{
	values.push_front(value); 
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnInsertion(0, 1);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::pop_back()
{
	OnRemoval(size() - 1, 1);
	values.pop_back();
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::pop_front() 
{
	values.pop_front(); 
	GetParent()->SetStructureChangedFlag();
	GetParent()->SetDirtyFlag(); 
	OnRemoval(0, 1);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::resize(size_t count)
{
	if (count > values.size()) OnInsertion(values.size(), count - values.size());
	else if (count < values.size()) OnRemoval(count, values.size() - count);
	values.resize(count);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::resize(size_t count, const TYPE& value)
{
	if (count > values.size()) OnInsertion(values.size(), count - values.size());
	else if (count < values.size()) OnRemoval(count - values.size(), values.size());
	values.resize(count, value);
	OnPotentialSizeChange();
}

template <class TYPE, int QUALIFIERS>
void FCDParameterListAnimatableT<TYPE, QUALIFIERS>::OnPotentialSizeChange()
{
	size_t animatedCount = animateds.size();
	if (animatedCount == 0) return;

	// Check if the first FCDAnimated points to the correct values.
	// If it does, then they all should be fine.
	FCDAnimated* animated = animateds.front();
	FUAssert((size_t) animated->GetArrayElement() < values.size(), return);
	if (animated->GetValue(0) == (void*) &values[animated->GetArrayElement()]) return;

	// Process all the animateds and set their value pointers.
	// IMPORTANT: it is assumed that these values are FLOATS and ORDERED.
	size_t stride = animated->GetValueCount();
	for (size_t i = 0; i < animatedCount; ++i)
	{
		animated = animateds[i];
		size_t arrayElement = animated->GetArrayElement();
		FUAssert(arrayElement < values.size(), return);
		for (size_t j = 0; j < stride; ++j)
		{
			animated->SetValue(j, (float*) (j * sizeof(float) + ((char*) &values[arrayElement])));
		}
	}
}

