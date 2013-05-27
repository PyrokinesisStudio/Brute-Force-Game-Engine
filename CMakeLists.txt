# ------------------------------- #
# This is the root CMakeLists.txt #
# ------------------------------- #

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

# Our helper scripts are located in this folder
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/share/cmake/Modules)

INCLUDE(Enuminit)

ENUMINIT()

# ---------------------- #
# Optional Configuration #
# ---------------------- #

SET (BFG_DEPENDENCY_BUNDLE_PATH "" CACHE PATH "This is useful for Win32 users, the location of the dependency bundle may be hinted with this variable.")
OPTION(BFG_SDK "Package target makes a packet with release and debug binaries. Relevant only for Win32" ON)

# -------- #
# Includes #
# -------- #

INCLUDE(BFG)
INCLUDE(DependencyBundle)
INCLUDE(FindAllDependencies)

IF(BFG_TEST_COVERAGE)
	INCLUDE(CodeCoverage)
ENDIF()

# --------------------#
# Project Declaration #
# --------------------#

BFG_INIT_PROJECT("Brute-Force Game Engine" "bfg")
SET(CMAKE_PROJECT_NAME "BFG-Engine")

IF(NOT BFG_ENGINE_INCLUDE_DIR)
	SET(BFG_ENGINE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/code/engine/")
ENDIF(NOT BFG_ENGINE_INCLUDE_DIR)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)                  # Use Boost.Log as shared lib
ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK )                 # Use Boost as shared libs

# ---------------------- #
# Windows Specific Stuff #
# -----------------------#

IF(WIN32)
	# For Visual Studio 2008 or 2010 or 2012
	IF(${MSVC_VERSION} EQUAL "1500" OR ${MSVC_VERSION} EQUAL "1600" OR ${MSVC_VERSION} EQUAL "1700")
		ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)                 # Target: WinXP
		ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)             # Disable std::copy warnings
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)             # Disable sprintf warnings
		#ADD_DEFINITIONS(-DBOOST_LIB_DIAGNOSTIC)

		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251;4275") # Disable some DLL warnings
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")          # Enable MultiProcess compiling

		# Temporary solution for starting and debugging applications within Visual Studio.
		# A better way would be to use the install targets and to split engine and user builds.
		SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Library output path")
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Executable output path")
		SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin CACHE PATH "Executable output path")
	ENDIF()
ENDIF(WIN32)

#DUMP_LOG_MESSAGE_TYPES()

IF(BFG_DEPENDENCY_BUNDLE_PATH)
	LOG_ACTION("Using dependency bundle at: ${BFG_DEPENDENCY_BUNDLE_PATH}")
	SETUP_BUNDLE_PATHS(${BFG_DEPENDENCY_BUNDLE_PATH})
ENDIF()

FIND_ALL_DEPENDENCIES()

# ------------------------------------- #
# Locally modified thirdparty libraries #
# ------------------------------------- #

SET(TINYXML_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/tinyxml")

# ------- #
# Testing #
# ------- #

ENABLE_TESTING()

IF(BFG_TEST_COVERAGE)
	SETUP_FOR_COVERAGE(lcov lcov)
ENDIF()

# ------------------ #
# Output Directories #
# ------------------ #

IF(UNIX)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Linux/")
	SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
	SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
ENDIF(UNIX)

ADD_SUBDIRECTORY(thirdparty)
ADD_SUBDIRECTORY(code)

# ------------------------ #
# Documentation Generation #
# ------------------------ #

FIND_PACKAGE (Doxygen)
IF(DOXYGEN_FOUND)
	ADD_CUSTOM_TARGET (doc
		COMMAND doxygen ${PROJECT_SOURCE_DIR}/doc/Doxyfile
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
	)
ENDIF(DOXYGEN_FOUND)

# ---------------#
# Install Shared #
# ---------------#

IF(RELEASE OR BFG_SDK)
	INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/share/cmake
		DESTINATION "share/bfg"
		COMPONENT devel
	)
ENDIF()

# ---------------------------------#
# Install Dependency DLLs on Win32 #
# ---------------------------------#

IF(WIN32)

	IF(BFG_SDK)
		FILE(GLOB BFG_RUNTIME_FILES
		"${PROJECT_SOURCE_DIR}/bin/Release/*.dll"
		"${PROJECT_SOURCE_DIR}/bin/Debug/*.dll"
	)
	
	FILE(GLOB BFG_LIB_FILES
		"${PROJECT_SOURCE_DIR}/bin/Debug/*.lib"
		"${PROJECT_SOURCE_DIR}/bin/Release/*.lib"
	)
	
	INSTALL(FILES ${BFG_LIB_FILES}
		DESTINATION "lib"
		COMPONENT devel
		CONFIGURATIONS Release
	)
	
	INSTALL(FILES ${BFG_RUNTIME_FILES}
		DESTINATION "bin"
		COMPONENT devel
		CONFIGURATIONS Release
	)

	ELSE()

		FILE(GLOB BFG_DEPENDENCY_DLLS_DEBUG
			"${PROJECT_SOURCE_DIR}/bin/Debug/boost_*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/cg.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/libsndfile-1.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/MyGUI*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/ogg*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/Ogre*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/OIS*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/OpenAL32.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/Plugin*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Debug/RenderSystem*.dll"
		)

		FILE(GLOB BFG_DEPENDENCY_DLLS_RELEASE
			"${PROJECT_SOURCE_DIR}/bin/Release/boost_*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/cg.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/libsndfile-1.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/MyGUI*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/ogg*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/Ogre*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/OIS*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/OpenAL32.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/Plugin*.dll"
			"${PROJECT_SOURCE_DIR}/bin/Release/RenderSystem*.dll"
		)

		IF(BFG_DEPENDENCY_DLLS_DEBUG)
			INSTALL(FILES ${BFG_DEPENDENCY_DLLS_DEBUG}
				DESTINATION "bin"
				COMPONENT runtime
				CONFIGURATIONS Debug
			)
		ENDIF()
		
		IF(BFG_DEPENDENCY_DLLS_RELEASE)
			INSTALL(FILES ${BFG_DEPENDENCY_DLLS_RELEASE}
				DESTINATION "bin"
				COMPONENT runtime
				CONFIGURATIONS Release
			)
		ENDIF()

		IF(NOT BFG_DEPENDENCY_DLLS_DEBUG OR BFG_DEPENDENCY_DLLS_RELEASE)
			LOG("You should copy the content of the /bin directory from our dependency")
			LOG(" bundle into the engine's /bin directory or you probably won't")
			LOG(" be able to use the engine after its compilation.")
		ENDIF()

	ENDIF()
	
ENDIF(WIN32)

# --------- #
# Packaging #
# --------- #

# Don't warn about missing redistributables (VS 2010 Express has none)
IF(WIN32 AND NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
	SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
ENDIF()


# http://stackoverflow.com/questions/4134725/installrequiredsystemlibraries-purpose
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Brute Force Game Engine")
SET(CPACK_PACKAGE_VENDOR "Brute-Force-Games")
SET(CPACK_PACKAGE_CONTACT "Sascha Wittkowski <w177us@gmail.com>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "6")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "BFG-Engine")

SET(BFG_ENGINE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

IF(WIN32 AND NOT UNIX)
	# There is a bug in NSI that does not handle full unix paths properly. Make
	# sure there is at least one set of four (4) backslashes.
	SET(CPACK_GENERATOR "NSIS")
	SET(CPACK_NSIS_DISPLAY_NAME "BFG-Engine")
	SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.brute-force-games.com")
	SET(CPACK_NSIS_CONTACT "contact@brute-force-games.com")
	SET(CPACK_NSIS_MODIFY_PATH ON)
	SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")

	# Unused
	#SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
	#SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
	#SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")

ELSEIF(UNIX)
	SET(CPACK_GENERATOR "DEB")

	IF(RELEASE)
		SET(CPACK_DEBIAN_PACKAGE_NAME "libbfe-dev")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
		SET(CPACK_STRIP_FILES TRUE)
	ELSE()
		SET(CPACK_DEBIAN_PACKAGE_NAME "libbfe-dbg")
		SET(CPACK_DEBIAN_PACKAGE_SECTION "debug")
		SET(CPACK_STRIP_FILES FALSE)
	ENDIF()

	IF(AMD64)
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
	ELSE()
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
	ENDIF()

	SET(CPACK_DEBIAN_PACKAGE_VERSION ${BFG_ENGINE_VERSION})
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libbfe-dev-deps (>= 0.4.0)")
	SET(CPACK_SOURCE_STRIP_FILES "")

	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ENDIF()
INCLUDE(CPack)

