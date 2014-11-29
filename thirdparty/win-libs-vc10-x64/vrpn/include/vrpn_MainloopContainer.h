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
#ifndef INCLUDED_vrpn_MainloopContainer_h_GUID_2146c66c_1925_4ac3_a192_354d10d7a39f
#define INCLUDED_vrpn_MainloopContainer_h_GUID_2146c66c_1925_4ac3_a192_354d10d7a39f

// Internal Includes
#include "vrpn_MainloopObject.h"

// Library/third-party includes
// - none

// Standard includes
#include <vector>

/// A container that holds and owns one or more VRPN objects,
class vrpn_MainloopContainer {
	public:
		/// Constructor
		vrpn_MainloopContainer() {}
		/// Destructor: invokes clear()
		~vrpn_MainloopContainer();

		/// Clear internal structure holding objects, deleting them
		/// in reverse order of their addition.
		void clear();

		/// Add an object wrapped by vrpn_MainloopObject.
		void add(vrpn_MainloopObject * o);

		/// Template method to automatically wrap objects
		/// with vrpn_MainloopObject before adding them.
		template<class T>
		void add(T o) {
			add(vrpn_MainloopObject::wrap(o));
		}

		/// Runs mainloop on all contained objects, in the order
		/// that they were added.
		void mainloop();

	private:
		std::vector<vrpn_MainloopObject *> _vrpn;
};

/* -- inline implementations -- */

inline vrpn_MainloopContainer::~vrpn_MainloopContainer() {
	clear();
}

inline void vrpn_MainloopContainer::add(vrpn_MainloopObject* o) {
	if (!o) {
		return;
	}
	_vrpn.push_back(o);
}

inline void vrpn_MainloopContainer::clear() {
	if (_vrpn.empty()) {
		return;
	}
	/// Delete in reverse order
	for (size_t i = _vrpn.size() - 1; i >= 0; --i) {
		delete _vrpn[i];
		_vrpn[i] = NULL;
	}
	_vrpn.clear();
}

inline void vrpn_MainloopContainer::mainloop() {
	const size_t n = _vrpn.size();
	for (size_t i = 0; i < n; ++i) {
		_vrpn[i]->mainloop();
	}
}

#endif // INCLUDED_vrpn_MainloopContainer_h_GUID_2146c66c_1925_4ac3_a192_354d10d7a39f
