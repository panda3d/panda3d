# This module should not import Panda3D modules globally as it contains hooks
# that may be invoked by setuptools even when Panda3D is not used.  If the
# Panda3D installation is broken, it should not affect other applications.

__all__ = ('finalize_distribution_options', )


def finalize_distribution_options(dist):
    """Entry point for compatibility with setuptools>=61, see #1394."""

    options = dist.get_option_dict('build_apps')
    if options.get('gui_apps') or options.get('console_apps'):
        # Make sure this is set to avoid auto-discovery taking place.
        if getattr(dist.metadata, 'py_modules', None) is None and \
           getattr(dist.metadata, 'packages', None) is None:
            dist.py_modules = []
