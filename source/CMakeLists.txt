CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Tiny-DEBUG C)

SET(PROJ_NAME "Tiny-DEBUG")

#---------------------------------------------------------------------------------------
#
# include dir
#
#---------------------------------------------------------------------------------------

# Tiny
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyBase)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyPortable)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyContainer)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyLog)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyStringUtil)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/ByteBuffer)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/Json)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/Bson)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/StringArray)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyMd5)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyUmm)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyThread)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyUuid)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TinyWorker)

# Netty
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/Netty)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/HttpClient)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/TcpClient)

#--------------------------------------------------------------------------------------
#
# lib
#
#--------------------------------------------------------------------------------------
IF (WIN32)
    SET(OS_LIB ws2_32.lib)
ELSE (WIN32)
    IF(LINUX)
        SET(OS_LIB rt)
    ENDIF(LINUX)
ENDIF (WIN32)

MESSAGE(STATUS "os_lib: ${OS_LIB}")

#-------------------------------------------------------------------------------------
# output dir
#-------------------------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/target)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/target)
MESSAGE(STATUS "executable output:  ${EXECUTABLE_OUTPUT_PATH}")
MESSAGE(STATUS "library output: ${LIBRARY_OUTPUT_PATH}")

#-------------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------------
ADD_SUBDIRECTORY(src)
