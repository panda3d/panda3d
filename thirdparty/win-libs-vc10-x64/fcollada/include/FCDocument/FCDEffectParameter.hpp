/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

//
// FCDEffectParameterT
//

template <class PrimitiveType>
FCDEffectParameterT<PrimitiveType>::~FCDEffectParameterT()
{
}

template <class PrimitiveType>
bool FCDEffectParameterT<PrimitiveType>::IsValueEqual(FCDEffectParameter* parameter)
{
	if (!FCDEffectParameter::IsValueEqual(parameter)) return false;
	FCDEffectParameterT<PrimitiveType>* param = (FCDEffectParameterT<PrimitiveType>*) parameter;
	return IsEquivalent(value, param->GetValue());
}

template <class PrimitiveType>
FCDEffectParameter* FCDEffectParameterT<PrimitiveType>::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterT<PrimitiveType>* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterT<PrimitiveType>(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->HasType(FCDEffectParameterT<PrimitiveType>::GetClassType())) clone = (FCDEffectParameterT<PrimitiveType>*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL) clone->value = *value;
	return _clone;
}

template <class PrimitiveType>
void FCDEffectParameterT<PrimitiveType>::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == GetType())
	{
		FCDEffectParameterT<PrimitiveType>* s = (FCDEffectParameterT<PrimitiveType>*) target;
		s->value = value;
		SetDirtyFlag();
	}
}

//
// FCDEffectParameterAnimatableT
//

template <class PrimitiveType, int Qualifiers>
FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>::~FCDEffectParameterAnimatableT()
{
}

template <class PrimitiveType, int Qualifiers>
bool FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>::IsValueEqual(FCDEffectParameter* parameter)
{
	if (!FCDEffectParameter::IsValueEqual(parameter)) return false;
	FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>* param = (FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>*) parameter;
	
	if (floatType != param->GetFloatType()) return false;
	return IsEquivalent(value, param->GetValue());
}

template <class PrimitiveType, int Qualifiers>
FCDEffectParameter* FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>::Clone(FCDEffectParameter* _clone) const
{
	FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>* clone = NULL;
	if (_clone == NULL) _clone = clone = new FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>(const_cast<FCDocument*>(GetDocument()));
	else if (_clone->HasType(FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>::GetClassType())) clone = (FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>*) _clone;

	if (_clone != NULL) FCDEffectParameter::Clone(_clone);
	if (clone != NULL)
	{
		clone->floatType = floatType;
		clone->value = *value;
	}
	return _clone;
}

template <class PrimitiveType, int Qualifiers>
void FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>::Overwrite(FCDEffectParameter* target)
{
	if (target->GetType() == GetType())
	{
		FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>* s = (FCDEffectParameterAnimatableT<PrimitiveType, Qualifiers>*) target;
		s->value = *value;
		SetDirtyFlag();
	}
}
