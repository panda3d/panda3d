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
#defer phase_prefix $[if $[PHASE],phase_$[PHASE]/]

#defer install_model_dir $[install_dir]/$[phase_prefix]$[INSTALL_TO]

#define build_eggs $[sort $[SOURCES(flt_egg):%.flt=%.egg]]
#define install_eggs $[sort $[SOURCES(install_egg)]]

#define install_egg_dirs $[sort $[forscopes install_egg,$[install_model_dir]]]
#define installed_eggs $[sort $[forscopes install_egg,$[SOURCES:%=$[install_model_dir]/%]]]
#define installed_bams $[sort $[forscopes install_egg,$[SOURCES:%.egg=$[install_model_dir]/%.bam]]]

#define pal_egg_targets $[install_eggs:%=$[pal_egg_dir]/%]
#define bam_targets $[install_eggs:%.egg=$[bam_dir]/%.bam]

#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#define all_targets \
    Makefile \
    $[texattrib_dir] \
    egg bam
all : $[all_targets]

egg : $[build_eggs]

pal : egg $[if $[pal_egg_targets],$[pal_egg_dir]] $[pal_egg_targets]

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
	rm -f $[build_eggs]
#endif

// We need a rule for each directory we might need to make.  This
// loops through the full set of directories and creates a rule to
// make each one, as needed.
#foreach directory $[sort \
    $[if $[pal_egg_targets],$[pal_egg_dir]] \
    $[if $[bam_targets],$[bam_dir]] \
    $[texattrib_dir] \
    $[install_egg_dirs] \
    ]
$[directory] :
	@test -d $[directory] || echo mkdir -p $[directory]
	@test -d $[directory] || mkdir -p $[directory]
#end directory


// Egg file generation.
#forscopes flt_egg
#foreach flt $[SOURCES]
#define target $[flt:%.flt=%.egg]
#define source $[flt]
$[target] : $[source]
	flt2egg -no -rt -o $[target] $[source]

#end flt
#end flt_egg


// Palettization rules.
#forscopes install_egg
#foreach egg $[SOURCES]
#define pt $[egg:%.egg=%.pt]
#define source $[egg]
#define target $[pal_egg_dir]/$[source]
$[target] : $[source] $[pt] $[build_eggs]
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
#end install_egg


// Bam file installation.
#forscopes install_egg
#foreach egg $[SOURCES]
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
