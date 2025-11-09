# Install script for directory: /Users/roma/QtProjects/Chat/external/IXWebSocket

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/libixwebsocket.11.4.6.dylib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libixwebsocket.11.4.6.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libixwebsocket.11.4.6.dylib")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/opt/homebrew/lib"
      -delete_rpath "/opt/homebrew/opt/hiredis/lib"
      -delete_rpath "/opt/homebrew/opt/redis-plus-plus/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libixwebsocket.11.4.6.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libixwebsocket.11.4.6.dylib")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/libixwebsocket.dylib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ixwebsocket" TYPE FILE FILES
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXBase64.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXBench.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXCancellationRequest.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXConnectionState.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXDNSLookup.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXExponentialBackoff.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXGetFreePort.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXGzipCodec.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXHttp.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXHttpClient.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXHttpServer.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXNetSystem.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXProgressCallback.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSelectInterrupt.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSelectInterruptFactory.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSelectInterruptPipe.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSelectInterruptEvent.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSetThreadName.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSocket.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSocketConnect.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSocketFactory.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSocketServer.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXSocketTLSOptions.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXStrCaseCompare.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUdpSocket.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUniquePtr.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUrlParser.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUuid.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUtf8Validator.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXUserAgent.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocket.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketCloseConstants.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketCloseInfo.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketErrorInfo.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketHandshake.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketHandshakeKeyGen.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketHttpHeaders.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketInitResult.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketMessage.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketMessageType.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketOpenInfo.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflate.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflateCodec.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketPerMessageDeflateOptions.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketProxyServer.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketSendData.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketSendInfo.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketServer.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketTransport.h"
    "/Users/roma/QtProjects/Chat/external/IXWebSocket/ixwebsocket/IXWebSocketVersion.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket" TYPE FILE FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/ixwebsocket-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/ixwebsocket.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-targets.cmake"
         "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/CMakeFiles/Export/dbc99e06a99e696141dafd40631f8060/ixwebsocket-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket/ixwebsocket-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket" TYPE FILE FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/CMakeFiles/Export/dbc99e06a99e696141dafd40631f8060/ixwebsocket-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ixwebsocket" TYPE FILE FILES "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/CMakeFiles/Export/dbc99e06a99e696141dafd40631f8060/ixwebsocket-targets-release.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/roma/QtProjects/Chat/Backend/ApigateWay/external/IXWebSocket-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
