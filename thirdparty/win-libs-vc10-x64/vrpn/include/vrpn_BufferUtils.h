/** @file
	@brief Internal header providing unbuffering facilities for a number of types.

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

// Tested in the context of vrpn_server and vrpn_print_devices running between
// an SGI running Irix 6.5 MIPS 32-bit (big endian) and Mac OSX intel 64-bit
// (little endian) machine with a NULL tracker and it worked using the SGI
// repaired commits from 3/17/2012.

#pragma once
#ifndef INCLUDED_vrpn_BufferUtils_h_GUID_6a741cf1_9fa4_4064_8af0_fa0c6a16c810
#define INCLUDED_vrpn_BufferUtils_h_GUID_6a741cf1_9fa4_4064_8af0_fa0c6a16c810

// Internal Includes
#include "vrpn_Shared.h"

// Library/third-party includes
// - none

// Standard includes
#ifdef sgi
#include <assert.h>
#else
#include <cassert> // for assert
#endif
#include <string.h> // for memcpy
#include <stdio.h> // for fprintf, stderr

#if !( defined(_WIN32) && defined(VRPN_USE_WINSOCK_SOCKETS) )
#include <netinet/in.h>
#endif

/// @brief Contains overloaded hton() and ntoh() functions that forward
/// to their correctly-typed implementations.
namespace vrpn_byte_order {
	namespace detail {
		/// Traits class to get the uint type of a given size
		template<int TypeSize>
		struct uint_traits;

		template<> struct uint_traits<1> {
			typedef vrpn_uint8 type;
		};
		template<> struct uint_traits<2> {
			typedef vrpn_uint16 type;
		};
		template<> struct uint_traits<4> {
			typedef vrpn_uint32 type;
		};
	} // end of namespace detail

	/// host to network byte order for 8-bit uints is a no-op
	inline vrpn_uint8 hton(vrpn_uint8 hostval) {
		return hostval;
	}

	/// network to host byte order for 8-bit uints is a no-op
	inline vrpn_uint8 ntoh(vrpn_uint8 netval) {
		return netval;
	}

	/// host to network byte order for 16-bit uints
	inline vrpn_uint16 hton(vrpn_uint16 hostval) {
		return htons(hostval);
	}

	/// network to host byte order for 16-bit uints
	inline vrpn_uint16 ntoh(vrpn_uint16 netval) {
		return ntohs(netval);
	}

	/// host to network byte order for 32-bit uints
	inline vrpn_uint32 hton(vrpn_uint32 hostval) {
		return htonl(hostval);
	}

	/// network to host byte order for 32-bit uints
	inline vrpn_uint32 ntoh(vrpn_uint32 netval) {
		return ntohl(netval);
	}

	/// host to network byte order for 64-bit floats, using vrpn htond
	inline vrpn_float64 hton(vrpn_float64 hostval) {
		return htond(hostval);
	}

	/// network to host byte order for 64-bit floats, using vrpn ntohd
	inline vrpn_float64 ntoh(vrpn_float64 netval) {
		return ntohd(netval);
	}

	/// Templated hton that type-puns to the same-sized uint type
	/// as a fallback for those types not explicitly defined above.
	template<typename T>
	inline T hton(T input) {
		union {
			T asInput;
			typename detail::uint_traits<sizeof(T)>::type asInt;
		} inVal, outVal;
		inVal.asInput = input;
		outVal.asInt = hton(inVal.asInt);
		return outVal.asInput;
	}

	/// Templated ntoh that type-puns to the same-sized uint type
	/// as a fallback for those types not explicitly defined above.
	template<typename T>
	inline T ntoh(T input) {
		union {
			T asInput;
			typename detail::uint_traits<sizeof(T)>::type asInt;
		} inVal, outVal;
		inVal.asInput = input;
		outVal.asInt = ntoh(inVal.asInt);
		return outVal.asInput;
	}
} // end of namespace vrpn_byte_order

namespace detail {
	template<typename T>
	struct remove_const {
		typedef T type;
	};

	template<typename T>
	struct remove_const<const T> {
		typedef T type;
	};
} // end of namespace detail

/// Function template to unbuffer values from a buffer stored in network
/// byte order. Specify the type to extract T as a template parameter.
/// The templated buffer type ByteT will be deduced automatically.
/// The input pointer will be advanced past the unbuffered value.
template<typename T, typename ByteT>
static inline T vrpn_unbuffer_from_little_endian(ByteT * & input) {
	using namespace vrpn_byte_order;

	/// @todo make this a static assertion
	assert(sizeof(ByteT) == 1);

	/// Union to allow type-punning
	union {
		typename ::detail::remove_const<ByteT>::type bytes[sizeof(T)];
		T typed;
	} value;

	/// Swap known little-endian into big-endian (aka network byte order)
	for (unsigned int i = 0, j = sizeof(T) - 1; i < sizeof(T); ++i, --j) {
		value.bytes[i] = input[j];
	}

	/// Advance input pointer
	input += sizeof(T);

	/// return value in host byte order
	return ntoh(value.typed);
}

/// Function template to unbuffer values from a buffer stored in network
/// byte order. Specify the type to extract T as a template parameter.
/// The templated buffer type ByteT will be deduced automatically.
/// The input pointer will be advanced past the unbuffered value.
template<typename T, typename ByteT>
inline T vrpn_unbuffer(ByteT * & input) {
	using namespace vrpn_byte_order;

	/// @todo make this a static assertion
	assert(sizeof(ByteT) == 1);

	/// Union to allow type-punning and ensure alignment
	union {
		typename ::detail::remove_const<ByteT>::type bytes[sizeof(T)];
		T typed;
	} value;

	/// Copy bytes into union
	memcpy(value.bytes, input, sizeof(T));

	/// Advance input pointer
	input += sizeof(T);

	/// return value in host byte order
	return ntoh(value.typed);
}

namespace templated_buffer {
	/// Function template to buffer values to a buffer stored in network
	/// byte order. Specify the type to buffer T as a template parameter.
	/// The templated buffer type ByteT will be deduced automatically.
	/// The input pointer will be advanced past the unbuffered value.
	template<typename T, typename ByteT>
	inline int vrpn_buffer(ByteT ** insertPt, vrpn_int32 * buflen, const T inVal) {
		using namespace vrpn_byte_order;

		/// @todo make this a static assertion
		assert(sizeof(ByteT) == 1);

		assert(insertPt);
		assert(buflen);

		if (sizeof(T) > static_cast<size_t>(*buflen)) {
			fprintf(stderr, "vrpn_buffer: buffer not large enough\n");
			return -1;
		}

		/// Union to allow type-punning and ensure alignment
		union {
			typename ::detail::remove_const<ByteT>::type bytes[sizeof(T)];
			T typed;
		} value;

		/// Populate union in network byte order
		value.typed = hton(inVal);

		/// Copy bytes into buffer
		memcpy(*insertPt, value.bytes, sizeof(T));

		/// Advance insert pointer
		*insertPt += sizeof(T);
		/// Decrement buffer length
		*buflen -= sizeof(T);

		return 0;
	}
}

namespace templated_unbuffer {
	template<typename T, typename ByteT>
	inline int vrpn_unbuffer(ByteT ** input, T * lvalue) {
		*lvalue = ::vrpn_unbuffer<T, ByteT>(*input);
		return 0;
	}
} // end of namespace templated_unbuffer

#endif // INCLUDED_vrpn_BufferUtils_h_GUID_6a741cf1_9fa4_4064_8af0_fa0c6a16c810
