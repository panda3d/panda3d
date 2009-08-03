// This file is read and processed by ppremake to generate
// p3d_plugin_config.h.

#output p3d_plugin_config.h notouch
/* p3d_plugin_config.h.  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]]. */
/********************************** DO NOT EDIT ****************************/

/* The URL that is the root of the download server that this plugin
   should contact.  The nppanda3d.dll file should be found at this
   location; as well as the contents.xml file that defines where the
   various Panda3D packages will be found. */
#$[]define P3D_PLUGIN_DOWNLOAD "$[P3D_PLUGIN_DOWNLOAD]$[if $[notdir $[P3D_PLUGIN_DOWNLOAD]],/]"

/* The filename(s) to generate output to when the plugin is running.
   For debugging purposes only. */
#$[]define P3D_PLUGIN_LOGFILE1 "$[subst \,\\,$[osfilename $[P3D_PLUGIN_LOGFILE1]]]"
#$[]define P3D_PLUGIN_LOGFILE2 "$[subst \,\\,$[osfilename $[P3D_PLUGIN_LOGFILE2]]]"

/* For development only: the location at which p3dpython.exe can be
   found.  Empty string for the default. */
#$[]define P3D_PLUGIN_P3DPYTHON "$[subst \,\\,$[osfilename $[P3D_PLUGIN_P3DPYTHON]]]"

/* For development only: the location at which p3d_plugin.dll/.so can
   be found.  Empty string for the default. */
#$[]define P3D_PLUGIN_P3D_PLUGIN "$[subst \,\\,$[osfilename $[P3D_PLUGIN_P3D_PLUGIN]]]"

#end p3d_plugin_config.h
