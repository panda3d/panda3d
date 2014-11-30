/** @file
	@brief Header

	@date 2011

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

//          Copyright Iowa State University 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef INCLUDED_vrpn_MainloopObject_h_GUID_38f638e4_40e0_4c6d_bebc_21c463794b88
#define INCLUDED_vrpn_MainloopObject_h_GUID_38f638e4_40e0_4c6d_bebc_21c463794b88

// Internal Includes
#include "vrpn_Connection.h"

// Library/third-party includes
// - none

// Standard includes
#include <stdexcept>

#ifdef VRPN_MAINLOOPOBJECT_VERBOSE
#	include <iostream>
#	define VRPN_MAINLOOPOBJECT_MSG(_x) std::cout << __FILE__ << ":" << __LINE__ << ": " << _x << std::endl;
#else
#	define VRPN_MAINLOOPOBJECT_MSG(_x)
#endif

class vrpn_Connection;

/// An interface for all VRPN objects that have a "mainloop" method.
/// Not instantiated directly: use vrpn_MainloopObject::wrap() to create one
class vrpn_MainloopObject {
	public:
		/// Exception thrown when trying to wrap a NULL pointer.
		struct CannotWrapNullPointerIntoMainloopObject : public std::logic_error {
			CannotWrapNullPointerIntoMainloopObject() : std::logic_error("Cannot wrap a null pointer into a vrpn_MainloopObject!") {}
		};

		/// Destructor
		virtual ~vrpn_MainloopObject() {}

		/// The mainloop function: the primary thing we look for in a VRPN object
		virtual void mainloop() = 0;

		/// Templated wrapping function
		template<class T>
		static vrpn_MainloopObject * wrap(T o);

		/// Templated wrapping function that can encourage the
		/// wrapper to not destroy the wrapped object at destruction
		template<class T>
		static vrpn_MainloopObject * wrap(T o, bool owner);
	protected:
		/// Internal function to return a typeless pointer of the contained
		/// object, for comparison purposes.
		virtual void * _returnContained() const = 0;
		vrpn_MainloopObject() {}
		friend bool operator==(vrpn_MainloopObject const & lhs, vrpn_MainloopObject const & rhs);
		friend bool operator!=(vrpn_MainloopObject const & lhs, vrpn_MainloopObject const & rhs);
};


/// @name Comparison operators
/// @relates vrpn_MainloopObject
/// @{
inline bool operator==(vrpn_MainloopObject const & lhs, vrpn_MainloopObject const & rhs) {
	return lhs._returnContained() == rhs._returnContained();
}

inline bool operator!=(vrpn_MainloopObject const & lhs, vrpn_MainloopObject const & rhs) {
	return lhs._returnContained() == rhs._returnContained();
}
/// @}

/// Namespace enclosing internal implementation details
namespace detail {
	template<class T>
	class TypedMainloopObject;

	/// Template class for holding generic VRPN objects with
	/// type information.
	template<class T>
	class TypedMainloopObject<T*> : public vrpn_MainloopObject {
		public:
			TypedMainloopObject(T * o, bool do_delete = true) :
				_instance(o),
				_do_delete(do_delete) {
				if (!o) {
					throw vrpn_MainloopObject::CannotWrapNullPointerIntoMainloopObject();
				}
				VRPN_MAINLOOPOBJECT_MSG("Wrapping vrpn object " << o)
			}
			virtual ~TypedMainloopObject() {
				if (_do_delete) {
					delete _instance;
					VRPN_MAINLOOPOBJECT_MSG("Deleted contained vrpn object " << _instance)
				} else {
					VRPN_MAINLOOPOBJECT_MSG("NOT deleting contained vrpn object " << _instance)
				}
			}

			virtual void mainloop() {
				_instance->mainloop();
			}

		protected:
			virtual void * _returnContained() const {
				return _instance;
			}
			T * _instance;
			bool _do_delete;
	};

	/// Specialization for connections, since they're reference-counted.
	template<>
	class TypedMainloopObject<vrpn_Connection *>: public vrpn_MainloopObject {
		public:
			TypedMainloopObject(vrpn_Connection * o) :
				_instance(o) {
				if (!o) {
					throw vrpn_MainloopObject::CannotWrapNullPointerIntoMainloopObject();
				}
				VRPN_MAINLOOPOBJECT_MSG("Wrapping vrpn connection " << o)
			}
			virtual ~TypedMainloopObject() {
				VRPN_MAINLOOPOBJECT_MSG("Unreferencing contained vrpn connection " << _instance)
				_instance->removeReference();
			}

			virtual void mainloop() {
				_instance->mainloop();
			}

		protected:
			virtual void * _returnContained() const {
				return _instance;
			}
			vrpn_Connection * _instance;
	};

} // end of namespace detail

template<class T>
inline vrpn_MainloopObject * vrpn_MainloopObject::wrap(T o) {
	return new detail::TypedMainloopObject<T>(o);
}

template<class T>
inline vrpn_MainloopObject * vrpn_MainloopObject::wrap(T o, bool owner) {
	return new detail::TypedMainloopObject<T>(o, owner);
}

#endif // INCLUDED_vrpn_MainloopObject_h_GUID_38f638e4_40e0_4c6d_bebc_21c463794b88
