//
// Template.msvc.pp
//
// This file defines the set of ouput files that will be generated to
// support a Microsoft Visual C++ .NET solution / project build.  It rolls
// libraries into their metalibs, generates .dll files, compiles source
// files to .obj files, and does other Windows-specific things.

// Before this file is processed, the following files are read and
// processed (in order):

// The Package.pp file in the root of the current source hierarchy
//   (e.g. $PANDA/Package.pp)
// $DTOOL/Package.pp
// $DTOOL/Config.pp
// $DTOOL/Config.Platform.pp
// The user's PPREMAKE_CONFIG file.
// $DTOOL/pptempl/System.pp
// All of the Sources.pp files in the current source hierarchy
// $DTOOL/pptempl/Global.pp
// $DTOOL/pptempl/Global.msvc.pp
// $DTOOL/pptempl/Depends.pp, once for each Sources.pp file
// Template.msvc.pp (this file), once for each Sources.pp file


#defun decygwin frompat,topat,path
  #foreach file $[path]
    #if $[isfullpath $[file]]
      $[patsubstw $[frompat],$[topat],$[cygpath_w $[file]]]
    #else
      $[patsubstw $[frompat],$[topat],$[osfilename $[file]]]
    #endif
  #end file
#end decygwin

// should overwrite read-only files
#define COPYCMD xcopy /Y /Q /R

#if $[ne $[CTPROJS],]
#define dtool_ver_dir_cyg $[DTOOL]/src/dtoolbase
#define dtool_ver_dir $[decygwin %,%,$[dtool_ver_dir_cyg]]
#endif

//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////
// For a source directory, build a single Makefile with rules to build
// each target.

#if $[build_directory]
  // This is the real set of lib_targets we'll be building.  On Windows,
  // we don't build the shared libraries which are included on metalibs.
  #define real_lib_targets
  #define deferred_objs
  #forscopes lib_target
    #if $[eq $[module $[TARGET],$[TARGET]],]
      // This library is not on a metalib, so we can build it.
      #set real_lib_targets $[real_lib_targets] $[active_target]
    #else
      // This library is on a metalib, so we can't build it, but we
      // should build all the obj's that go into it.
      #set deferred_objs $[deferred_objs] \
        $[patsubst %_src.cxx,,%.c %.cxx %.yxx %.lxx,$[so_dir]\%.obj,%,,$[get_sources] $[get_igateoutput]]
    #endif
  #end lib_target

  #defer lib_projects \
    $[active_target(metalib_target lib_target noinst_lib_target)] \
    $[real_lib_targets]
  #defer static_lib_projects \
    $[active_target(static_lib_target ss_lib_target)]
  #defer bin_projects \
    $[active_target(bin_target noinst_bin_target)]
  #defer test_bin_projects \
    $[active_target(test_bin_target)]

  // We need to know the various targets we'll be building.
  // $[lib_targets] will be the list of dynamic libraries,
  // $[static_lib_targets] the list of static libraries, and
  // $[bin_targets] the list of binaries.  $[test_bin_targets] is the
  // list of binaries that are to be built only when specifically asked
  // for.
  #define lib_targets $[patsubst %,lib%$[dllext].$[dlllib],$[lib_projects]]
  #define static_lib_targets $[static_lib_projects:%=lib%$[dllext].lib]
  #define bin_targets $[bin_projects:%=%.exe] $[active_target(sed_bin_target)]
  #define test_bin_targets $[test_bin_projects:%=%.exe]

  #define all_targets $[unique $[lib_targets] $[static_lib_targets] \
                               $[bin_targets] $[test_bin_targets]]

  // $[so_sources] is the set of sources that belong on a shared object,
  // and $[st_sources] is the set of sources that belong on a static
  // object, like a static library or an executable.  In Windows, we
  // don't need to make this distinction, but we do anyway in case we
  // might in the future for some nutty reason.
  #defer so_sources $[get_sources(metalib_target lib_target noinst_lib_target)]
  #defer st_sources $[get_sources(static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]

  // These are the source files that our dependency cache file will
  // depend on.  If it's an empty list, we won't bother writing rules to
  // freshen the cache file.
  #define dep_sources $[sort $[filter %.c %.cxx %.yxx %.lxx %.h %.I %.I,$[so_sources] $[st_sources]]]

  #if $[eq $[so_dir],$[st_dir]]
    // If the static and shared directories are the same, we have to use the
    // same rules to build both shared and static targets.
    #set st_sources $[so_sources] $[st_sources]
    #set so_sources
  #endif
#endif  // $[build_directory]

// And these are the various source files, extracted out by type.
#defer cxx_so_sources $[filter_out %_src.cxx,$[filter %.cxx,$[so_sources]]]
#defer cxx_st_sources $[filter_out %_src.cxx,$[filter %.cxx,$[st_sources]]]
#defer c_so_sources $[filter %.c,$[so_sources]]
#defer c_st_sources $[filter %.c,$[st_sources]]
#defer yxx_so_sources $[filter %.yxx,$[so_sources]]
#defer yxx_st_sources $[filter %.yxx,$[st_sources]]
#defer lxx_so_sources $[filter %.lxx,$[so_sources]]
#defer lxx_st_sources $[filter %.lxx,$[st_sources]]
#defer h_sources $[filter %.h,$[so_sources] $[st_sources]]
#defer i_sources $[filter %.I,$[so_sources] $[st_sources]]

#if $[DO_PCH]
#define pch_header_source $[get_precompiled_header(metalib_target lib_target noinst_lib_target)]

#define st_pch_files $[patsubst %.h,$[st_dir]\%.pch,$[pch_header_source]]
#define st_pch_obj_files $[patsubst %.h,$[st_dir]\%.obj,$[pch_header_source]]

#endif

// This map variable gets us all the various source files from all the
// targets in this directory.  We need it to look up the context in
// which to build a particular source file, since some targets may
// have different requirements (e.g. different local_libs, or
// different USE_this or USE_that) than other targets.
#map all_sources get_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)

// $[target_ipath] is the proper ipath to put on the command line,
// from the context of a particular target.
#defer target_ipath $[RELDIR] $[TOPDIR] $[sort $[complete_ipath]] $[other_trees:%=%\include] $[get_ipath]

// $[converted_ipath] is the properly-formatted version of the include path
// for Visual Studio .NET.  The resulting list is semicolon separated and uses
// Windows-style pathnames.
#defer converted_ipath $[join ;,$[osfilename $[target_ipath]]]

// $[file_ipath] is the ipath from the context of a particular source
// file, given in $[file].  It uses the all_sources map to look up
// the target the source file belongs on, to get the proper context.
#defer file_ipath $[all_sources $[target_ipath],$[file]]

// These are the complete set of extra flags the compiler requires,
// from the context of a particular file, given in $[file].
#defer cflags $[all_sources $[get_cflags] $[CFLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]
#defer c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]

// These are the same flags, sans the compiler optimizations.
#defer noopt_c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]

// $[complete_lpath] is rather like $[complete_ipath]: the list of
// directories (from within this tree) we should add to our -L list.
#defer complete_lpath $[libs $[RELDIR:%=%\$[ODIR]],$[actual_local_libs]] $[EXTRA_LPATH]

// $[lpath] is like $[target_ipath]: it's the list of directories we
// should add to our -L list, from the context of a particular target.
#defer lpath $[sort $[complete_lpath]] $[other_trees:%=%\lib] $[get_lpath]

// $[converted_lpath] is the properly-formatted version of the library path
// for Visual Studio .NET.  The resulting list is semicolon separated and uses
// Windows-style pathnames.
#defer converted_lpath $[join ;,$[osfilename $[lpath]]]

// And $[libs] is the set of libraries we will link with.
#defer libs $[unique $[actual_local_libs:%=%$[dllext]] $[patsubst %:c,,%:m %,%$[dllext],$[OTHER_LIBS]] $[get_libs]]

#defer converted_libs $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]]

// This is the set of files we might copy into *.prebuilt, if we have
// bison and flex (or copy from *.prebuilt if we don't have them).
#define bison_prebuilt $[patsubst %.yxx,%.h,$[yxx_so_sources] $[yxx_st_sources]] $[patsubst %.yxx,%.cxx,$[yxx_so_sources] $[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_so_sources] $[lxx_st_sources]]

// Rather than making a rule to generate each install directory later,
// we create the directories now.  This reduces problems from
// multiprocess builds.
#mkdir $[sort \
    $[if $[install_lib],$[install_lib_dir]] \
    $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
    $[if $[install_headers],$[install_headers_dir]] \
    $[if $[install_parser_inc],$[install_parser_inc_dir]] \
    $[if $[install_data],$[install_data_dir]] \
    $[if $[install_config],$[install_config_dir]] \
    $[if $[install_igatedb],$[install_igatedb_dir]] \
    $[if $[install_py],$[install_py_dir] $[install_py_package_dir]] \
    ]

// Pre-compiled headers are one way to speed the compilation of many
// C++ source files that include similar headers, but it turns out a
// more effective (and more portable) way is simply to compile all the
// similar source files in one pass.

// We do this by generating a *_composite.cxx file that has an
// #include line for each of several actual source files, and then we
// compile the composite file instead of the original files.
#foreach composite_file $[composite_list]
#output $[composite_file] notouch
#format collapse
/* Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE]
. */
/* ################################# DO NOT EDIT ########################### */

#foreach file $[$[composite_file]_sources]
##include "$[file]"
#end file

#end $[composite_file]
#end composite_file


/////////////////////////////////////////////////////////////////////
// First, the dynamic libraries.  Each lib_target and metalib_target
// is a dynamic library.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target
// In Windows, we don't actually build all the libraries.  In
// particular, we don't build any libraries that are listed on a
// metalib.  Is this one such library?
#define build_it $[eq $[module $[TARGET],$[TARGET]],]

// We might need to define a BUILDING_ symbol for win32.  We use the
// BUILDING_DLL variable name, defined typically in the metalib, for
// this; but in some cases, where the library isn't part of a metalib,
// we define BUILDING_DLL directly for the target.
#define building_var $[or $[BUILDING_DLL],$[module $[BUILDING_DLL],$[TARGET]]]
#define defines $[join ;,$[extra_defines] $[building_var]]

// $[igatescan] is the set of C++ headers and source files that we
// need to scan for interrogate.  $[igateoutput] is the name of the
// generated .cxx file that interrogate will produce (and which we
// should compile into the library).  $[igatedb] is the name of the
// generated .in file that interrogate will produce (and which should
// be installed into the /etc directory).
#define igatescan $[get_igatescan]
#define igateoutput $[get_igateoutput]
#define igatedb $[get_igatedb]

// If this is a metalib, it may have a number of components that
// include interrogated interfaces.  If so, we need to generate a
// 'module' file within this library.  This is mainly necessary for
// Python; it contains a table of all of the interrogated functions,
// so we can load the library as a Python module and have access to
// the interrogated functions.

// $[igatemscan] is the set of .in files generated by all of our
// component libraries.  If it is nonempty, then we do need to
// generate a module, and $[igatemout] is the name of the .cxx file
// that interrogate will produce to make this module.
#define igatemscan $[get_igatemscan]
#define igatemout $[get_igatemout]

#if $[build_it]
  #define target $[ODIR]\$[lib_prefix]$[TARGET].$[dlllib]

  // Installation paths
  #define mybasename $[basename $[notdir $[target]]]
  #define tmpdirname_cyg $[install_lib_dir]/$[mybasename]
  #define tmpdirname_win $[osfilename $[tmpdirname_cyg]]

  // List of object files that will be combined to form this metalib target.
  #define objects \
    $[components \
      $[osfilename $[patsubst %,$[RELDIR]\$[%_obj],$[compile_sources]]], \
      $[active_component_libs]]
#endif $[build_it]

// Additional rules to generate and compile the interrogate data, if needed.
#if $[igatescan]
  #define igatelib $[lib_prefix]$[TARGET]

  // The module name comes from the metalib that includes this library.
  #define igatemod $[module $[TARGET],$[TARGET]]
  #if $[eq $[igatemod],]
    // ... unless no metalib includes this library.
    #define igatemod $[TARGET]
  #endif

  // Built the complete interrogate.exe commandline
  #define igate_commandline \
    $[install_bin_dir]\$[INTERROGATE] -od $[igatedb] -oc $[igateoutput] \
    $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" \
    $[igatescan]

  // TODO: Install $[igatedb] in $[install_igatedb_dir]
#endif  // igatescan

// And finally, some additional rules to build the interrogate module
// file into the library, if this is a metalib that includes
// interrogated components.
#if $[igatemout]
  #define igatelib $[lib_prefix]$[TARGET]
  #define igatemod $[TARGET]

  #define igatemod_commandline \
    $[install_bin_dir]\$[INTERROGATE_MODULE] -oc $[igatemout] \
    -module "$[igatemod]" -library "$[igatelib]" $[interrogate_module_options] \
    $[igatemscan]
#endif  // igatemout

#output $[TARGET].vcproj
#format straight
<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.00"
	Name="$[TARGET]"
	ProjectGUID="{$[makeguid $[TARGET]]}"
	Keyword="CustomAppWizProj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="$[ODIR]|Win32"
			IntermediateDirectory="$[ODIR]"
			OutputDirectory="$[ODIR]"
#if $[build_it]
			ConfigurationType="2">
#else
			ConfigurationType="4">
#endif
			<Tool
				Name="VCCLCompilerTool"
				AdditionalOptions="/Zm350"
				AdditionalIncludeDirectories="$[converted_ipath]"
				PreprocessorDefinitions="$[defines]"
				RuntimeLibrary="2"/>
			<Tool
				Name="VCCustomBuildTool"/>
#if $[build_it]
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="$[converted_libs] $[objects]"
				OutputFile="$[target]"
				AdditionalLibraryDirectories="$[converted_lpath]"/>
#else
			<Tool
				Name="VCLibrarianTool"
				OutputFile=""/>
#endif
			<Tool
				Name="VCMIDLTool"/>
#if $[build_it]
			<Tool
				Name="VCPostBuildEventTool"
				Description="Copying $[target] to $[install_lib_dir]..."
				CommandLine="$[COPYCMD] $[target] $[install_bin_dir]"/>
#else
			<Tool
				Name="VCPostBuildEventTool"/>
#endif
#if $[igatescan]
			<Tool
				Name="VCPreBuildEventTool"
				Description="Generating $[igateoutput] using $[INTERROGATE]..."
				CommandLine='$[igate_commandline]'/>
#elif $[igatemscan]
			<Tool
				Name="VCPreBuildEventTool"
				Description="Generating $[igatemout] using $[INTERROGATE_MODULE]..."
				CommandLine='$[igatemod_commandline]'/>
#else
			<Tool
				Name="VCPreBuildEventTool"/>
#endif
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCResourceCompilerTool"/>
		</Configuration>
	</Configurations>
	<Files>
		<Filter
			Name="Build"
			Filter="">
			<File
				RelativePath="Sources.pp">
			</File>
			<File
				RelativePath="$[TARGET].vcproj">
			</File>
		</Filter>
#if $[compile_sources]
#foreach file $[sort $[compile_sources]]
		<File
			RelativePath="$[file]">
#if $[or $[eq $[file],$[igateoutput]],$[eq $[file],$[igatemout]]]
			<FileConfiguration
				Name="Opt3-Win32|Win32">
				<Tool
					Name="VCCLCompilerTool"
					ObjectFile="$[patsubst %,$[%_obj],$[file]]"/>
			</FileConfiguration>
#endif
		</File>
#end file
#endif
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
#end $[TARGET].vcproj
#end metalib_target lib_target


/////////////////////////////////////////////////////////////////////
// Now the static libraries.  Again, we assume there's no interrogate
// interfaces going on in here, and there's no question of this being
// a metalib, making the rules relatively simple.
/////////////////////////////////////////////////////////////////////

#forscopes static_lib_target ss_lib_target
#define target $[ODIR]\$[lib_prefix]$[TARGET].lib
#define defines $[join ;,$[extra_defines]]

#output $[TARGET].vcproj
#format straight
<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.00"
	Name="$[TARGET]"
	ProjectGUID="{$[makeguid $[TARGET]]}"
	Keyword="CustomAppWizProj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="$[ODIR]|Win32"
			IntermediateDirectory="$[ODIR]"
			OutputDirectory="$[ODIR]"
			ConfigurationType="4">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalOptions="/Zm350"
				AdditionalIncludeDirectories="$[converted_ipath]"
				PreprocessorDefinitions="$[defines]"
				RuntimeLibrary="2"/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCLibrarianTool"
				OutputFile="$[target]"/>
			<Tool
				Name="VCMIDLTool"/>
			<Tool
				Name="VCPostBuildEventTool"
				Description="Copying $[target] to $[install_lib_dir]..."
				CommandLine="$[COPYCMD] $[target] $[install_lib_dir]"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCResourceCompilerTool"/>
		</Configuration>
	</Configurations>
	<Files>
#if $[compile_sources]
#foreach file $[sort $[compile_sources]]
		<File
			RelativePath="$[file]">
		</File>
#end file
#endif
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
#end $[TARGET].vcproj
#end static_lib_target ss_lib_target


/////////////////////////////////////////////////////////////////////
// And now, the bin_targets.  These are normal C++ executables.  No
// interrogate, metalibs, or any such nonsense here.
/////////////////////////////////////////////////////////////////////

#forscopes bin_target
#define target $[ODIR]\$[TARGET].exe
#define defines $[join ;,$[extra_defines]]

#output $[TARGET].vcproj
#format straight
<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.00"
	Name="$[TARGET]"
	ProjectGUID="{$[makeguid $[TARGET]]}"
	Keyword="CustomAppWizProj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="$[ODIR]|Win32"
			IntermediateDirectory="$[ODIR]"
			OutputDirectory="$[ODIR]"
			ConfigurationType="1">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalOptions="/Zm350"
				AdditionalIncludeDirectories="$[converted_ipath]"
				PreprocessorDefinitions="$[defines]"
				RuntimeLibrary="2"/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCLinkerTool"
				AdditionalDependencies="$[converted_libs] $[objects]"
				OutputFile="$[target]"
				AdditionalLibraryDirectories="$[converted_lpath]"/>
			<Tool
				Name="VCMIDLTool"/>
			<Tool
				Name="VCPostBuildEventTool"
				Description="Copying $[target] to $[install_bin_dir]..."
				CommandLine="$[COPYCMD] $[target] $[install_bin_dir]"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCResourceCompilerTool"/>
		</Configuration>
	</Configurations>
	<Files>
#if $[compile_sources]
#foreach file $[sort $[compile_sources]]
		<File
			RelativePath="$[file]">
		</File>
#end file
#endif
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
#end $[TARGET].vcproj
#end bin_target


//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], group]
//////////////////////////////////////////////////////////////////////

// This is a group directory: a directory above a collection of source
// directories, e.g. $DTOOL/src.  We don't need to output anything in
// this directory.


//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], toplevel]
//////////////////////////////////////////////////////////////////////

// This is the toplevel directory, e.g. $DTOOL.  Here we build the
// package solution file and also synthesize the dtool_config.h (or
// whichever file) we need.

//#define project_scopes */static_lib_target */ss_lib_target */lib_target */noinst_lib_target */test_lib_target */metalib_target */bin_target */test_bin_target
#define project_scopes */metalib_target */lib_target */static_lib_target */ss_lib_target */bin_target

#output $[PACKAGE].sln
#format straight
Microsoft Visual Studio Solution File, Format Version 7.00
#forscopes $[project_scopes]
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "$[TARGET]", "$[osfilename $[PATH]]\$[TARGET].vcproj", "{$[makeguid $[TARGET]]}"
EndProject
#end $[project_scopes]
Global
	GlobalSection(SolutionConfiguration) = preSolution
		ConfigName.0 = $[ODIR]
	EndGlobalSection
	GlobalSection(ProjectDependencies) = postSolution
#forscopes $[project_scopes]
  #define count 0
  #foreach dependency $[DEPEND_DIRS]
		{$[makeguid $[TARGET]]}.$[count] = {$[makeguid $[dependency]]}
    #set count $[+ $[count],1]
  #end dependency
#end $[project_scopes]
	EndGlobalSection
	GlobalSection(ProjectConfiguration) = postSolution
#forscopes $[project_scopes]
#define guid $[makeguid $[TARGET]]
		{$[guid]}.$[ODIR].ActiveCfg = $[ODIR]|Win32
		{$[guid]}.$[ODIR].Build.0 = $[ODIR]|Win32
#end $[project_scopes]
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
	EndGlobalSection
	GlobalSection(ExtensibilityAddIns) = postSolution
	EndGlobalSection
EndGlobal
#end $[PACKAGE].sln

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.msvc.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp

//////////////////////////////////////////////////////////////////////
#elif $[or $[eq $[DIR_TYPE], models],$[eq $[DIR_TYPE], models_toplevel],$[eq $[DIR_TYPE], models_group]]
//////////////////////////////////////////////////////////////////////

#include $[THISDIRPREFIX]Template.models.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE
