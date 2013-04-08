
MACRO(SETUP_BUNDLE_PATHS _BUNDLE_PATH)

	# Use for search (dirs)
	LIST(APPEND BFG_SEARCH_INCDIRS ${_BUNDLE_PATH})
	LIST(APPEND BFG_SEARCH_LIBDIRS ${_BUNDLE_PATH})

	# ----- #
	# Boost #
	# ----- #

	# CMake <=> Bjam Mapping for Visual Studio versions
	IF(${MSVC_VERSION} EQUAL "1500")
		SET(BJAM_MS_INTERFIX "vc90")
		SET(BOOST_MINOR_VERSION_NR "47")
	ELSEIF(${MSVC_VERSION} EQUAL "1600")
		SET(BJAM_MS_INTERFIX "vc100")
		SET(BOOST_MINOR_VERSION_NR "47")
	ELSEIF(${MSVC_VERSION} EQUAL "1700")
		SET(BJAM_MS_INTERFIX "vc110")
		SET(BOOST_MINOR_VERSION_NR "49")
	ELSE()
		SET(BJAM_MS_INTERFIX "" STRING)
			LOG_WARNING("Unknown Microsoft compiler detected, so you'll have"
						" to set the correct interfix (e.g. \"vc110\") manually.")
	ENDIF(${MSVC_VERSION} EQUAL "1500")
	
	
	SET(BOOST_ROOT "${_BUNDLE_PATH}/boost_1_${BOOST_MINOR_VERSION_NR}_0")
	SET(Boost_USE_MULTITHREADED TRUE)
	SET(Boost_USE_STATIC_LIBS   FALSE)

	# --------- #
	# Boost.Log #
	# --------- #

	SET(BoostLog_INCLUDE_DIR "${_BUNDLE_PATH}/boost-log")
	SET(BoostLog_LIBRARY_DIR "${_BUNDLE_PATH}/boost-log/lib")

	IF(CMAKE_CONFIGURATION_TYPES)
		SET(BoostLog_LIBRARIES
			debug ${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-gd-1_${BOOST_MINOR_VERSION_NR}.lib
			debug ${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-gd-1_${BOOST_MINOR_VERSION_NR}.lib
			optimized ${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-1_${BOOST_MINOR_VERSION_NR}.lib
			optimized ${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-1_${BOOST_MINOR_VERSION_NR}.lib
		)
	ELSE()
		IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(BoostLog_LIBRARIES
				${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-gd-1_${BOOST_MINOR_VERSION_NR}.lib
				${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-gd-1_${BOOST_MINOR_VERSION_NR}.lib
			)
		ELSE() # Release!
			SET(BoostLog_LIBRARIES
				${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-1_${BOOST_MINOR_VERSION_NR}.lib
				${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-1_${BOOST_MINOR_VERSION_NR}.lib
			)
		ENDIF()
	ENDIF()

	# -------#
	# OpenAL #
	# -------#

	SET(OPENAL_INCLUDE_DIR "${_BUNDLE_PATH}/OpenAL/include")
	SET(OPENAL_LIBRARY_DIR "${_BUNDLE_PATH}/OpenAL/libs/Win32")
	SET(OPENAL_LIBRARY "${OPENAL_LIBRARY_DIR}/OpenAL32.lib")

	
	# --------#
	# PugiXML #
	# --------#

	SET(PUGIXML_INCLUDE_DIR "${_BUNDLE_PATH}/pugixml-1.2/include")
	SET(PUGIXML_LIBRARY_DIR "${_BUNDLE_PATH}/pugixml-1.2/lib/")
	
	# --------#
	#   ODE   #
	# --------#

	SET(ODE_INCLUDE_DIR "${_BUNDLE_PATH}/ode/include")
	SET(ODE_LIBRARY_DIR "${_BUNDLE_PATH}/ode/lib/")
	
	
ENDMACRO(SETUP_BUNDLE_PATHS)
