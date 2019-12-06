cmake_minimum_required(VERSION 3.5)
project(chatgroupService)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES
        source/Tools/Log.hpp
        source/Tools/Log.cpp
        source/Common/CommonVar.h
        source/Command/ChatGroupCmd.cpp
        source/Command/ChatGroupCmd.hpp
        source/DataBase/DatabaseProxy.h
        source/DataBase/DatabaseProxy.cpp
        source/DataBase/MemberInfo.h
        source/DataBase/MemberInfo.cpp
        source/DataBase/MessageInfo.h
        source/DataBase/MessageInfo.cpp
        source/ThirdParty/CompatibleFileSystem.hpp
        source/ThirdParty/ghc-filesystem.hpp
        source/ChatGroupService.h
        source/ChatGroupService.cpp
        source/ErrCode.cpp
        source/ErrCode.h)
set(CHATGROUP_LIB_INCLUDE lib/include)
INCLUDE_DIRECTORIES(${CHATGROUP_LIB_INCLUDE})
INCLUDE_DIRECTORIES(source/)
INCLUDE_DIRECTORIES(lib/include/Elastos.SDK.Contact)
INCLUDE_DIRECTORIES(lib/include/Elastos.SDK.Keypair.C)
LINK_DIRECTORIES("source/")

add_library(chatgroupService  SHARED )
target_sources(chatgroupService PRIVATE
        ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(chatgroupService pthread sqlite3
        )
