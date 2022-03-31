find_package(PkgConfig QUIET)

set(__gtk3_required_version "${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION}")
if(__gtk3_required_version)
    set(__gtk3_required_version " >= ${__gtk3_required_version}")
endif()
pkg_check_modules(GTK3 QUIET "gtk+-3.0${__gtk3_required_version}" IMPORTED_TARGET)

if (NOT TARGET PkgConfig::GTK3)
    set(GTK3_FOUND 0)
endif()
unset(__gtk3_required_version)
