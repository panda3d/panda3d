/** @file
	@brief Header allowing use of a output stream-style method of sending
	text messages from devices. Contains the operator<< overload needed to
	stream into a vrpn_BaseClassUnique::SendTextMessageBoundCall, and the
	proxy object containing an ostringstream that executes the bound call
	when it's done having data streamed in to it.

	This is a separate header to avoid including <sstream> in vrpn_BaseClass.h
	since the functionality is only used by device implementations themselves,
	and not even all of them. Further, since we create the stream proxy
	only once we've seen an operator<< (rather than as a part of the bound call),
	we know that if a stream proxy exists, it has a message to send.

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
#ifndef INCLUDED_vrpn_SendTextMessageStreamProxy_h_GUID_4845c20b_f6d9_46ae_87f2_eb0a08f8a584
#define INCLUDED_vrpn_SendTextMessageStreamProxy_h_GUID_4845c20b_f6d9_46ae_87f2_eb0a08f8a584

// Internal Includes
#include "vrpn_BaseClass.h"

// Library/third-party includes
// - none

// Standard includes
#include <sstream>

class vrpn_SendTextMessageStreamProxy {
	private:
		/// Functor allowing us to perform a send_text_message call on
		/// a device's behalf once we're done having data streamed in.
		vrpn_BaseClassUnique::SendTextMessageBoundCall const& _call;

		/// Stream that will accumulate the message
		std::ostringstream _s;

		/// assignment operator disabled
		vrpn_SendTextMessageStreamProxy& operator=(vrpn_SendTextMessageStreamProxy const&);
	public:

		/// Templated constructor taking anything streamable.
		template<typename T>
		vrpn_SendTextMessageStreamProxy(vrpn_BaseClassUnique::SendTextMessageBoundCall const& call, T const& firstData)
			: _call(call) {
			_s << firstData;
		}

		/// Constructor taking a std::string, since we can use the ostringstream's
		/// std::string constructor in this case.
		vrpn_SendTextMessageStreamProxy(vrpn_BaseClassUnique::SendTextMessageBoundCall const& call, std::string const& firstData)
			: _call(call)
			, _s(firstData) {}

		/// Copy constructor - required for return by value (?)
		vrpn_SendTextMessageStreamProxy(vrpn_SendTextMessageStreamProxy const& other)
			: _call(other._call)
			, _s(other._s.str())
		{}

		/// Destructor performs the send_text_message call with all contents
		/// streamed into it.
		~vrpn_SendTextMessageStreamProxy() {
			_call(_s.str().c_str());
		}

		/// Template operator<<, used for the second item streamed into the results
		/// of a BoundCall-returning send_text_message() call. The first one is
		/// handled below, and creates this temporary proxy object. Now, we can
		/// return a reference to the internal ostream, and we'll still
		/// stick around until the end of the statement to make the call
		///once it's all done.
		template<typename T>
		std::ostream & operator<<(T const& other) {
			_s << other;
			return _s;
		}
};

/// Templated operator << that takes a vrpn_BaseClassUnique::SendTextMessageBoundCall on the left, and some data on the
/// right, and uses that to initialize and return a temporary vrpn_SendTextMessageStreamProxy who will be able to accept
/// additional streamed data before making the send_text_message call in its destructor at the
/// end of the statement.
template<typename T>
vrpn_SendTextMessageStreamProxy operator<<(vrpn_BaseClassUnique::SendTextMessageBoundCall const& call, T const& firstData) {
	return vrpn_SendTextMessageStreamProxy(call, firstData);
}

#endif // INCLUDED_vrpn_SendTextMessageStreamProxy_h_GUID_4845c20b_f6d9_46ae_87f2_eb0a08f8a584
