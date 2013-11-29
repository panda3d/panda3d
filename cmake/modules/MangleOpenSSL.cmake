# Filename: MangleOpenSSL.cmake
# Author: kestred (28 Nov, 2013)
#
# MangleOpenSSL replaces the output variables of the builtin FindOpenSSL
# with vars matching Panda3D's existing config-var names.
#

if(OPENSSL_FOUND)
	set(HAVE_OPENSSL TRUE)
	set(OPENSSL_LIBS ssl crypto)

	list(GET OPENSSL_LIBRARIES 0 OPENSSL_LIBRARY)

	get_filename_component(OPENSSL_LIBRARY_DIR "${OPENSSL_LIBRARY}" PATH)

	set(OPENSSL_IPATH "${OPENSSL_INCLUDE_DIR}" CACHE PATH "The path to OpenSSL's include directory.") # Include path
	set(OPENSSL_LPATH "${OPENSSL_LIBRARY_DIR}" CACHE PATH "The path to OpenSSL's library directory.") # Library path
	mark_as_advanced(OPENSSL_IPATH)
	mark_as_advanced(OPENSSL_LPATH)

	unset(OPENSSL_FOUND)
	unset(OPENSSL_LIBRARY)
	unset(OPENSSL_LIBRARIES)
	unset(OPENSSL_INCLUDE_DIR CACHE)
	unset(OPENSSL_LIBRARY_DIR)
endif()