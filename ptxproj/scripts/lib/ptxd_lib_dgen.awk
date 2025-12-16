#!/usr/bin/gawk -f
#
# Copyright (C) 2006, 2007, 2008 by the PTXdist project
#               2009 by Marc Kleine-Budde <mkl@pengutronix.de>
#               2010 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

BEGIN {
	FS = "[[:space:]]*[+]=[[:space:]]*|=";

	MAP_ALL			= ENVIRON["PTX_MAP_ALL"];
	MAP_ALL_MAKE		= ENVIRON["PTX_MAP_ALL_MAKE"];
	MAP_DEPS		= ENVIRON["PTX_MAP_DEPS"];
	DGEN_DEPS_PRE		= ENVIRON["PTX_DGEN_DEPS_PRE"];
	DGEN_DEPS_POST		= ENVIRON["PTX_DGEN_DEPS_POST"];
	DGEN_RULESFILES_MAKE	= ENVIRON["PTX_DGEN_RULESFILES_MAKE"];
	OPTIMIZE_IO		= ENVIRON["PTXDIST_OPTIMIZE_IO"]
	PTXDIST_TEMPDIR		= ENVIRON["PTXDIST_TEMPDIR"];
	PARALLEL		= ENVIRON["PTXDIST_PARALLELMFLAGS_EXTERN"]
	DIRTY			= ENVIRON["PTXDIST_DIRTY"];
	PTXDIST_OLD_MAKE	= ENVIRON["PTXDIST_OLD_MAKE"];
	PTXDIST_GEN_ALL		= ENVIRON["PTXDIST_GEN_ALL"];
	DEP			= DIRTY == "true" ? "|" : ""
	PTXDIST_HASHLIST	= PTXDIST_TEMPDIR "/pkghash.list"
	CHECK_LICENSES		= 0
	GENERATE_REPORTS	= 0
}

#
# called when a new file is opened
#
FNR == 1 {
	#
	# remember ARGIND of current file
	#
	move_argc = ARGIND;

	#
	# "include" all makefile files which are _not_ pkgs explicitly.
	# the make files which are actually pkgs will be "include"d
	# in the END rule
	#
	if (old_filename && old_filename ~ /.+\/rules\/.+\.make/ && !is_pkg)
		print "include "old_filename				> DGEN_RULESFILES_MAKE;

	# remember the current opened file
	old_filename = FILENAME;
	lineno = 0;

	# will be set later, if makefile belongs to a pkg
	is_pkg = "";

	if (FILENAME ~ /.+\/rules\/.+\..+\.make/) {
		this_pkg = gensub(/.+\/rules\/(.+)\..+\.make/, "\\1", 1, FILENAME)
		this_aux = gensub(/.+\/rules\/.+\.(.+)\.make/, "\\1", 1, FILENAME)
		is_pkg = this_pkg
		pkg_to_aux_makefile[this_pkg][this_aux] = FILENAME;
	}
}

/^/ {
	lineno += 1;
}

#
# skip comments and empty lines
#
/^#|^$/ {
	next;
}



#
# handle "include <MAKEFILE>" lines:
#
# add "<MAKEFILE>" to argv array after the file that includes
# "<MAKEFILE>"
#
$0 ~ /^include[[:space:]]+\/.*\.make$/ {
	move_argc++;

	for (i = ARGC; i > move_argc; i--)
		ARGV[i] = ARGV[i - 1];

	ARGV[i] = gensub(/^include[[:space:]]+/, "", "g");
	ARGC++;

	next;
}


#
# warn user if an image, host, or cross package contains a targetinstall rule
# which will not be executed
#
$1 ~ /^\$\(STATEDIR\)\/(image-.*|host-.*|cross-.*)\.targetinstall(.post)?:/ {
	match($0, /\$\(STATEDIR\)\/((image-.*|host-.*|cross-.*)\.targetinstall(.post)?):/, m);
	print "\nError in " old_filename " line " lineno ":\n" \
		"  '" m[1] "' stage in host/cross/image rule will be ignored.\n" \
		"  See section 'Rule File Layout' in the PTXdist reference for more info:\n" \
		"  https://www.ptxdist.org/doc/ref_manual.html#rule-file-layout"
	exit 1;
}


#
# parse "PACKAGES-$(PTXCONF_PKG) += pkg" lines, i.e. rules-files from
# rules/*.make. Setup mapping between upper and lower case pkg names
#
# out:
# PKG_to_pkg		array that maps from upper case to lower case pkg name
# pkg_to_PKG		array that maps from lower case to upper case pkg name
# PKG_to_makefile	array that maps from upper case pkg name to makefile name
#
$1 ~ /^[A-Z_]*PACKAGES-/ {
	this_PKG = gensub(/^[A-Z_]*PACKAGES-\$\(PTXCONF_([^\)]*)\)/, "\\1", "g", $1);
	this_PKG = gensub(/^[A-Z0-9_]*-\$\(PTXCONF_([^\)]*)\)/, "\\1", "g", this_PKG);

	is_pkg = this_pkg = gensub(/^[[:space:]]*\<(.*)\>[[:space:]]*$/,"\\1",1, $2);
	if (this_pkg ~ /[A-Z]+/) {
		print \
			"\n" \
			"error: upper case chars in package '" this_pkg "' detected, please fix!\n" \
			"\n\n"
		exit 1
	}

	PKG_to_pkg[this_PKG] = this_pkg;
	pkg_to_PKG[this_pkg] = this_PKG;
	# make sure each file is only included once
	if (FILENAME) {
		PKG_to_makefile[this_PKG] = FILENAME;
		FILENAME = "";
	}
	if (PTXDIST_GEN_ALL == "1")
		active_PKG_to_pkg[this_PKG] = this_pkg;

	print "PTX_MAP_TO_package_" this_PKG "=\"" this_pkg "\""	> MAP_ALL;
	print "PTX_MAP_TO_package_" this_PKG "="   this_pkg 		> MAP_ALL_MAKE;
	print "PTX_MAP_TO_PACKAGE_" this_pkg "="   this_PKG		> MAP_ALL_MAKE;

	next;
}

#
# parses "PTX_MAP_[BR]_DEP_PKG=FOO" lines, which are the raw dependencies
# generated from kconfig. these deps usually contain non pkg symbols,
# these are filtered out here.
#
$1 ~ /^PTX_MAP_._DEP/ {
	this_PKG = gensub(/PTX_MAP_._DEP_/, "", "g", $1);
	dep_type = gensub(/PTX_MAP_(.)_DEP_.*/, "\\1", "g", $1);

	if ($2 ~ /\<VIRTUAL\>/)
		virtual_pkg[this_PKG] = 1
	else if (!(this_PKG in PKG_to_pkg)) # no pkg
		next;

	n = split($2, this_DEP_array, ":");

	# no deps
	if (n == 0)
		next;

	this_PKG_DEP = ""
	for (i = 1; i <= n; i++) {
		this_DEP = this_DEP_array[i];

		if (this_DEP ~ /^VIRTUAL$/)
			continue;

		if (this_DEP ~ /^BASE$/) {
			base_PKG_to_pkg[this_PKG] = PKG_to_pkg[this_PKG];

			if (dep_type == "R")
				PKG_to_R_DEP["BASE"] = PKG_to_R_DEP["BASE"] " " this_PKG;
			else
				PKG_to_B_DEP["BASE"] = PKG_to_B_DEP["BASE"] " " this_PKG;

			continue;
		}

		this_PKG_DEP = this_PKG_DEP " " this_DEP;
	}

	# no deps to pkgs
	if (this_PKG_DEP == "")
		next;

	if (dep_type == "R")
		PKG_to_R_DEP[this_PKG] = PKG_to_R_DEP[this_PKG] this_PKG_DEP;
	else
		PKG_to_B_DEP[this_PKG] = PKG_to_B_DEP[this_PKG] this_PKG_DEP;

	next;
}

$1 ~ /^PTX_MAP_._SOURCE/ {
	this_PKG = gensub(/PTX_MAP_._SOURCE_/, "", "g", $1);
	if (this_PKG in PKG_to_pkg) {
		if (index($2, "/") != 1)
			tmp = PTXDIST_TEMPDIR "/kconfig/" $2;
		else
			tmp = $2
		PKG_to_infile[this_PKG] = tmp;
	}
	next;
}

$1 ~ /^PTX_MAP_._SYMBOLS/ {
	name = gensub(/PTX_MAP_._SYMBOLS_/, "", "g", $1);
	n = split($2, symbol_array, ":");

	for (i = 1; i <= n; i++)
		# range limits are unknown symbols in kconfig. Drop them here
		if (name == "ALL" || !(symbol_array[i] ~ /^([0-9]*|0x[0-9a-f]*)$/))
			config_symbols[name] = config_symbols[name] " " symbol_array[i]

	next;
}

function write_symbols(name, symbols) {
	symbol_file = PTXDIST_TEMPDIR "/SYMBOLS_" name
	n = split(symbols, symbol_array, " ")
	asort(symbol_array)
	symbols = ""
	last = ""
	for (i = 1; i <= n; i++) {
		if (last == symbol_array[i])
			continue
		print symbol_array[i]				> symbol_file
		last = symbol_array[i]
	}
	close(symbol_file)
}

#
# parse the ptx- and platformconfig
# record yes and module packages
#
$1 ~ /^PTXCONF_/ {
	if (old_filename ~ /.+\/rules\/.+\.make/)
		next;

	this_PKG = gensub(/^PTXCONF_/, "", "g", $1);

	if ($2 ~ /^[ym]$/ && this_PKG in PKG_to_pkg)
		active_PKG_to_pkg[this_PKG] = PKG_to_pkg[this_PKG];

	if (this_PKG == "PROJECT_CHECK_LICENSES")
		CHECK_LICENSES = 1;

	if (this_PKG == "PROJECT_GENERATE_REPORTS")
		GENERATE_REPORTS = 1;

	do {
		if (this_PKG in PKG_to_pkg || this_PKG in virtual_pkg) {
			PKG_HASHFILE = PTXDIST_TEMPDIR "/pkghash-" this_PKG;
			if (PTXDIST_OLD_MAKE) {
				if (PKG_HASHFILE != last_PKG_HASHFILE) {
					close(last_PKG_HASHFILE);
					last_PKG_HASHFILE = PKG_HASHFILE;
				}
			}
			if (!($0 in allsym)) {
				if (PTXDIST_OLD_MAKE)
					print $0 >> PKG_HASHFILE;
				else {
					print "ifdef PTXDIST_SETUP_ONCE"			> DGEN_DEPS_POST;
					print "$(file >>" PKG_HASHFILE "," $1 "=$(" $1 "))"	> DGEN_DEPS_POST;
					print "endif"						> DGEN_DEPS_POST;
				}
			}
			break;
		}
	} while (sub(/_+[^_]+$/, "", this_PKG));

	allsym[$0] = 1

	next;
}

function pkg_aux_makefiles(this_pkg, l, ll, i) {
	delete l;
	if (this_pkg in pkg_to_aux_makefile) {
		asorti(pkg_to_aux_makefile[this_pkg], ll);
		for (i in ll) {
			l[i] = pkg_to_aux_makefile[this_pkg][ll[i]]
		}
	}
}

function write_vars_all(this_PKG) {
	#
	# include this rules file
	#
	if (this_PKG in PKG_to_makefile) {
		print "include " PKG_to_makefile[this_PKG]			> DGEN_RULESFILES_MAKE;
		pkg_aux_makefiles(PKG_to_pkg[this_PKG], makefiles);
		for (i in makefiles) {
			print "include " makefiles[i]				> DGEN_RULESFILES_MAKE;
			print this_PKG "_EXTRA_MAKEFILES += " makefiles[i]	> DGEN_DEPS_PRE;
		}
		print this_PKG "_MAKEFILE = " PKG_to_makefile[this_PKG]		> DGEN_DEPS_PRE;
	}
	if (this_PKG in PKG_to_infile) {
		print this_PKG "_INFILE = " PKG_to_infile[this_PKG]		> DGEN_DEPS_PRE;
	}
}

function pkg_dep(this_PKG, dep_type) {
	if (dep_type == "R")
		return PKG_to_R_DEP[this_PKG];
	else
		return PKG_to_B_DEP[this_PKG];
}

function write_maps(this_PKG, dep_type) {
	this_PKG_DEP = pkg_dep(this_PKG, dep_type);
	if (this_PKG_DEP == "")
		return;

	n = split(this_PKG_DEP, this_DEP_array, " ");
	for (i = 1; i <= n; i++) {
		if (this_DEP_array[i] in virtual_pkg) {
			virtual_PKG_DEP = pkg_dep(this_DEP_array[i], dep_type);
			this_PKG_DEP = this_PKG_DEP " " virtual_PKG_DEP
		}
	}
	n = split(this_PKG_DEP, this_DEP_array, " ");
	asort(this_DEP_array);
	this_PKG_dep = ""
	this_PKG_DEP = ""
	last = ""
	for (i = 1; i <= n; i++) {
		if (last ==  this_DEP_array[i])
			continue
		if (this_DEP_array[i] in virtual_pkg) {
			print "RULES: " this_PKG " " PTXDIST_TEMPDIR "/pkghash-" this_DEP_array[i] > PTXDIST_HASHLIST
			continue
		}
		if (!(this_DEP_array[i] in PKG_to_pkg))
			continue
		this_PKG_DEP = this_PKG_DEP " " this_DEP_array[i];
		this_PKG_dep = this_PKG_dep " " PKG_to_pkg[this_DEP_array[i]];
		last = this_DEP_array[i]
	}

	if (dep_type == "R")
		 PKG_to_R_DEP[this_PKG]= this_PKG_DEP;
	else
		 PKG_to_B_DEP[this_PKG]= this_PKG_DEP;

	if (this_PKG_DEP == "")
		return;

	print "PTX_MAP_" dep_type "_DEP_" this_PKG "=" this_PKG_DEP	> MAP_DEPS;
	print "PTX_MAP_" dep_type "_dep_" this_PKG "=" this_PKG_dep	> MAP_DEPS;
	print "PTX_MAP_" dep_type "_dep_" this_PKG "=" this_PKG_dep	> MAP_ALL_MAKE;
}

function pkg_all_deps(this_PKG, dep_type, n, i, this_PKG_dep, this_DEP_array) {
	if (this_PKG in PKG_to_dep_all)
		return PKG_to_dep_all[this_PKG];

	this_PKG_dep = ""
	n = split(pkg_dep(this_PKG, dep_type), this_DEP_array, " ");
	for (i = 1; i <= n; i++)
		this_PKG_dep = this_PKG_dep " " PKG_to_pkg[this_DEP_array[i]] " " pkg_all_deps(this_DEP_array[i], dep_type)
	n = split(this_PKG_dep, this_DEP_array, " ");
	asort(this_DEP_array);
	this_PKG_dep = ""
	last = ""
	for (i = 1; i <= n; i++) {
		if (last == this_DEP_array[i])
			continue
		this_PKG_dep = this_PKG_dep " " this_DEP_array[i];
		last = this_DEP_array[i]
	}
	PKG_to_dep_all[this_PKG] = this_PKG_dep;
	return this_PKG_dep;
}

function write_all_deps(dep_type, PKG_to_dep_all) {
	for (this_PKG in PKG_to_pkg) {
		this_PKG_dep = pkg_all_deps(this_PKG, dep_type);
		if (this_PKG_dep == "")
			continue;
		print "PTX_MAP_" dep_type "_dep_all_" this_PKG "=" this_PKG_dep	> MAP_DEPS;
		print "PTX_MAP_" dep_type "_dep_all_" this_PKG "=" this_PKG_dep	> MAP_ALL_MAKE;
	}
}

function write_vars_pkg_all(this_PKG, this_pkg, prefix, dir_prefix) {
	#
	# post install hooks
	#
	stage = "install";
	print this_PKG "_HOOK_POST_" toupper(stage) \
		" := $(STATEDIR)/" this_pkg "." stage ".post"		> DGEN_DEPS_PRE;

	#
	# archive name for devel packages
	#
	this_devpkg = "$(" this_PKG ")-$(PTXCONF_ARCH_STRING)-$(" this_PKG "_CFGHASH)-dev.tar.gz"

	#
	# define ${PKG}_PKGDIR, ${PKG}_DEVPKG & ${PKG}_PARTS
	#
	print this_PKG "_PKGDIR = $(PKGDIR)/" prefix "$(" this_PKG ")"	> DGEN_DEPS_PRE;
	print this_PKG "_DEVPKG = " prefix this_devpkg			> DGEN_DEPS_PRE;
	print this_PKG "_PARTS = " this_PKG				> DGEN_DEPS_PRE;

	if (this_PKG ~ /^HOST_SYSTEM_PYTHON3_/) {
		target_PKG = gensub(/^HOST_SYSTEM_/, "", 1, this_PKG);
		PREFIX = "HOST_"
		dir_prefix = "system-"
		if (!(target_PKG in PKG_to_pkg))
			target_PKG = "HOST_" target_PKG
	} else {
		target_PKG = gensub(/^HOST_|^CROSS_/, "", 1, this_PKG);
		PREFIX = gensub(/^(HOST_|CROSS_).*/, "\\1", 1, this_PKG);
	}

	# define default ${PKG}, ${PKG}_SOURCE, ${PKG}_DIR
	if ((PREFIX != this_PKG) && (target_PKG in PKG_to_pkg)) {
		print this_PKG " = " dir_prefix "$(" target_PKG ")"	> DGEN_DEPS_PRE;
		print this_PKG "_VERSION = $(" target_PKG "_VERSION)"	> DGEN_DEPS_PRE;
		print this_PKG "_MD5 = $(" target_PKG "_MD5)"		> DGEN_DEPS_PRE;
		print this_PKG "_SOURCE = $(" target_PKG "_SOURCE)"	> DGEN_DEPS_PRE;
		print this_PKG "_URL = $(" target_PKG "_URL)"		> DGEN_DEPS_PRE;
		print this_PKG "_DIR = $(addprefix $(" PREFIX \
			"BUILDDIR)/,$(" this_PKG "))"			> DGEN_DEPS_PRE;
		print this_PKG "_SUBDIR = $(" target_PKG "_SUBDIR)"	> DGEN_DEPS_PRE;
		print this_PKG "_STRIP_LEVEL = $(" target_PKG \
			"_STRIP_LEVEL)"					> DGEN_DEPS_PRE;
		print this_PKG "_LICENSE = $(" target_PKG "_LICENSE)"	> DGEN_DEPS_PRE;
		print this_PKG "_LICENSE_FILES = $(" target_PKG \
			"_LICENSE_FILES)"				> DGEN_DEPS_PRE;
	}
}

function write_sources(this_PKG) {
	print "ifneq ($(" this_PKG "_SOURCES),)"								> DGEN_DEPS_POST;
	print this_PKG "_PARTS := $(" this_PKG "_PARTS) $(foreach source,$(" this_PKG "_SOURCES),$($(source)))"	> DGEN_DEPS_POST;
	print this_PKG "_SOURCES := $(" this_PKG "_SOURCE) $(" this_PKG "_SOURCES)"				> DGEN_DEPS_POST;
	print "else"												> DGEN_DEPS_POST;
	print this_PKG "_SOURCES += $(filter-out $(if $(" this_PKG "_DIR),$(" this_PKG "_DIR)%,)," \
			"$(foreach part,$(" this_PKG "_PARTS),$($(part)_SOURCE)))"				> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
}

function write_deps_pkg_all(this_PKG, this_pkg) {
	#
	# .get rule
	#
	print "$(STATEDIR)/" this_pkg ".get: | $(" this_PKG "_SOURCES)"	> DGEN_DEPS_POST;
	print "ifneq ($(call remove_quotes, $(PTXCONF_PROJECT_DEVMIRROR)),)" > DGEN_DEPS_POST;
	print "ifneq ($(strip $(" this_PKG "_DEVPKG)),NO)" > DGEN_DEPS_POST;
	print "ifneq ($(" this_PKG "_CFGHASH),)" > DGEN_DEPS_POST;
	print "getdev: $(call remove_quotes, $(PTXCONF_PROJECT_DEVPKGDIR))/$(" this_PKG "_DEVPKG)" > DGEN_DEPS_POST;
	print "endif" > DGEN_DEPS_POST;
	print "endif" > DGEN_DEPS_POST;
	print "endif" > DGEN_DEPS_POST;
}

function write_deps_pkg_active_cfghash(this_PKG, this_pkg) {
	this_pkg_prefix = gensub(/^(host-|cross-|image-|).*/, "\\1", 1, this_pkg)

	if (this_PKG in PKG_to_infile)
		print "RULES: " this_PKG " " PKG_to_infile[this_PKG]						> PTXDIST_HASHLIST;
	if (this_PKG in PKG_to_makefile)
		print "RULES: " this_PKG " " PKG_to_makefile[this_PKG]						> PTXDIST_HASHLIST;
	pkg_aux_makefiles(PKG_to_pkg[this_PKG], makefiles);
	for (i in makefiles)
		print "RULES: " this_PKG " " makefiles[i]							> PTXDIST_HASHLIST;

	target_PKG = gensub(/^HOST_|^CROSS_/, "", 1, this_PKG);
	if (prefix != "" && target_PKG in active_PKG_to_pkg)
		print "ifneq ($(" this_PKG "_SOURCE),$(" target_PKG "_SOURCE))"					> DGEN_DEPS_POST;
	print "$(foreach part,$(" this_PKG "_PARTS), $(eval " \
			"$(if $($(part)_SOURCE),$(eval $($(part)_SOURCE) := $(part)))))"			> DGEN_DEPS_POST;
	if (prefix != "" && target_PKG in active_PKG_to_pkg)
		print "endif"											> DGEN_DEPS_POST;

	print "ifneq ($(" this_PKG "),)"									> DGEN_DEPS_POST;
	print "ifneq ($(" this_PKG "_PATCHES),)"								> DGEN_DEPS_POST;
	print this_PKG "_PATCH_DIRS := $(call ptx/in-path-all,PTXDIST_PATH_PATCHES,$(" this_PKG "_PATCHES))"	> DGEN_DEPS_POST;
	print "else"												> DGEN_DEPS_POST;
	print this_PKG "_PATCH_DIRS := $(call ptx/in-path-all,PTXDIST_PATH_PATCHES,$(" this_PKG "))"		> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "ifeq ($(" this_PKG "_PATCH_DIRS),)"								> DGEN_DEPS_POST;
	print "undefine " this_PKG "_PATCH_DIRS"								> DGEN_DEPS_POST;
	print "else"												> DGEN_DEPS_POST;
	print this_PKG "_PATCH_DIR := $(firstword $(" this_PKG "_PATCH_DIRS))"					> DGEN_DEPS_POST;
	print "ifdef PTXDIST_SETUP_ONCE"									> DGEN_DEPS_POST;
	print "PTXDIST_HASHLIST_DATA += PATCHES: " this_PKG " $(" this_PKG "_PATCH_DIR)\\n"			> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "ifneq ($(" this_PKG "_SOURCE),)"									> DGEN_DEPS_POST;
	print "ifdef PTXDIST_SETUP_ONCE"									> DGEN_DEPS_POST;
	print "_tmp :=$(foreach part, $(" this_PKG "_PARTS),$($(part)_MD5) $(notdir $($(part)_SOURCE)))"	> DGEN_DEPS_POST;
	print "$(file >>" PTXDIST_TEMPDIR "/pkghash-" this_PKG "_EXTRACT,$(_tmp))"				> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "ifdef PTXDIST_SETUP_ONCE"									> DGEN_DEPS_POST;
	print this_PKG "_CONFIG := $(" this_PKG "_CONFIG)"							> DGEN_DEPS_POST;
	print "ifeq ($(" this_PKG "_CONFIG),)"									> DGEN_DEPS_POST;
	print "undefine " this_PKG "_CONFIG"									> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "ifneq ($(filter /%,$(" this_PKG "_CONFIG)),)"							> DGEN_DEPS_POST;
	print "ifneq ($(wildcard $(" this_PKG "_CONFIG)),)"							> DGEN_DEPS_POST;
	print "PTXDIST_HASHLIST_DATA += CONFIG: " this_PKG " $(" this_PKG "_CONFIG)\\n"				> DGEN_DEPS_POST;
	print "else"												> DGEN_DEPS_POST;
	# WAGO: update to PTXdist-2024.12.0
	# Since our kernel and bootloader configurations are located
	# in a different path, we need to disable some autogenerated dependencies.
	if (this_pkg == "kernel" || this_pkg == "barebox" || this_pkg == "barebox_mlo") {
		# NOP
	}
	else if (this_pkg_prefix != "image-")
		print "$(STATEDIR)/" this_pkg ".prepare: "            "$(" this_PKG "_CONFIG)"			> DGEN_DEPS_POST;
	else
		print "$(" this_PKG "_IMAGE): "                       "$(" this_PKG "_CONFIG)"			> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "ifneq ($(wildcard $(" this_PKG "_MESON_CROSS_FILE)),)"						> DGEN_DEPS_POST;
	print "PTXDIST_HASHLIST_DATA += CONFIG: " this_PKG " $(" this_PKG "_MESON_CROSS_FILE)\\n"		> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	if (this_pkg_prefix == "image-") {
		print "ifneq ($(" this_PKG "_PKGS),)"								> DGEN_DEPS_POST;
		print "$(file >>" PTXDIST_TEMPDIR "/pkghash-" this_PKG "," this_PKG "_PKGS = $(" this_PKG "_PKGS))" \
														> DGEN_DEPS_POST;
		print "endif"											> DGEN_DEPS_POST;
		print "ifneq ($(" this_PKG "_FILES),)"								> DGEN_DEPS_POST;
		print "$(file >>" PTXDIST_TEMPDIR "/pkghash-" this_PKG "," this_PKG "_FILES = $(" this_PKG "_FILES))" \
														> DGEN_DEPS_POST;
		print "endif"											> DGEN_DEPS_POST;
	}
	print "endif"												> DGEN_DEPS_POST;
}

function write_deps_all_active(this_PKG, this_pkg, prefix) {
	if (DIRTY != "true") {
		print "$(STATEDIR)/" this_pkg ".report: " \
						"$(STATEDIR)/" this_pkg ".$(" this_PKG "_CFGHASH).cfghash"	> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg ".fast-report: " \
						"$(STATEDIR)/" this_pkg ".$(" this_PKG "_CFGHASH).cfghash"	> DGEN_DEPS_POST;
	}
}

function write_deps_pkg_active(this_PKG, this_pkg, prefix) {
	#
	# default deps
	#
	if (DIRTY != "true") {
		print "ifeq ($(" this_PKG "_EXTRACT_CFGHASH),)"							> DGEN_DEPS_POST;
		print this_PKG "_EXTRACT_CFGHASH := 00000000000000000000000000000000"				> DGEN_DEPS_POST;
		print "endif"											> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg ".get: " \
					"$(STATEDIR)/" this_pkg ".$(" this_PKG "_EXTRACT_CFGHASH).srchash"	> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg ".extract: "            "$(STATEDIR)/" this_pkg ".get"		> DGEN_DEPS_POST;
	} else
		print "$(STATEDIR)/" this_pkg ".extract: | "          "$(STATEDIR)/" this_pkg ".get"		> DGEN_DEPS_POST;

	print "$(STATEDIR)/" this_pkg ".extract.post: "               "$(STATEDIR)/" this_pkg ".extract"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".cargosync: "                  "$(STATEDIR)/" this_pkg ".extract.post"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".prepare: "                    "$(STATEDIR)/" this_pkg ".extract.post"	> DGEN_DEPS_POST;
	if (DIRTY != "true") {
		print "$(STATEDIR)/" this_pkg ".prepare: " \
						"$(STATEDIR)/" this_pkg ".$(" this_PKG "_CFGHASH).cfghash"	> DGEN_DEPS_POST;
	}
	print "$(STATEDIR)/" this_pkg ".tags: "                       "$(STATEDIR)/" this_pkg ".prepare"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".compile: "                    "$(STATEDIR)/" this_pkg ".prepare"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".install: "                    "$(STATEDIR)/" this_pkg ".compile"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".install.pack: "               "$(STATEDIR)/" this_pkg ".install"	> DGEN_DEPS_POST;
	print "ifeq ($(strip $(wildcard $(PTXDIST_DEVPKG_PLATFORMDIR)/$(" this_PKG "_DEVPKG))),)"		> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".install.post: "               "$(STATEDIR)/" this_pkg ".install.pack"	> DGEN_DEPS_POST;
	print "else"												> DGEN_DEPS_POST;
	if (DIRTY != "true")
		print "$(STATEDIR)/" this_pkg ".install.unpack: " \
						"$(STATEDIR)/" this_pkg ".$(" this_PKG "_CFGHASH).cfghash"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".install.post: "               "$(STATEDIR)/" this_pkg ".install.unpack"	> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	if (prefix == "") {
		print "$(STATEDIR)/" this_pkg ".targetinstall: "      "$(STATEDIR)/" this_pkg ".install.post"	> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg ".targetinstall.post: " "$(STATEDIR)/" this_pkg ".targetinstall"	> DGEN_DEPS_POST;
	}
	print "$(STATEDIR)/" this_pkg ".report: "                     "$(STATEDIR)/" this_pkg ".extract"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".release: "                    "$(STATEDIR)/" this_pkg ".extract"	> DGEN_DEPS_POST;
	if (CHECK_LICENSES) {
		if (prefix == "")
			print "$(STATEDIR)/" this_pkg ".targetinstall.post: $(STATEDIR)/" this_pkg ".report"	> DGEN_DEPS_POST;
		else
			print "$(STATEDIR)/" this_pkg ".install.post: $(STATEDIR)/" this_pkg ".report"		> DGEN_DEPS_POST;
	}

	#
	# conditional dependencies
	#
	print "ifneq ($(" this_PKG "),)"						> DGEN_DEPS_POST;
	# on autogen script
	print "ifneq ($(call autogen_dep,$(" this_PKG ")),)"				> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".extract.post:" DEP " $(STATEDIR)/autogen-tools"	> DGEN_DEPS_POST;
	print "endif"									> DGEN_DEPS_POST;
	# on lndir
	print "ifneq ($(findstring lndir://,$(" this_PKG "_URL)),)"			> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".extract: $(STATEDIR)/host-lndir.install.post"	> DGEN_DEPS_POST;
	print "endif"									> DGEN_DEPS_POST;
	print "endif"									> DGEN_DEPS_POST;

	#
	# add dep to pkgs we depend on
	#
	this_PKG_DEPS = PKG_to_B_DEP[this_PKG];
	n = split(this_PKG_DEPS, this_DEP_array, " ");
	for (i = 1; i <= n; i++) {
		this_dep = PKG_to_pkg[this_DEP_array[i]]

		if (PARALLEL != "-j1" && OPTIMIZE_IO == "true")
			print "$(STATEDIR)/" this_pkg	".extract:| " "$(STATEDIR)/" this_dep ".install.post"	> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg	".extract.post:" DEP   " $(STATEDIR)/" this_dep ".install.post"	> DGEN_DEPS_POST;
		print "$(STATEDIR)/" this_pkg	".install.unpack:" DEP " $(STATEDIR)/" this_dep ".install.post"	> DGEN_DEPS_POST;

	}
	this_PKG_DEPS = PKG_to_R_DEP[this_PKG];
	n = split(this_PKG_DEPS, this_DEP_array, " ");
	for (i = 1; i <= n; i++) {
		this_dep = PKG_to_pkg[this_DEP_array[i]]

		#
		# only target packages have targetinstall rules
		#
		if (this_dep ~ /^host-|^cross-/)
			continue;

		print "$(STATEDIR)/" this_pkg ".targetinstall:" DEP " $(STATEDIR)/" this_dep ".targetinstall"	> DGEN_DEPS_POST;
	}
}

#
# add deps to virtual pkgs
#
function write_deps_pkg_active_virtual(this_PKG, this_pkg, prefix) {
	if (this_pkg ~ /^host-dummy-install-info$/)
		return;
	if (this_pkg ~ /^host-pkgconf$/)
		return;
	if (this_pkg ~ /^host-chrpath$/)
		return;
	if (this_pkg ~ /^host-system-/)
		return;

	if (prefix != "")
		virtual = "virtual-host-tools";
	else {
		if (this_PKG in base_PKG_to_pkg || this_pkg ~ /^base$/)
			virtual = "virtual-cross-tools";
		else
			virtual = "base";
	}
	if (PARALLEL != "-j1" && OPTIMIZE_IO == "true")
		print "$(STATEDIR)/" this_pkg ".extract:| "           "$(STATEDIR)/" virtual  ".install"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".extract.post:" DEP           " $(STATEDIR)/" virtual  ".install"	> DGEN_DEPS_POST;
	print "$(STATEDIR)/" this_pkg ".install.unpack:" DEP         " $(STATEDIR)/" virtual  ".install"	> DGEN_DEPS_POST;
}

function write_deps_pkg_active_image(this_PKG, this_pkg, prefix) {
	print "ifdef PTXCONF_IMAGE_INSTALL_FROM_IPKG_REPOSITORY"						> DGEN_DEPS_POST;
	print "ifneq ($(" this_PKG "_PKGS),)"									> DGEN_DEPS_POST;
	print "$(" this_PKG "_IMAGE): $(STATEDIR)/ipkg-push"							> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;
	print "$(" this_PKG "_IMAGE): "                               "$(STATEDIR)/host-fakeroot.install.post"	> DGEN_DEPS_POST;
	print "$(" this_PKG "_IMAGE): " \
		"$(addprefix $(STATEDIR)/,$(addsuffix .targetinstall.post,$(" this_PKG "_PKGS)))"		> DGEN_DEPS_POST;
	print "$(" this_PKG "_IMAGE): " \
		"$(STATEDIR)/" this_pkg ".$(" this_PKG "_CFGHASH).cfghash"					> DGEN_DEPS_POST;
	print "$(" this_PKG "_IMAGE): "                               "$(" this_PKG "_FILES)"			> DGEN_DEPS_POST;
	if (GENERATE_REPORTS)
		print "$(" this_PKG "_IMAGE): $(STATEDIR)/" this_pkg ".reports"					> DGEN_DEPS_POST
	print "$(STATEDIR)/" this_pkg ".install.post: "               "$(" this_PKG "_IMAGE)"			> DGEN_DEPS_POST;
	print "images: "                                              "$(" this_PKG "_IMAGE)"			> DGEN_DEPS_POST;
	#
	# add dep to pkgs we depend on
	#
	this_PKG_DEPS = PKG_to_B_DEP[this_PKG];
	n = split(this_PKG_DEPS, this_DEP_array, " ");
	for (i = 1; i <= n; i++) {
		this_dep = PKG_to_pkg[this_DEP_array[i]]
		this_dep_prefix = gensub(/^(host-|cross-|image-|).*/, "\\1", 1, this_dep)
		if (this_dep_prefix == "")
			print "$(" this_PKG "_IMAGE): "         "$(STATEDIR)/" this_dep ".targetinstall.post"	> DGEN_DEPS_POST;
		else
			print "$(" this_PKG "_IMAGE): "               "$(STATEDIR)/" this_dep ".install.post"	> DGEN_DEPS_POST;
	}
	#
	# images don't depend on world, so this is needed to extract the packages
	#
	print "ifneq ($(strip $(" this_PKG "_PKGS)),)"								> DGEN_DEPS_POST
	print "$(" this_PKG "_IMAGE):" " \
		$(STATEDIR)/host-opkg.install.post"								> DGEN_DEPS_POST
	print "ifeq ($(strip $(" this_PKG "_NFSROOT)),YES)"							> DGEN_DEPS_POST
	print "$(foreach pkg,$(" this_PKG "_PKGS),$(eval $(PTX_MAP_TO_PACKAGE_$(pkg))_NFSROOT_DIRS += " \
		"$(PTXDIST_PLATFORMDIR)/nfsroot/" this_pkg "))"							> DGEN_DEPS_POST
	print "endif"												> DGEN_DEPS_POST
	print "endif"												> DGEN_DEPS_POST
}

END {
	for (symbol in config_symbols)
		write_symbols(symbol, config_symbols[symbol])

	# writing maps first as this affect the pkghash via virtual packages
	for (this_PKG in PKG_to_pkg) {
		this_pkg = PKG_to_pkg[this_PKG];
		write_maps(this_PKG, "R")
		write_maps(this_PKG, "B")
		write_sources(this_PKG)
	}
	# extend pkghash files fist
	for (this_PKG in active_PKG_to_pkg)
		write_deps_pkg_active_cfghash(this_PKG, PKG_to_pkg[this_PKG])

	print "ifdef PTXDIST_SETUP_ONCE"									> DGEN_DEPS_POST;
	for (this_PKG in PKG_to_pkg)
		print "DEPS: " this_PKG " " PKG_to_B_DEP[this_PKG] > PTXDIST_HASHLIST
	print "$(call ptx/force-shell, echo -e '$(PTXDIST_HASHLIST_DATA)' >> " PTXDIST_HASHLIST ")"		> DGEN_DEPS_POST;
	print "$(call ptx/force-sh, $(PTXDIST_LIB_DIR)/ptxd_make_pkghash.awk " PTXDIST_HASHLIST ")"		> DGEN_DEPS_POST;
	print "endif"												> DGEN_DEPS_POST;

	print "$(call ptx/force-sh, md5sum " PTXDIST_TEMPDIR "/pkghash-* | " \
		"sed 's;^\\([a-z0-9]*\\).*pkghash-\\(.*\\)$$;\\2_CFGHASH := \\1;' > " \
			PTXDIST_TEMPDIR "/pkghash.make)"							> DGEN_DEPS_POST;
	print "include " PTXDIST_TEMPDIR "/pkghash.make"							> DGEN_DEPS_POST;

	# for all pkgs
	all_pkg = ""
	for (this_PKG in PKG_to_pkg) {
		this_pkg = PKG_to_pkg[this_PKG];
		this_pkg_prefix = gensub(/^(host-|cross-|image-|).*/, "\\1", 1, this_pkg)

		write_vars_all(this_PKG)
		if (this_pkg_prefix != "image-") {
			write_deps_pkg_all(this_PKG, this_pkg)
			write_vars_pkg_all(this_PKG, this_pkg, this_pkg_prefix)
		}
		all_pkg = all_pkg " " this_pkg
	}
	write_all_deps("R")
	write_all_deps("B")
	print "PTX_PACKAGES_ALL := " all_pkg									> DGEN_DEPS_PRE;
	virtual_pkgs = ""
	for (this_PKG in virtual_pkg) {
		this_pkg = gensub("_", "-", "g", tolower(this_PKG));
		virtual_pkgs = virtual_pkgs " " this_pkg
	}
	print "PTX_PACKAGES_VIRTUAL := " virtual_pkgs								> DGEN_DEPS_PRE;

	# for active pkgs
	for (this_PKG in active_PKG_to_pkg) {
		this_pkg = PKG_to_pkg[this_PKG];
		this_pkg_prefix = gensub(/^(host-|cross-|image-|).*/, "\\1", 1, this_pkg)

		write_deps_all_active(this_PKG, this_pkg, this_pkg_prefix)
		if (this_pkg_prefix != "image-") {
			write_deps_pkg_active(this_PKG, this_pkg, this_pkg_prefix)
			write_deps_pkg_active_virtual(this_PKG, this_pkg, this_pkg_prefix)
		}
		else
			write_deps_pkg_active_image(this_PKG, this_pkg, this_pkg_prefix)
	}

	close(PKG_HASHFILE);
	close(MAP_ALL);
	close(MAP_ALL_MAKE);
	close(MAP_DEPS);
	close(DGEN_DEPS_PRE);
	close(DGEN_DEPS_POST);
	close(DGEN_RULESFILES_MAKE);
}
