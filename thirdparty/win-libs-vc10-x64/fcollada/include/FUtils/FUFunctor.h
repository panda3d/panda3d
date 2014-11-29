/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	This file was taken off the Protect project on 26-09-2005
*/

#ifndef _FU_FUNCTOR_H_
#define _FU_FUNCTOR_H_

/**
	A functor with no arguments.
	@ingroup FUtils
*/
template<class ReturnType>
class IFunctor0
{
public:
	/** Destructor. */
	virtual ~IFunctor0() {}

	/** Calls the functor.
		@return Implementation-dependant. */
	virtual ReturnType operator()() const = 0;

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const = 0;

	/** Returns a copy of this functor */
	virtual IFunctor0<ReturnType>* Copy() const = 0;
};

/**
	A functor with no arguments.
	@ingroup FUtils
*/
template<class Class, class ReturnType>
class FUFunctor0 : public IFunctor0<ReturnType>
{
private:
	Class* m_pObject;
	ReturnType (Class::*m_pFunction) ();

public:
	/** Constructor.
		@param object An object.
		@param function A member function of this object. */
	FUFunctor0(Class* object, ReturnType (Class::*function) ()) { m_pObject = object; m_pFunction = function; }

	/** Destructor. */
	virtual ~FUFunctor0() {}

	/** Calls the functor.
		@return Implementation-dependant. */
	virtual ReturnType operator()() const
	{ return ((*m_pObject).*m_pFunction)(); }

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const
	{ return object == m_pObject && (size_t)function == *(size_t*)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor0<ReturnType>* Copy() const 
	{ return new FUFunctor0<Class, ReturnType>(m_pObject, m_pFunction); }
};

/** Shortcut for new FUFunctor0<>.
	@param o An object.
	@param function A member function. */
template <class ClassName, class ReturnType>
inline FUFunctor0<ClassName, ReturnType>* NewFUFunctor0(ClassName* o, ReturnType (ClassName::*function) ())
{
	return new FUFunctor0<ClassName, ReturnType>(o, function);
}

/** 
	A pseudo-functor with no arguments, specialized for static functions.
	@ingroup FUtils
*/
template<class ReturnType>
class FUStaticFunctor0 : public IFunctor0<ReturnType>
{
private:
	ReturnType (*m_pFunction) ();

public:
	/** Constructor.
		@param function A static function. */
	FUStaticFunctor0(ReturnType (*function) ()) { m_pFunction = function; }

	/** Destructor. */
	virtual ~FUStaticFunctor0() {}

	/** Calls the functor.
		@return Implementation-dependant. */
	virtual ReturnType operator()() const
	{ return (*m_pFunction)(); }

	/** Checks whether this pseudo-functor points towards the given function.
		@param UNUSED Unused.
		@param function The static function.
		@return Whether this pseudo-functor points towards the given function. */
	virtual bool Compare(void* UNUSED(object), void* function) const
	{ return (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor0<ReturnType>* Copy() const 
	{ return new FUStaticFunctor0<ReturnType>(m_pFunction); }
};

/** 
	A functor with one argument.
	@ingroup FUtils
*/
template<class Arg1, class ReturnType>
class IFunctor1
{
public:
	/** Destructor. */
	virtual ~IFunctor1() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1) const = 0;

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const = 0;

	/** Returns a copy of this functor */
	virtual IFunctor1<Arg1, ReturnType>* Copy() const = 0;
};

/** 
	A functor with one arguments.
	@ingroup FUtils
*/
template<class Class, class Arg1, class ReturnType>
class FUFunctor1 : public IFunctor1<Arg1, ReturnType>
{
private:
	Class* m_pObject;
	ReturnType (Class::*m_pFunction) (Arg1);

public:
	/** Constructor.
		@param object An object.
		@param function A member function of this object. */
	FUFunctor1(Class* object, ReturnType (Class::*function) (Arg1)) { m_pObject = object; m_pFunction = function; }

	/** Destructor. */
	virtual ~FUFunctor1() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1) const
	{ return ((*m_pObject).*m_pFunction)(argument1); }

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const
	{ return object == m_pObject && (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor1<Arg1, ReturnType>* Copy() const 
	{ return new FUFunctor1<Class, Arg1, ReturnType>(m_pObject, m_pFunction); }
};

/** Shortcut for new FUFunctor1<>.
	@param o An object.
	@param function A member function. */
template <class ClassName, class Argument1, class ReturnType>
inline FUFunctor1<ClassName, Argument1, ReturnType>* NewFUFunctor1(ClassName* o, ReturnType (ClassName::*function) (Argument1))
{
	return new FUFunctor1<ClassName, Argument1, ReturnType>(o, function);
}

/** 
	A pseudo-functor with one arguments, specialized for static functions.
	@ingroup FUtils
*/
template<class Arg1, class ReturnType>
class FUStaticFunctor1 : public IFunctor1<Arg1, ReturnType>
{
private:
	ReturnType (*m_pFunction) (Arg1);

public:
	/** Constructor.
		@param function A static function. */
	FUStaticFunctor1(ReturnType (*function) (Arg1)) { m_pFunction = function; }

	/** Destructor. */
	virtual ~FUStaticFunctor1() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1) const
	{ return (*m_pFunction)(argument1); }

	/** Checks whether this pseudo-functor points towards the given function.
		@param UNUSED Unused.
		@param function The static function.
		@return Whether this pseudo-functor points towards the given function. */
	virtual bool Compare(void* UNUSED(object), void* function) const
	{ return (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor1<Arg1, ReturnType>* Copy() const 
	{ return new FUStaticFunctor1<Arg1, ReturnType>(m_pFunction); }
};

/** 
	A functor with two arguments.
	@ingroup FUtils
*/
template<class Arg1, class Arg2, class ReturnType>
class IFunctor2
{
public:
	/** Destructor. */
	virtual ~IFunctor2() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2) const = 0;

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const = 0;

	/** Returns a copy of this functor */
	virtual IFunctor2<Arg1, Arg2, ReturnType>* Copy() const = 0;
};

/** 
	A functor with two arguments.
	@ingroup FUtils
*/
template<class Class, class Arg1, class Arg2, class ReturnType>
class FUFunctor2 : public IFunctor2<Arg1, Arg2, ReturnType>
{
private:
	Class* m_pObject;
	ReturnType (Class::*m_pFunction) (Arg1, Arg2);

public:
	/** Constructor.
		@param object An object.
		@param function A member function of this object. */
	FUFunctor2(Class* object, ReturnType (Class::*function) (Arg1, Arg2)) { m_pObject = object; m_pFunction = function; }

	/** Destructor. */
	virtual ~FUFunctor2() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2) const
	{ return ((*m_pObject).*m_pFunction)(argument1, argument2); }

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const
	{ return object == m_pObject && (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor2<Arg1, Arg2, ReturnType>* Copy() const 
	{ return new FUFunctor2<Class, Arg1, Arg2, ReturnType>(m_pObject, m_pFunction); }
};

/** Shortcut for new FUFunctor2<>.
	@param o An object.
	@param function A member function. */
template <class ClassName, class Argument1, class Argument2, class ReturnType>
inline FUFunctor2<ClassName, Argument1, Argument2, ReturnType>* NewFUFunctor2(ClassName* o, ReturnType (ClassName::*function) (Argument1, Argument2))
{
	return new FUFunctor2<ClassName, Argument1, Argument2, ReturnType>(o, function);
}

/** 
	A pseudo-functor with two arguments, specialized for static functions.
	@ingroup FUtils
*/
template<class Arg1, class Arg2, class ReturnType>
class FUStaticFunctor2 : public IFunctor2<Arg1, Arg2, ReturnType>
{
private:
	ReturnType (*m_pFunction) (Arg1, Arg2);

public:
	/** Constructor.
		@param function A static function. */
	FUStaticFunctor2(ReturnType (*function) (Arg1, Arg2)) { m_pFunction = function; }

	/** Destructor. */
	virtual ~FUStaticFunctor2() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2) const
	{ return (*m_pFunction)(argument1, argument2); }

	/** Checks whether this pseudo-functor points towards the given function.
		@param UNUSED Unused.
		@param function The static function.
		@return Whether this pseudo-functor points towards the given function. */
	virtual bool Compare(void* UNUSED(object), void* function) const
	{ return (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor2<Arg1, Arg2, ReturnType>* Copy() const 
	{ return new FUStaticFunctor2<Arg1, Arg2, ReturnType>(m_pFunction); }
};

/** 
	A functor with three arguments.
	@ingroup FUtils
*/
template<class Arg1, class Arg2, class Arg3, class ReturnType>
class IFunctor3
{
public:
	/** Destructor. */
	virtual ~IFunctor3() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@param argument3 A third argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2, Arg3 argument3) const = 0;

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const = 0;

	/** Returns a copy of this functor */
	virtual IFunctor3<Arg1, Arg2, Arg3, ReturnType>* Copy() const = 0;
};

/** 
	A functor with three arguments.
	@ingroup FUtils
*/
template<class Class, class Arg1, class Arg2, class Arg3, class ReturnType>
class FUFunctor3 : public IFunctor3<Arg1, Arg2, Arg3, ReturnType>
{
private:
	Class* m_pObject;
	ReturnType (Class::*m_pFunction) (Arg1, Arg2, Arg3);

public:
	/** Constructor.
		@param object An object.
		@param function A member function of this object. */
	FUFunctor3(Class* object, ReturnType (Class::*function) (Arg1, Arg2, Arg3)) { m_pObject = object; m_pFunction = function; }

	/** Destructor. */
	virtual ~FUFunctor3() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@param argument3 A third argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2, Arg3 argument3) const
	{ return ((*m_pObject).*m_pFunction)(argument1, argument2, argument3); }

	/** Checks whether this functor points towards the given member function.
		@param object The object which holds the member function.
		@param function The member function.
		@return Whether this functor points towards the given member function. */
	virtual bool Compare(void* object, void* function) const
	{ return object == m_pObject && (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor3<Arg1, Arg2, Arg3, ReturnType>* Copy() const
	{ return new FUFunctor3<Class, Arg1, Arg2, Arg3, ReturnType>(m_pObject, m_pFunction); };
};

/** Shortcut for new FUFunctor2<>.
	@param o An object.
	@param function A member function. */
template <class ClassName, class Argument1, class Argument2, class Argument3, class ReturnType>
inline FUFunctor3<ClassName, Argument1, Argument2, Argument3, ReturnType>* NewFUFunctor3(ClassName* o, ReturnType (ClassName::*function) (Argument1, Argument2, Argument3))
{
	return new FUFunctor3<ClassName, Argument1, Argument2, Argument3, ReturnType>(o, function);
}

/** 
	A pseudo-functor with three arguments, specialized for static functions.
	@ingroup FUtils
*/
template<class Arg1, class Arg2, class Arg3, class ReturnType>
class FUStaticFunctor3 : public IFunctor3<Arg1, Arg2, Arg3, ReturnType>
{
private:
	ReturnType (*m_pFunction) (Arg1, Arg2, Arg3);

public:
	/** Constructor.
		@param function A static function. */
	FUStaticFunctor3(ReturnType (*function) (Arg1, Arg2, Arg3)) { m_pFunction = function; }

	/** Destructor. */
	virtual ~FUStaticFunctor3() {}

	/** Calls the functor.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@param argument3 A third argument.
		@return Implementation-dependant. */
	virtual ReturnType operator()(Arg1 argument1, Arg2 argument2, Arg3 argument3) const
	{ return (*m_pFunction)(argument1, argument2, argument3); }

	/** Checks whether this pseudo-functor points towards the given function.
		@param UNUSED Unused.
		@param function The static function.
		@return Whether this pseudo-functor points towards the given function. */
	virtual bool Compare(void* UNUSED(object), void* function) const
	{ return (size_t)function == *(size_t*)(size_t)&m_pFunction; }

	/** Returns a copy of this functor */
	virtual IFunctor3<Arg1, Arg2, Arg3, ReturnType>* Copy() const
	{ return new FUStaticFunctor3<Arg1, Arg2, Arg3, ReturnType>(m_pFunction); };
};

#endif //_FU_FUNCTOR_H_

