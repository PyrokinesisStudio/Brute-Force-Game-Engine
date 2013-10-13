#!/bin/bash

CMAKE=`which cmake`
SVN=`which svn`
TAR=`which tar`
WGET=`which wget`

if [ ! -x $CMAKE ]; then echo "Please install CMake" && exit; fi
if [ ! -x $SVN ]; then echo "Please install Subversion" && exit; fi
if [ ! -x $TAR ]; then echo "Please install tar" && exit; fi
if [ ! -x $WGET ]; then echo "Please install wget" && exit; fi

# pbuilder:
# 1. apt-get install git wget subversion vim cmake ssh automake libtool
# 2. check compile flags (use -mtune=generic)

PACKAGE=package
PREFIX=$PACKAGE/usr
CPUS=`grep -c processor /proc/cpuinfo`
JOBS=`echo print $CPUS + 1 | perl`
VERSION="0.7.0"
DEBIAN_ARCH="`dpkg --print-architecture`"

USER_AGENT='Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.16) Gecko/20120421 Firefox/11.0'

BOOST_VERSION_DOTS='1.49.0'
BOOST_VERSION_SCORES=`echo $BOOST_VERSION_DOTS | tr . _`

BOOST_URL="http://netcologne.dl.sourceforge.net/project/boost/boost/$BOOST_VERSION_DOTS/boost_$BOOST_VERSION_SCORES.tar.bz2"
OGRE_URL="http://netcologne.dl.sourceforge.net/project/ogre/ogre/1.7/ogre_src_v1-7-3.tar.bz2"
PUGIXML_URL="https://pugixml.googlecode.com/files/pugixml-1.2.tar.gz"

BOOST_FILENAME="boost_$BOOST_VERSION_SCORES.tar.bz2"
OGRE_FILENAME='ogre_src_v1-7-3.tar.bz2'
PUGIXML_FILENAME="pugixml-1.2.tar.gz"

BOOST_DIR="boost_$BOOST_VERSION_SCORES"
PUGIXML_DIR='pugixml-1.2'

BOOST_LOG_REV='607'
BOOST_GEOMETRY_EXTENSIONS_REV='77008'
MYGUI_REV='4431'
ODE_REV='1727'

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
	mkdir MAKE_BUNDLE_DEP
	cd MAKE_BUNDLE_DEP

	# Libs
	/usr/bin/apt-get install --ignore-missing \
		libbz2-dev       \
		libfreeimage-dev \
		libfreetype6-dev \
		libgl1-mesa-dev  \
		libglu-dev       \
		libois-dev       \
		libxaw7-dev      \
		libxrandr-dev    \
		libzip-dev       \
		libzzip-dev

	/bin/mkdir -p $PREFIX
}

function postlude
{
	# Leave temporary build directory
	cd ..
}

# Build Bjam, Boost and Boost.Log
##################################

function buildBoost
{
	$WGET -U "$USER_AGENT" -c $BOOST_URL -O $BOOST_FILENAME

	echo "Unpacking $BOOST_FILENAME ..."
	$TAR -xjf $BOOST_FILENAME

	echo "Adding Boost.Geometry arithmetic extension ..."
	$SVN export -r $BOOST_GEOMETRY_EXTENSIONS_REV http://svn.boost.org/svn/boost/trunk/boost/geometry/extensions $BOOST_DIR/boost/geometry/extensions

	cd $BOOST_DIR

	./bootstrap.sh

	./b2                               \
		--with-date_time           \
		--with-filesystem          \
		--with-graph               \
		--with-iostreams           \
		--with-program_options     \
		--with-random              \
		--with-regex               \
		--with-serialization       \
		--with-system              \
		--with-test                \
		--with-thread              \
		define=BOOST_TEST_DYN_LINK \
		define=BOOST_TEST_MAIN     \
		variant=release            \
		threading=multi            \
		link=shared                \
		-j$JOBS                    \
		--prefix=../$PREFIX        \
		install
	cd ..
 }

function buildBoostLog
{
	$SVN export -r $BOOST_LOG_REV https://boost-log.svn.sourceforge.net/svnroot/boost-log/trunk/boost-log boost-log

	/bin/cp -r boost-log/* $BOOST_DIR

	cd $BOOST_DIR
	./b2                               \
		--with-log                 \
		define=BOOST_LOG_USE_CHAR  \
		variant=release            \
		threading=multi            \
		link=shared                \
		-j$JOBS                    \
		--prefix=../$PREFIX        \
		install
	cd ..
}

# Build OpenAL (requires libasound-dev)
########################################

function buildOpenAL
{
	$SVN export svn://connect.creativelabs.com/OpenAL/trunk OpenAL

	mkdir openal-build
	cd openal-build
	$CMAKE ../OpenAL/OpenAL-Soft -DCMAKE_INSTALL_PREFIX=../$PREFIX
	make -j$JOBS install
	cd ..
}

# Build OGRE
#############

function buildOgre
{
	$WGET -U "$USER_AGENT" -c $OGRE_URL -O $OGRE_FILENAME

	echo "Unpacking $OGRE_FILENAME ..."
	$TAR -xjf $OGRE_FILENAME

	mkdir ogre-build
	cd ogre-build

	$CMAKE ../ogre_src_v1-7-3 \
		-DOGRE_BUILD_SAMPLES=0 \
		-DOGRE_BUILD_TOOLS=0 \
		-DBOOST_INCLUDEDIR=../$PREFIX/include \
		-DBOOST_LIBRARYDIR=../$PREFIX/lib \
		-DCMAKE_INSTALL_PREFIX=../$PREFIX

	make -j$JOBS install
	cd ..
}

# Build MyGUI
##############

function buildMyGUI
{
	$SVN export -r $MYGUI_REV https://my-gui.svn.sourceforge.net/svnroot/my-gui/trunk my-gui

	# HACK: Fool MyGUI! It's too stupid to find Boost.
	ln -s ../../../../../$BOOST_DIR/boost my-gui/Platforms/Ogre/OgrePlatform/include

	/bin/mkdir mygui-build
	cd mygui-build

	$CMAKE ../my-gui \
		-DMYGUI_BUILD_DEMOS=0 \
		-DMYGUI_BUILD_TOOLS=0 \
		-DCMAKE_INSTALL_PREFIX=../$PREFIX \
		-DOGRE_INCLUDE_DIR=../$PREFIX/include/OGRE \
		-DOGRE_LIBRARIES=../$PREFIX/lib

	make -ik -j$JOBS install        # Skip some strange linker errors, which may hinder the installation process
	cd ..

	# install media
	/bin/mkdir -p $PREFIX/share/MyGUI
	/bin/cp -vR my-gui/Media $PREFIX/share/MyGUI
}

# Build PugiXml
################

function buildPugiXml
{
	$WGET -U "$USER_AGENT" -c $PUGIXML_URL -O $PUGIXML_FILENAME

	echo "Unpacking $PUGIXML_FILENAME ..."
	/bin/mkdir $PUGIXML_DIR
	$TAR -C $PUGIXML_DIR -xzf $PUGIXML_FILENAME

	mkdir pugixml-build
	cd pugixml-build
	$CMAKE ../$PUGIXML_DIR/scripts -DCMAKE_INSTALL_PREFIX=../$PREFIX
	make -j$JOBS install
	cd ..
}

# Build ODE
############

function buildOde
{
	$SVN export -r $ODE_REV svn://svn.code.sf.net/p/opende/code/trunk ode

	echo "Applying local-transform.patch to ODE"
	(cd ode && exec patch -p0 < ../../../../thirdparty/local-transform.patch)

	# Out-of-source build is not possible
	cd ode
	./autogen.sh
	./configure --prefix="`pwd`/../$PREFIX"
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
Package: libbfe-dev-deps
Version: $VERSION
Section: libdevel
Priority: optional
Architecture: $DEBIAN_ARCH
Depends: libsndfile-dev, libopenal-dev, libasound-dev, libois-dev, uuid-dev, libbz2-dev, libzip-dev, libbz2-dev, libfreeimage-dev, libzzip-dev
Suggests: doxygen, cmake
Conflicts: libboost-dev, libogre-dev, libode-dev, libode-sp-dev
Installed-Size: `du -s $PREFIX | perl -pe 's#[\t]+.+##g'` 
Maintainer: Sascha Wittkowski <w177us@gmail.com>
Description: Developer package (dependencies) for the Brute Force Game Engine" > $PACKAGE/DEBIAN/control

	PACKET_FILENAME="libbfe-dev-deps_"$VERSION"_"$DEBIAN_ARCH".deb"
	/usr/bin/dpkg --build $PACKAGE $PACKET_FILENAME
	/usr/bin/md5sum $PACKET_FILENAME > $PACKET_FILENAME.md5
}

# Do it
########

prelude
buildBoost
buildBoostLog
#buildOpenAL  (unnecessary: debian package is fine)
buildOgre
buildMyGUI
buildPugiXml
buildOde
makePackage
postlude
