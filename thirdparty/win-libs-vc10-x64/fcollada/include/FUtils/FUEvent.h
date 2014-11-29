/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	This file was taken off the Protect project on 26-09-2005
*/

/**
	@file FUEvent.h
	This file contains templates to contain and trigger callback events.
*/


#ifndef _FU_EVENT_H_
#define _FU_EVENT_H_

#ifndef _FU_FUNCTOR_H_
#include "FUtils/FUFunctor.h"
#endif // _FU_FUNCTOR_H_

/**
	An event with no argument.
	@ingroup FUtils
*/
class FUEvent0
{
private:
	typedef IFunctor0<void> Handler;
	typedef fm::pvector<Handler> HandlerList;
	HandlerList handlers;

public:
	/** Constructor. */
	FUEvent0() {}

	/** Destructor. */
	~FUEvent0()
	{
		FUAssert(handlers.empty(), CLEAR_POINTER_VECTOR(handlers));
	}

	/** Retrieves the number of callbacks registered for this event.
		@return The number of callbacks registered. */
	size_t GetHandlerCount() { return handlers.size(); }

	/** Adds a new callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function to callback. */
	template <class Class>
	void InsertHandler(Class* handle, void (Class::*_function)())
	{
		handlers.push_back(new FUFunctor0<Class, void>(handle, _function));
	}

	/** Adds a functor that handles the event.
		Note that the event will own the memory for the functor.
		@param functor The functor that handlers the event. */
	void InsertHandler(Handler* functor)
	{
		handlers.push_back(functor);
	}		

	/** Adds a new callback that handles the event.
		@param _function The static function to callback. */
	void InsertHandler(void (*_function)())
	{
		handlers.push_back(new FUStaticFunctor0<void>(_function));
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The handle of the function container.
			This pointer will be NULL for static functions.
		@param function The address of the function callback to unregister. */
	void ReleaseHandler(void* handle, void* function)
	{
		HandlerList::iterator it;
		for (it = handlers.begin(); it != handlers.end(); ++it)
		{
			if ((*it)->Compare(handle, function))
			{
				delete (*it);
				handlers.erase(it);
				break;
			}
		}
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function callback to unregister. */
	template <class Class>
	void ReleaseHandler(Class* handle, void (Class::*_function)())
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler((void*) handle, function);
	}

	/** Releases and unregisters a callback that handles the event.
		@param _function The static function callback to unregister. */
	void ReleaseHandler(void (*_function)())
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler(NULL, function);
	}

	/** Triggers the event.
		All the registered callbacks will be called, in reverse-order
		of their registration. */
	void operator()()
	{
		intptr_t index = handlers.size() - 1; 
		for (; index >= 0; --index) 
		{ 
			(*handlers[index])();
		}
	}
};

/**
	An event with one argument.
	@ingroup FUtils
*/
template <class Arg1>
class FUEvent1
{
private:
	typedef IFunctor1<Arg1, void> Handler;
	typedef fm::pvector<Handler> HandlerList;
	HandlerList handlers;

public:
	/** Constructor. */
	FUEvent1() {}

	/** Destructor. */
	~FUEvent1()
	{
		FUAssert(handlers.empty(), CLEAR_POINTER_VECTOR(handlers));
	}

	/** Retrieves the number of callbacks registered for this event.
		@return The number of callbacks registered. */
	size_t GetHandlerCount() { return handlers.size(); }

	/** Adds a new callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function to callback. */
	template <class Class>
	void InsertHandler(Class* handle, void (Class::*_function)(Arg1))
	{
		handlers.push_back(NewFUFunctor1(handle, _function));
	}

	/** Adds a functor that handles the event.
		Note that the event will own the memory for the functor.
		@param functor The functor that handlers the event. */
	void InsertHandler(Handler* functor)
	{
		handlers.push_back(functor);
	}		

	/** Adds a new callback that handles the event.
		@param _function The static function to callback. */
	void InsertHandler(void (*_function)(Arg1))
	{
		handlers.push_back(new FUStaticFunctor1<Arg1, void>(_function));
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The handle of the function container.
			This pointer will be NULL for static functions.
		@param function The address of the function callback to unregister. */
	void ReleaseHandler(void* handle, void* function)
	{
		typename HandlerList::iterator it;
		for (it = handlers.begin(); it != handlers.end(); ++it)
		{
			if ((*it)->Compare(handle, function))
			{
				delete (*it);
				handlers.erase(it);
				break;
			}
		}
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function callback to unregister. */
	template <class Class>
	void ReleaseHandler(Class* handle, void (Class::*_function)(Arg1))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler((void*) handle, function);
	}

	/** Releases and unregisters a callback that handles the event.
		@param _function The static function callback to unregister. */
	void ReleaseHandler(void (*_function)(Arg1))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler(NULL, function);
	}

	/** Triggers the event.
		All the registered callbacks will be called, in reverse-order
		of their registration.
		@param argument1 A first argument. */
	void operator()(Arg1 argument1)
	{
		intptr_t index = handlers.size() - 1; 
		for (; index >= 0; --index) 
		{ 
			(*handlers[index])(argument1);
		} 
	}
};

/**
	An event with two argument.
	@ingroup FUtils
*/
template <class Arg1, class Arg2>
class FUEvent2
{
private:
	typedef IFunctor2<Arg1, Arg2, void> Handler;
	typedef fm::pvector<Handler> HandlerList;
	HandlerList handlers;

public:
	/** Constructor. */
	FUEvent2() {}

	/** Destructor. */
	~FUEvent2()
	{
		FUAssert(handlers.empty(), CLEAR_POINTER_VECTOR(handlers));
	}

	/** Retrieves the number of callbacks registered for this event.
		@return The number of callbacks registered. */
	size_t GetHandlerCount() { return handlers.size(); }

	/** Adds a new callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function to callback. */
	template <class Class>
	void InsertHandler(Class* handle, void (Class::*_function)(Arg1, Arg2))
	{
		handlers.push_back(new FUFunctor2<Class, Arg1, Arg2, void>(handle, _function));
	}

	/** Adds a functor that handles the event.
		Note that the event will own the memory for the functor.
		@param functor The functor that handlers the event. */
	void InsertHandler(Handler* functor)
	{
		handlers.push_back(functor);
	}		

	/** Adds a new callback that handles the event.
		@param _function The static function to callback. */
	void InsertHandler(void (*_function)(Arg1, Arg2))
	{
		handlers.push_back(new FUStaticFunctor2<Arg1, Arg2, void>(_function));
	}
	
	/** Releases and unregisters a callback that handles the event.
		@param handle The handle of the function container.
			This pointer will be NULL for static functions.
		@param function The address of the function callback to unregister. */
	void ReleaseHandler(void* handle, void* function)
	{
		typename HandlerList::iterator it;
		for (it = handlers.begin(); it != handlers.end(); ++it)
		{
			if ((*it)->Compare(handle, function))
			{
				delete (*it);
				handlers.erase(it);
				break;
			}
		}
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function callback to unregister. */
	template <class Class>
	void ReleaseHandler(Class* handle, void (Class::*_function)(Arg1, Arg2))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler((void*) handle, function);
	}

	/** Releases and unregisters a callback that handles the event.
		@param _function The static function callback to unregister. */
	void ReleaseHandler(void (*_function)(Arg1, Arg2))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler(NULL, function);
	}

	/** Triggers the event.
		All the registered callbacks will be called, in reverse-order
		of their registration.
		@param argument1 A first argument.
		@param argument2 A second argument. */
	void operator()(Arg1 argument1, Arg2 argument2)
	{
		intptr_t index = handlers.size() - 1; 
		for (; index >= 0; --index) 
		{ 
			(*handlers[index])(argument1, argument2);
		}
	}
};


/**
	An event with three argument.
	@ingroup FUtils
*/
template <class Arg1, class Arg2, class Arg3>
class FUEvent3
{
private:
	typedef IFunctor3<Arg1, Arg2, Arg3, void> Handler;
	typedef fm::pvector<Handler> HandlerList;
	HandlerList handlers;

public:
	/** Constructor. */
	FUEvent3() {}

	/** Destructor. */
	~FUEvent3()
	{
		FUAssert(handlers.empty(), CLEAR_POINTER_VECTOR(handlers));
	}

	/** Retrieves the number of callbacks registered for this event.
		@return The number of callbacks registered. */
	size_t GetHandlerCount() { return handlers.size(); }

	/** Adds a new callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function to callback. */
	template <class Class>
	void InsertHandler(Class* handle, void (Class::*_function)(Arg1, Arg2, Arg3))
	{
		handlers.push_back(new FUFunctor3<Class, Arg1, Arg2, Arg3, void>(handle, _function));
	}

	/** Adds a functor that handles the event.
		Note that the event will own the memory for the functor.
		@param functor The functor that handlers the event. */
	void InsertHandler(Handler* functor)
	{
		handlers.push_back(functor);
	}		

	/** Adds a new callback that handles the event.
		@param _function The static function to callback. */
	void InsertHandler(void (*_function)(Arg1, Arg2, Arg3))
	{
		handlers.push_back(new FUStaticFunctor3<Arg1, Arg2, Arg3, void>(_function));
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The handle of the function container.
			This pointer will be NULL for static functions.
		@param function The address of the function callback to unregister. */
	void ReleaseHandler(void* handle, void* function)
	{
		typename HandlerList::iterator it;
		for (it = handlers.begin(); it != handlers.end(); ++it)
		{
			if ((*it)->Compare(handle, function))
			{
				delete (*it);
				handlers.erase(it);
				break;
			}
		}
	}

	/** Releases and unregisters a callback that handles the event.
		@param handle The object that contains the member function.
		@param _function The member function callback to unregister. */
	template <class Class>
	void ReleaseHandler(Class* handle, void (Class::*_function)(Arg1, Arg2, Arg3))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler((void*) handle, function);
	}

	/** Releases and unregisters a callback that handles the event.
		@param _function The static function callback to unregister. */
	void ReleaseHandler(void (*_function)(Arg1, Arg2, Arg3))
	{
		void* function = *(void**)(size_t)&_function;
		ReleaseHandler(NULL, function);
	}

	/** Triggers the event.
		All the registered callbacks will be called, in reverse-order
		of their registration.
		@param argument1 A first argument.
		@param argument2 A second argument.
		@param argument3 A third argument. */
	void operator()(Arg1 argument1, Arg2 argument2, Arg3 argument3)
	{
		intptr_t index = handlers.size() - 1; 
		for (; index >= 0; --index) 
		{ 
			(*handlers[index])(argument1, argument2, argument3);
		}
	}
};

#endif // _FU_EVENT_H_
