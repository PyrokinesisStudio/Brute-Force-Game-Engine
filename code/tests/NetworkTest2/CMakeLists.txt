BFG_INIT_PACKAGE(NetworkTest2)

FILE(GLOB SOURCES
	*.cpp
)

FILE(GLOB HEADERS
	*.h
)

INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${BFG_ENGINE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(..)	# For subdirectories
INCLUDE_DIRECTORIES(.)	# For subdirectories

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

ADD_EXECUTABLE(bfgNetworkTest2 ${SOURCES} ${HEADERS})

ADD_TEST(NAME NetworkTest2 COMMAND bfgNetworkTest2)

SET_TARGET_PROPERTIES(bfgNetworkTest2 PROPERTIES OUTPUT_NAME_DEBUG "bfgNetworkTest2_d")

TARGET_LINK_LIBRARIES(bfgNetworkTest2
	${Boost_LIBRARIES}
	${BoostLog_LIBRARIES}
	bfgNetwork
)

INSTALL(TARGETS bfgNetworkTest2
	RUNTIME DESTINATION ${BFG_BIN_DIR}
	COMPONENT devel
)
