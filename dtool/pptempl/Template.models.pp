//
// Template.models.pp
//
// This file defines the Makefiles that will be built to generate
// models (egg, bam models computed from flt, soft, alias,
// etc. sources).  Unlike the other Template files, this is not based
// directly on the BUILD_TYPE, but is specifically included when a
// directory specifies a DIR_TYPE of "models".  It uses some
// Unix-specific conventions (like forward slashes as a directory
// separator), so it requires either a Unix platform or a Cygwin
// environment.
//

#if $[< $[PPREMAKE_VERSION],0.57]
  #error You need at least ppremake version 0.58 to build models.
#endif

// Search for the texattrib dir definition.  This will be in the
// models_topdir directory.
#define texattrib_dir $[dir_type $[TEXATTRIB_DIR],models_toplevel]

// Prefix $[TOPDIR].  If it wasn't defined, make a default.
#if $[texattrib_dir]
  #define texattrib_dir $[TOPDIR]/$[texattrib_dir]
#else
  #define texattrib_dir $[TOPDIR]/src/maps
#endif

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

#defer source_prefix $[SOURCE_DIR:%=%/]

#define build_models \
   $[SOURCES(flt_egg):%.flt=%.egg] \
   $[patsubst %.lwo %.LWO,%.egg,$[SOURCES(lwo_egg)]] \
   $[patsubst %.ma %.mb,%.egg,$[SOURCES(maya_egg)]] \
   $[forscopes soft_char_egg,$[POLY_MODEL:%=$[EGG_PREFIX]%.egg] $[NURBS_MODEL:%=$[EGG_PREFIX]%.egg]] \
   $[forscopes maya_char_egg,$[POLY_MODEL:%=$[EGG_PREFIX]%.egg] $[NURBS_MODEL:%=$[EGG_PREFIX]%.egg]]

#define build_anims \
   $[forscopes soft_char_egg,$[ANIMS:%=$[EGG_PREFIX]%$[CHAN_SUFFIX].egg]] \
   $[forscopes maya_char_egg,$[ANIMS:%=$[EGG_PREFIX]%$[CHAN_SUFFIX].egg]]

#define build_eggs $[sort $[build_models] $[build_anims]]
#define install_eggs $[sort $[notdir $[SOURCES(install_egg)] $[UNPAL_SOURCES(install_egg)] $[UNPAL_SOURCES_NC(install_egg)]]]
#define install_other $[sort $[SOURCES(install_audio install_dna install_icons install_misc)]]

#define install_egg_dirs $[sort $[forscopes install_egg,$[install_model_dir]]]
#define installed_eggs $[sort $[forscopes install_egg,$[patsubst %,$[install_model_dir]/%,$[notdir $[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]]]]
#define installed_bams $[sort $[forscopes install_egg,$[patsubst %.egg,$[install_model_dir]/%.bam,$[notdir $[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]]]]

#define install_other_dirs $[sort $[forscopes install_audio install_dna install_icons install_misc,$[install_model_dir]]]
#define installed_other $[sort $[forscopes install_audio install_dna install_icons install_misc,$[SOURCES:%=$[install_model_dir]/%]]]

#define pal_egg_targets $[sort $[patsubst %,$[pal_egg_dir]/%,$[notdir $[SOURCES(install_egg)]]]]
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
    $[forscopes install_egg,$[patsubst %,$[source_prefix]%,$[notdir $[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]]]
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

#define install_other_targets \
    $[install_other_dirs] \
    $[installed_other]
install-other : $[install_other_targets]

install : all install-other install-bam
uninstall : uninstall-other uninstall-bam uninstall-egg

clean-bam :
#if $[bam_targets]
$[TAB]rm -rf $[bam_dir]
#endif

clean-pal : clean-bam
#if $[pal_egg_targets]
$[TAB]rm -rf $[pal_egg_dir]
#endif

clean : clean-pal
#if $[build_eggs]
$[TAB]rm -f $[build_eggs] $[install_eggs:%.egg=%.pt]
#endif
#if $[POLY_MODEL(soft_char_egg)] $[NURBS_MODEL(soft_char_egg)]
$[TAB]rm -rf $[soft_maps_dir]
#endif
#if $[filter_dirs]
$[TAB]rm -rf $[filter_dirs]
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
    $[install_other_dirs] \
    ]
$[directory] :
$[TAB]@test -d $[directory] || echo mkdir -p $[directory]
$[TAB]@test -d $[directory] || mkdir -p $[directory]

// Sometimes we need a target to depend on the directory existing, without
// being fooled by the directory's modification times.  We use this
// phony timestamp file to achieve that.
$[directory]/stamp :
$[TAB]@test -d $[directory] || echo mkdir -p $[directory]
$[TAB]@test -d $[directory] || mkdir -p $[directory]
$[TAB]@touch $[directory]/stamp
#end directory

// Decompressing compressed files.
#forscopes gz
  #foreach gz $[SOURCES]
    #define target $[gz:%.gz=%]
    #define source $[gz]
$[target] : $[source]
$[TAB]rm -f $[target]
$[TAB]gunzip $[GUNZIP_OPTS] < $[source] > $[target]

  #end gz
#end gz

// Egg file generation from Flt files.
#forscopes flt_egg
  #foreach flt $[SOURCES]
    #define target $[flt:%.flt=%.egg]
    #define source $[flt]
$[target] : $[source]
$[TAB]flt2egg $[FLT2EGG_OPTS] -o $[target] $[source]

  #end flt
#end flt_egg

// Egg file generation from Lightwave files.
#forscopes lwo_egg
  #foreach lwo $[SOURCES]
    #define target $[patsubst %.lwo %.LWO,%.egg,$[lwo]]
    #define source $[lwo]
$[target] : $[source]
$[TAB]lwo2egg $[LWO2EGG_OPTS] -o $[target] $[source]

  #end lwo
#end lwo_egg

// Egg file generation from Maya files (for unanimated models).
#forscopes maya_egg
  #foreach maya $[SOURCES]
    #define target $[patsubst %.ma %.mb,%.egg,$[maya]]
    #define source $[maya]
$[target] : $[source]
$[TAB]maya2egg $[MAYA2EGG_OPTS] -o $[target] $[source]

  #end maya
#end maya_egg

// Egg character model generation from Maya files.
#forscopes maya_char_egg
  #if $[POLY_MODEL]
    #define target $[EGG_PREFIX]$[POLY_MODEL].egg
    #define source $[MAYA_PREFIX]$[POLY_MODEL].mb
$[target] : $[source]
$[TAB]maya2egg $[MAYA2EGG_OPTS] -p -a model -cn "$[CHAR_NAME]" -o $[target] $[source]
  #endif

#end maya_char_egg

// Egg animation generation from Maya files.
#forscopes maya_char_egg
  #foreach anim $[ANIMS]
    #define target $[EGG_PREFIX]$[anim]$[CHAN_SUFFIX].egg
    #define source $[MAYA_PREFIX]$[anim].mb
    #define begin 0
    #define end
    #if $[$[anim]_frames]
      #set begin $[word 1,$[$[anim]_frames]]
      #set end $[word 2,$[$[anim]_frames]]
    #endif
$[target] : $[source]
$[TAB]maya2egg $[MAYA2EGG_OPTS] -a chan -cn "$[CHAR_NAME]" -o $[target] -sf $[begin] $[if $[end],-ef $[end]] $[source]
  #end anim
#end maya_char_egg

// Egg character model generation from Soft databases.
#forscopes soft_char_egg
  #if $[POLY_MODEL]
    #define target $[EGG_PREFIX]$[POLY_MODEL].egg
    #define scene $[SCENE_PREFIX]$[MODEL].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
$[target] : $[source]
$[TAB]soft2egg $[SOFT2EGG_OPTS] -p -M $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] -t $[soft_maps_dir]
  #endif
  #if $[NURBS_MODEL]
    #define target $[EGG_PREFIX]$[NURBS_MODEL].egg
    #define scene $[SCENE_PREFIX]$[MODEL].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
$[target] : $[source]
$[TAB]soft2egg $[SOFT2EGG_OPTS] -n -M $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] -t $[soft_maps_dir]
  #endif

#end soft_char_egg

// Egg animation generation from Soft database.
#forscopes soft_char_egg
  #foreach anim $[ANIMS]
    #define target $[EGG_PREFIX]$[anim]$[CHAN_SUFFIX].egg
    #define scene $[SCENE_PREFIX]$[anim].1-0.dsc
    #define source $[DATABASE]/SCENES/$[scene]
    #define begin 1
    #define end
    #if $[$[anim]_frames]
      #set begin $[word 1,$[$[anim]_frames]]
      #set end $[word 2,$[$[anim]_frames]]
    #endif
$[target] : $[source]
$[TAB]soft2egg $[SOFT2EGG_OPTS] -a -A $[target] -N $[CHAR_NAME] -d $[DATABASE] -s $[scene] $[begin:%=-b%] $[end:%=-e%]
  #end anim
#end soft_char_egg


// Generic egg filters.
#forscopes filter_egg
  #foreach egg $[SOURCES]
    #define source $[source_prefix]$[egg]
    #define target $[TARGET_DIR]/$[notdir $[egg]]
$[target] : $[source] $[pt] $[TARGET_DIR]/stamp
$[TAB]$[COMMAND]
  #end egg
#end filter_egg

// Character optimization.
#forscopes optchar_egg
  #define sources $[SOURCES:%=$[source_prefix]%]
  #define target $[TARGET_DIR]/$[notdir $[firstword $[SOURCES]]]

   // A bunch of rules to make each generated egg file depend on the
   // first one.
  #foreach egg $[notdir $[wordlist 2,9999,$[SOURCES]]]
$[TARGET_DIR]/$[egg] : $[target] $[TARGET_DIR]/stamp
$[TAB]touch $[TARGET_DIR]/$[egg]
  #end egg

   // And this is the actual optchar pass.
$[target] : $[sources] $[TARGET_DIR]/stamp
$[TAB]egg-optchar $[OPTCHAR_OPTS] -d $[TARGET_DIR] $[sources]
#end optchar_egg


// Palettization rules.
#forscopes install_egg
  #foreach egg $[SOURCES]
    #define pt $[egg:%.egg=$[source_prefix]%.pt]
    #define source $[source_prefix]$[egg]
    #define target $[pal_egg_dir]/$[notdir $[egg]]
$[target] : $[source] $[pt] $[pal_egg_dir]/stamp
    #if $[PHASE]
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dr $[install_dir] -dm $[install_dir]/%g/maps -ds $[install_dir]/shadow_pal -g phase_$[PHASE] -gdir phase_$[PHASE] -o $[target] $[source]
    #else
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dr $[install_dir] -dm $[install_dir]/%g/maps -ds $[install_dir]/shadow_pal -o $[target] $[source]
    #endif

$[pt] :
$[TAB]touch $[pt]

  #end egg
#end install_egg


// Bam file creation.
#forscopes install_egg
  #foreach egg $[SOURCES]
    #define source $[pal_egg_dir]/$[notdir $[egg]]
    #define target $[bam_dir]/$[notdir $[egg:%.egg=%.bam]]
$[target] : $[source] $[bam_dir]/stamp
$[TAB]egg2bam -pp $[install_dir] -ps rel -pd $[install_dir] $[EGG2BAM_OPTS] -o $[target] $[source]
  #end egg

  #foreach egg $[UNPAL_SOURCES]
    #define source $[source_prefix]$[egg]
    #define target $[bam_dir]/$[notdir $[egg:%.egg=%.bam]]
$[target] : $[source] $[bam_dir]/stamp
$[TAB]egg2bam $[EGG2BAM_OPTS] -o $[target] $[source]
  #end egg

  #foreach egg $[UNPAL_SOURCES_NC]
    #define source $[source_prefix]$[egg]
    #define target $[bam_dir]/$[notdir $[egg:%.egg=%.bam]]
$[target] : $[source] $[bam_dir]/stamp
$[TAB]egg2bam $[EGG2BAM_OPTS] -NC -o $[target] $[source]
  #end egg
#end install_egg


// Egg file installation.
#forscopes install_egg
  #foreach egg $[notdir $[SOURCES]]
    #define local $[egg]
    #define sourcedir $[pal_egg_dir]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[sourcedir]/$[local]
//      cd ./$[sourcedir] && $[INSTALL]
$[TAB]rm -f $[dest]/$[local]
$[TAB]cp $[sourcedir]/$[local] $[dest]

  #end egg
  #foreach egg $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]
    #define local $[egg]
    #define dest $[install_model_dir]
$[dest]/$[notdir $[local]] : $[source_prefix]$[local]
//      $[INSTALL]
$[TAB]rm -f $[dest]/$[notdir $[local]]
$[TAB]cp $[source_prefix]$[local] $[dest]

  #end egg
#end install_egg

// Egg file uninstallation.
uninstall-egg :
#forscopes install_egg
  #define files $[patsubst %,$[install_model_dir]/%,$[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]
  #if $[files]
$[TAB]rm -f $[files]
  #endif
#end install_egg


// Bam file installation.
#forscopes install_egg
  #foreach egg $[notdir $[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]
    #define local $[egg:%.egg=%.bam]
    #define sourcedir $[bam_dir]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[sourcedir]/$[local]
//      cd ./$[sourcedir] && $[INSTALL]
$[TAB]rm -f $[dest]/$[local]
$[TAB]cp $[sourcedir]/$[local] $[dest]

  #end egg
#end install_egg

// Bam file uninstallation.
uninstall-bam :
#forscopes install_egg
  #define files $[patsubst %.egg,$[install_model_dir]/%.bam,$[notdir $[SOURCES] $[UNPAL_SOURCES] $[UNPAL_SOURCES_NC]]]
  #if $[files]
$[TAB]rm -f $[files]
  #endif
#end install_egg



// Miscellaneous file installation.
#forscopes install_audio install_dna install_icons install_misc
  #foreach file $[SOURCES]
    #define local $[file]
    #define dest $[install_model_dir]
$[dest]/$[local] : $[local]
//      $[INSTALL]
$[TAB]rm -f $[dest]/$[local]
$[TAB]cp $[local] $[dest]

  #end file
#end install_audio install_dna install_icons install_misc

// Miscellaneous file uninstallation.
uninstall-other:
#forscopes install_audio install_dna install_icons install_misc
  #define files $[patsubst %,$[install_model_dir]/%,$[SOURCES]]
  #if $[files]
$[TAB]rm -f $[files]
  #endif
#end install_audio install_dna install_icons install_misc


#end Makefile




//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], models_group]
//////////////////////////////////////////////////////////////////////

// This is a group directory: a directory above a collection of source
// directories, e.g. $DTOOL/src.  We don't need to output anything in
// this directory.



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

all : egg pal repal $[subdirs]
egg : $[subdirs:%=egg-%]
pal : $[subdirs:%=pal-%]
bam : $[subdirs:%=bam-%]
clean-bam : $[subdirs:%=clean-bam-%]
clean-pal : $[subdirs:%=clean-pal-%]
clean : $[subdirs:%=clean-%]
cleanall : $[subdirs:%=cleanall-%]
install-egg : egg pal repal $[subdirs:%=install-egg-%]
install-bam : egg pal repal $[subdirs:%=install-bam-%]
install-other : $[subdirs:%=install-other-%]
install : egg pal repal $[subdirs:%=install-%]
uninstall-egg : $[subdirs:%=uninstall-egg-%]
uninstall-bam : $[subdirs:%=uninstall-bam-%]
uninstall-other : $[subdirs:%=uninstall-other-%]
uninstall : $[subdirs:%=uninstall-%]

#
# opt-pal : reorder and resize the palettes to be as optimal as
# possible.  This forces a rebuild of all the egg files.
#
opt-pal : pal do-opt-pal install
optimize-palettes : opt-pal

do-opt-pal :
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dm $[install_dir]/%g/maps -opt -egg

#
# repal : reexamine the textures.txa file and do whatever needs to be
# done to bring everything up to sync with it.  Also make sure all egg
# files are up-to-date.
#
repal :
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dm $[install_dir]/%g/maps -all -egg

re-pal : repal

#
# fix-pal : something has gone wrong with the palettes; rebuild all
# palette images to fix it.
#
fix-pal :
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dm $[install_dir]/%g/maps -redo -all -egg

#
# undo-pal : blow away all the palettization information and start fresh.
#
undo-pal : clean-pal
$[TAB]rm -f $[texattrib_file:%.txa=%.boo]

#
# pi : report the palettization information to standard output for the
# user's perusal.
#
pi :
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dm $[install_dir]/%g/maps -pi

#
# pal-stats : report palettization statistics to standard output for the
# user's perusal.
#
pal-stats :
$[TAB]egg-palettize $[PALETTIZE_OPTS] -a $[texattrib_file] -dm $[install_dir]/%g/maps -s
stats-pal : pal-stats

// Somehow, something in the cttools confuses some shells, so that
// when we are attached, 'cd foo' doesn't work, but 'cd ./foo' does.
// Weird.  We get around this by putting a ./ in front of each cd
// target below.

#formap dirname subdirs
$[dirname] : $[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]
$[TAB]cd ./$[RELDIR] && $(MAKE) all
#end dirname

#formap dirname subdirs
egg-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) egg
#end dirname

#formap dirname subdirs
bam-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) bam
#end dirname

#formap dirname subdirs
pal-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) pal
#end dirname

#formap dirname subdirs
clean-bam-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) clean-bam
#end dirname

#formap dirname subdirs
clean-pal-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) clean-pal
#end dirname

#formap dirname subdirs
clean-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) clean
#end dirname

#formap dirname subdirs
cleanall-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-egg-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) install-egg
#end dirname

#formap dirname subdirs
install-bam-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) install-bam
#end dirname

#formap dirname subdirs
install-other-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) install-other
#end dirname

#formap dirname subdirs
install-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) install
#end dirname

#formap dirname subdirs
uninstall-egg-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) uninstall-egg
#end dirname

#formap dirname subdirs
uninstall-bam-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) uninstall-bam
#end dirname

#formap dirname subdirs
uninstall-other-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) uninstall-other
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
$[TAB]cd ./$[RELDIR] && $(MAKE) uninstall
#end dirname

#end Makefile



//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE
