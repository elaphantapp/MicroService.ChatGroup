cmake_minimum_required(VERSION 3.5)
#project(ChatGroupService)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(SOURCE_FILES
        source/Tools/Log.hpp
        source/Tools/Log.cpp
        source/Common/CommonVar.h
        source/Command/ChatGroupCmd.cpp
        source/Command/ChatGroupCmd.hpp
        source/Common/FileUtils.cpp
        source/Common/FileUtils.hpp
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
INCLUDE_DIRECTORIES(source/)
LINK_DIRECTORIES("source/")

add_library(ChatGroupService  SHARED )
target_sources(ChatGroupService PRIVATE
        ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(ChatGroupService pthread sqlite3)

pkg_search_module(pkg-Elastos.SDK.Contact REQUIRED Elastos.SDK.Contact)
message(STATUS "Elastos.SDK.Contact include path: ${pkg-Elastos.SDK.Contact_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(ChatGroupService PRIVATE "${pkg-Elastos.SDK.Contact_INCLUDE_DIRS}" )
TARGET_LINK_LIBRARIES(ChatGroupService PeerNode)

install(TARGETS ChatGroupService
        LIBRARY DESTINATION lib/PeerNodePlugins
        ARCHIVE DESTINATION lib/PeerNodePlugins)
