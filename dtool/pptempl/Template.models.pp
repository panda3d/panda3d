//
// Template.models.pp
//
// This file defines the Makefiles that will be built to generate
// models (egg, bam models computed from flt, soft, alias,
// etc. sources).  Unlike the other Template files, this is not based
// directly on the BUILD_TYPE, but is specifically included when a
// directory speficies a DIR_TYPE of "models".  It uses some
// Unix-specific conventions (like forward slashes as a directory
// separator), so it requires either a Unix platform or a Cygwin
// environment.
//

#if $[< $[PPREMAKE_VERSION],0.57]
  #error You need at least ppremake version 0.58 to build models.
#endif

#define texattrib_dir $[TOPDIR]/src/maps
#define texattrib_file $[texattrib_dir]/textures.txa

//////////////////////////////////////////////////////////////////////
#if $[eq $[DIR_TYPE], models]
//////////////////////////////////////////////////////////////////////

#define pal_egg_dir pal_egg
#define bam_dir bams
#define soft_maps_dir soft_maps
#defer phase_prefix $[if $[PHASE],phase_$[PHASE]/]

#defer install_model_dir $[install_dir]/$[phase_prefix]$[INSTALL_TO]
#define filter_dirs $[TARGET_DIR(filter_egg optchar_egg)]

#define build_eggs $[sort \
   $[SOURCES(flt_egg):%.flt=%.egg] \
   $[forscopes soft_char_egg,$[POLY_MODEL:%=$[EGG_PREFIX]%.egg] $[NURBS_MODEL:%=$[EGG_PREFIX]%.egg]] \
   $[forscopes soft_char_egg,$[ANIMS:%=$[EGG_PREFIX]%.egg]] \
   ]
#define install_eggs $[sort $[SOURCES(install_egg)] $[UNPAL_SOURCES(install_egg)]]

#define install_egg_dirs $[sort $[forscopes install_egg,$[install_model_dir]]]
#define installed_eggs $[sort $[forscopes install_egg,$[SOURCES:%=$[install_model_dir]/%] $[UNPAL_SOURCES:%=$[install_model_dir]/%]]]
#define installed_bams $[sort $[forscopes install_egg,$[SOURCES:%.egg=$[install_model_dir]/%.bam] $[UNPAL_SOURCES:%.egg=$[install_model_dir]/%.bam]]]

#define pal_egg_targets $[sort $[SOURCES(install_egg):%=$[pal_egg_dir]/%]]
#define bam_targets $[install_eggs:%.egg=$[bam_dir]/%.bam]

#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#define all_targets \
    Makefile \
    $[texattrib_dir] \
    $[filter_dirs] \
    $[optchar_dirs] \
    egg bam
all : $[all_targets]

#define egg_targets \
    $[if $[POLY_MODEL(soft_char_egg)] $[NURBS_MODEL(soft_char_egg)],$[soft_maps_dir]] \
    $[build_eggs]
egg : $[egg_targets]

#define filter_targets \
    $[filter_dirs] \
    $[forscopes install_egg,$[patsubst %,$[SOURCE_DIR:%=%/]%,$[SOURCES] $[UNPAL_SOURCES]]]
filter : egg $[filter_targets]

pal : filter $[if $[pal_egg_targets],$[pal_egg_dir]] $[pal_egg_targets]

bam : pal $[if $[bam_targets],$[bam_dir]] $[bam_targets]

#define install_egg_targets \
    $[install_egg_dirs] \
    $[installed_eggs]
install-egg : $[install_egg_targets]

#define install_bam_targets \
    $[install_egg_dirs] \
    $[installed_bams]
install-bam : $[install_bam_targets]

install : install-bam

clean-bam :
#if $[bam_targets]
	rm -rf $[bam_dir]
#endif

clean-pal : clean-bam
#if $[pal_egg_targets]
	rm -rf $[pal_egg_dir]
#endif

clean : clean-pal
#if $[build_eggs]
	rm -f $[build_eggs] $[install_eggs:%.egg=%.pt]
#endif
#if $[POLY_MODEL(soft_char_egg)] $[NURBS_MODEL(soft_char_egg)]
	rm -rf $[soft_maps_dir]
#endif
#if $[filter_dirs]
	rm -rf $[filter_dirs]
#endif

// We need a rule for each directory we might need to make.  This
// loops through the full set of directories and creates a rule to
// make each one, as needed.
#foreach directory $[sort \
    $[filter_dirs] \
    $[if $[pal_egg_targets],$[pal_egg_dir]] \
    $[if $[bam_targets],$[bam_dir]] \
    $[if $[POLY_MODEL(soft_char_egg)] $[NURBS_MODEL(soft_char_egg)],$[soft_maps_dir]] \
    $[texattrib_dir] \
    $[install_egg_dirs] \
    ]
$[directory] :
	@test -d $[directory] || echo mkdir -p $[directory]
	@test -d $[directory] || mkdir -p $[directory]
#end directory


// Egg file generation from Flt files.
#forscopes flt_egg
  #foreach flt $[SOURCES]
    #define target $[flt:%.flt=%.egg]
    #define source $[flt]
$[target] : $[source]
	flt2egg -no -rt -o $[target] $[source]

  #end flt
#end flt_egg

// Egg character model generation from Soft databases.
#forscopes soft_char_egg
  #if $[POLY_MODEL]
    #define target $[EGG_PREFIX]$[POLY_MODEL].egg
    #define scene $[SCENE_PREFIX]$[MODEL].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
$[target] : $[source]
	soft2egg $[SOFT2EGG_OPTS] -p -M $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] -t $[soft_maps_dir]
  #endif
  #if $[NURBS_MODEL]
    #define target $[EGG_PREFIX]$[NURBS_MODEL].egg
    #define scene $[SCENE_PREFIX]$[MODEL].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
$[target] : $[source]
	soft2egg $[SOFT2EGG_OPTS] -n -M $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] -t $[soft_maps_dir]
  #endif

#end soft_char_egg

// Egg animation generation from Soft database.
#forscopes soft_char_egg
  #foreach anim $[ANIMS]
    #define target $[EGG_PREFIX]$[anim].egg
    #define scene $[SCENE_PREFIX]$[anim].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
    #define begin $[or $[$[anim]_b],1]
    #define end $[$[anim]_e]
$[target] : $[source]
	soft2egg $[SOFT2EGG_OPTS] -a -A $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] $[begin:%=-b%] $[end:%=-e%]
  #end anim
#end soft_char_egg


// Generic egg filters.
#forscopes filter_egg
  #define source_prefix $[SOURCE_DIR:%=%/]
  #foreach egg $[SOURCES]
    #define source $[source_prefix]$[egg]
    #define target $[TARGET_DIR]/$[egg]
$[target] : $[source] $[pt]
	$[COMMAND]
  #end egg
#end filter_egg

// Character optimization.
#forscopes optchar_egg
  #define source_prefix $[SOURCE_DIR:%=%/]
  #define sources $[SOURCES:%=$[source_prefix]%]
  #define target $[TARGET_DIR]/$[notdir $[firstword $[SOURCES]]]

   // A bunch of rules to make each generated egg file depend on the
   // first one.
  #foreach egg $[notdir $[wordlist 2,9999,$[SOURCES]]]
$[TARGET_DIR]/$[egg] : $[target]
	touch $[TARGET_DIR]/$[egg]
  #end egg

   // And this is the actual optchar pass.
$[target] : $[sources]
	egg-optchar $[OPTCHAR_OPTS] -d $[TARGET_DIR] $[sources]
#end optchar_egg


// Palettization rules.
#forscopes install_egg
  #foreach egg $[SOURCES]
    #define pt $[egg:%.egg=$[SOURCE_DIR]/%.pt]
    #define source $[SOURCE_DIR]/$[egg]
    #define target $[pal_egg_dir]/$[egg]
$[target] : $[source] $[pt]
    #if $[PHASE]
	egg-palettize-new -C -dm $[install_dir]/%s/maps -g phase_$[PHASE] -gdir phase_$[PHASE] -P256,256 -2 -o $[target] $[texattrib_file] $[source]
    #else
	egg-palettize-new -C -dm $[install_dir]/maps -P256,256 -2 -o $[target] $[texattrib_file] $[source]
    #endif

$[pt] :
	touch $[pt]

  #end egg
#end install_egg


// Bam file creation.
#forscopes install_egg
  #foreach egg $[SOURCES]
    #define source $[pal_egg_dir]/$[egg]
    #define target $[bam_dir]/$[egg:%.egg=%.bam]
$[target] : $[source]
	egg2bam -o $[target] $[source]

  #end egg
  #foreach egg $[UNPAL_SOURCES]
    #define source $[SOURCE_DIR]/$[egg]
    #define target $[bam_dir]/$[egg:%.egg=%.bam]
$[target] : $[source]
	egg2bam -o $[target] $[source]

  #end egg
#end install_egg


// Egg file installation.
#forscopes install_egg
  #foreach egg $[SOURCES]
    #define local $[egg]
    #define sourcedir $[pal_egg_dir]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[sourcedir]/$[local]
	cd ./$[sourcedir] && $[INSTALL]

  #end egg
  #foreach egg $[UNPAL_SOURCES]
    #define local $[egg]
    #define sourcedir $[SOURCE_DIR]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[sourcedir]/$[local]
	cd ./$[sourcedir] && $[INSTALL]

  #end egg
#end install_egg


// Bam file installation.
#forscopes install_egg
  #foreach egg $[SOURCES] $[UNPAL_SOURCE]
    #define local $[egg:%.egg=%.bam]
    #define sourcedir $[bam_dir]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[sourcedir]/$[local]
	cd ./$[sourcedir] && $[INSTALL]

  #end egg
#end install_egg

#end Makefile




//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], models_toplevel]
//////////////////////////////////////////////////////////////////////

// This is the toplevel directory for a models tree, e.g. $TTMODELS.
// Here we build the root makefile.

#map subdirs
// Iterate through all of our known source files.  Each models type
// file gets its corresponding Makefile listed here.
#forscopes */
#if $[eq $[DIR_TYPE], models]
#if $[build_directory]
  #addmap subdirs $[DIRNAME]
#endif
#endif
#end */

#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

all : $[subdirs]
egg : $[subdirs:%=egg-%]
pal : $[subdirs:%=pal-%]
bam : $[subdirs:%=bam-%]
clean-bam : $[subdirs:%=clean-bam-%]
clean-pal : $[subdirs:%=clean-pal-%]
clean : $[subdirs:%=clean-%]
cleanall : $[subdirs:%=cleanall-%]
install-egg : $[subdirs:%=install-egg-%]
install-bam : $[subdirs:%=install-bam-%]
install : $[subdirs:%=install-%]
uninstall : $[subdirs:%=uninstall-%]

optimize-palettes : regen-palettes pal

regen-palettes :
	egg-palettize-new -C -fRt $[texattrib_file]

// Somehow, something in the cttools confuses some shells, so that
// when we are attached, 'cd foo' doesn't work, but 'cd ./foo' does.
// Weird.  We get around this by putting a ./ in front of each cd
// target below.

#formap dirname subdirs
$[dirname] : $[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]
	cd ./$[PATH] && $(MAKE) all
#end dirname

#formap dirname subdirs
egg-$[dirname] :
	cd ./$[PATH] && $(MAKE) egg
#end dirname

#formap dirname subdirs
bam-$[dirname] :
	cd ./$[PATH] && $(MAKE) bam
#end dirname

#formap dirname subdirs
pal-$[dirname] :
	cd ./$[PATH] && $(MAKE) pal
#end dirname

#formap dirname subdirs
clean-bam-$[dirname] :
	cd ./$[PATH] && $(MAKE) clean-bam
#end dirname

#formap dirname subdirs
clean-pal-$[dirname] :
	cd ./$[PATH] && $(MAKE) clean-pal
#end dirname

#formap dirname subdirs
clean-$[dirname] :
	cd ./$[PATH] && $(MAKE) clean
#end dirname

#formap dirname subdirs
cleanall-$[dirname] :
	cd ./$[PATH] && $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-egg-$[dirname] :
	cd ./$[PATH] && $(MAKE) install-egg
#end dirname

#formap dirname subdirs
install-bam-$[dirname] :
	cd ./$[PATH] && $(MAKE) install-bam
#end dirname

#formap dirname subdirs
install-$[dirname] : 
	cd ./$[PATH] && $(MAKE) install
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
	cd ./$[PATH] && $(MAKE) uninstall
#end dirname

#end Makefile



//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE
