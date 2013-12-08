#!/bin/bash

CMAKE=`which cmake`

if [ ! -x $CMAKE ]; then echo "Please install CMake" && exit; fi

PACKAGE=package
PREFIX=$PACKAGE/usr
CPUS=`grep -c processor /proc/cpuinfo`
JOBS=`echo print $CPUS + 1 | perl`
VERSION="0.9.0"
DEBIAN_ARCH="`dpkg --print-architecture`"

# Relative to the temporary build directory
RELATIVE_BFG_ROOT_PATH="../../.."

# Make sure only root can run our script
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

# Prelude
##########

function prelude
{
	# Create and enter temporary build directory
	mkdir MAKE_ENGINE_DEP
	cd MAKE_ENGINE_DEP

	/bin/mkdir -p $PREFIX
}

function postlude
{
	# Leave temporary build directory
	cd ..
}

# Build Bfg
############

function buildBfg
{
	mkdir bfg-build
	cd bfg-build
	$CMAKE "../$RELATIVE_BFG_ROOT_PATH" -DCMAKE_INSTALL_PREFIX="../$PREFIX"
	make -j$JOBS install
	cd ..
}

# MD5SUMS
##########

function makePackage
{
	cd $PACKAGE
	/usr/bin/find . | xargs md5sum 2&>/dev/null > ../md5sums
	/bin/mkdir -p DEBIAN
	/bin/mv ../md5sums DEBIAN
	cd ..

	/usr/bin/find $PREFIX -type d | xargs /bin/chmod 755

	echo "
Package: libbfe-dev
Version: $VERSION
Section: libdevel
Priority: optional
Architecture: $DEBIAN_ARCH
Depends: 
Recommends: libbfe-dev-deps
Conflicts: 
Installed-Size: `du -s $PREFIX | perl -pe 's#[\t]+.+##g'` 
Maintainer: Sascha Wittkowski <w177us@gmail.com>
Description: The Brute Force Game Engine" > $PACKAGE/DEBIAN/control

	PACKET_FILENAME="libbfe-dev_"$VERSION"_"$DEBIAN_ARCH".deb"
	/usr/bin/dpkg --build $PACKAGE $PACKET_FILENAME
	/usr/bin/md5sum $PACKET_FILENAME > $PACKET_FILENAME.md5
}

# Do it
########

prelude
buildBfg
makePackage
postlude
