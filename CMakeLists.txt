# Modified from the public domain file inclued in Yue release 2.0

cmake_minimum_required(VERSION 3.1)

if((CMAKE_GENERATOR STREQUAL "Unix Makefiles") AND (NOT CMAKE_BUILD_TYPE))
  message(FATAL_ERROR "You must specify a build type.")
endif()

project(Lack)
set(APP_NAME "lack")
set(TEST_NAME "lackt")
set(TEST_JSON_HANDLER "test_json_handler")
set(TEST_CHANNEL_CONTAINER "test_channel_container")

# Define tests - seriously, need tests...
enable_testing()
add_test(ahungry_http_request ${TEST_NAME} ) # Args to come next
add_test(test_json_handler ${TEST_JSON_HANDLER} ) # Args to come next
add_test(test_channel_container ${TEST_CHANNEL_CONTAINER} ) # Args to come next


add_executable(${APP_NAME} ${APP_NAME}/main.cc)
add_executable(${TEST_NAME} ${APP_NAME}/include/lib/t/main.cc)
add_executable(${TEST_JSON_HANDLER} ${APP_NAME}/include/lib/t/test_json_handler.cc)
add_executable(${TEST_CHANNEL_CONTAINER} ${APP_NAME}/include/lib/t/test_channel_container.cc)

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${TEST_JSON_HANDLER} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${TEST_CHANNEL_CONTAINER} PROPERTY CXX_STANDARD 11)

target_include_directories(${APP_NAME} PRIVATE "yue/include" "lack/include")
target_include_directories(${TEST_NAME} PRIVATE "yue/include" "lack/include")
target_include_directories(${TEST_JSON_HANDLER} PRIVATE "yue/include" "lack/include")
target_include_directories(${TEST_CHANNEL_CONTAINER} PRIVATE "yue/include" "lack/include")

# macOS configuration.
if(APPLE)
  list(APPEND CMAKE_PREFIX_PATH /usr/local)

  set(CURL_LIBRARY "-lcurl")
  find_package(CURL)
  find_package(LibWebSockets)

  if (LibWebSockets_FOUND)
    include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})
    target_link_libraries(${APP_NAME} ${LIBWEBSOCKETS_LIBRARIES})
  else()
    message("not found libwebsockets")
  endif()

  if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(${APP_NAME} ${CURL_LIBRARIES})
  else()
    message("CURL not found")
  endif()

  find_library(APPKIT AppKit)
  find_library(IOKIT IOKit)
  find_library(SECURITY Security)

  target_link_libraries(${APP_NAME}
                        ${APPKIT} ${IOKIT} ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})

  target_link_libraries(${TEST_NAME}
                        ${APPKIT} ${IOKIT} ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})

  target_link_libraries(${TEST_JSON_HANDLER}
                        ${APPKIT} ${IOKIT} ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})

  target_link_libraries(${TEST_CHANNEL_CONTAINER}
                        ${APPKIT} ${IOKIT} ${SECURITY}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})

#  target_link_libraries(${APP_NAME}
#                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
#                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
#                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})
#  target_link_libraries(${TEST_NAME}
#                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
#                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
#                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES})

  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-Wl,-dead_strip")
endif()

# win32 configuration
if(WIN32)
  target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
               PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "/DELAYLOAD:setupapi.dll \
                         /DELAYLOAD:powrprof.dll \
                         /DELAYLOAD:dwmapi.dll \
                         /SUBSYSTEM:WINDOWS")
  target_link_libraries(${APP_NAME}
                        setupapi.lib powrprof.lib ws2_32.lib dbghelp.lib
                        shlwapi.lib version.lib winmm.lib psapi.lib dwmapi.lib
                        comctl32.lib gdi32.lib gdiplus.lib
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.lib
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.lib)
  foreach(flag_var
           CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
           CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
  endforeach()
endif()

# Linux configuration
if(UNIX AND NOT APPLE)
  find_package(PkgConfig)
  pkg_search_module(GTK3 REQUIRED gtk+-3.0)
  pkg_search_module(X11 REQUIRED x11)
  pkg_search_module(CURL REQUIRED libcurl)
  pkg_search_module(WEBSOCKET REQUIRED libwebsockets)
  pkg_search_module(JSONC REQUIRED json-c)
  #target_link_libraries(${APP_NAME}
  #                      pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES})
  target_include_directories(${APP_NAME} PUBLIC
                             ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
  target_include_directories(${TEST_NAME} PUBLIC
                             ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
  target_include_directories(${TEST_JSON_HANDLER} PUBLIC
                             ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
  target_include_directories(${TEST_CHANNEL_CONTAINER} PUBLIC
                             ${GTK3_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
  target_compile_options(${APP_NAME} PUBLIC
                         ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
  target_compile_options(${TEST_NAME} PUBLIC
                         ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
  target_compile_options(${TEST_JSON_HANDLER} PUBLIC
                         ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
  target_compile_options(${TEST_CHANNEL_CONTAINER} PUBLIC
                         ${GTK3_CFLAGS_OTHER} ${X11_CFLAGS_OTHER})
  target_link_libraries(${APP_NAME}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES} ${JSONC_LIBRARIES})
  target_link_libraries(${TEST_NAME}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES} ${JSONC_LIBRARIES})
  target_link_libraries(${TEST_JSON_HANDLER}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES} ${JSONC_LIBRARIES})
  target_link_libraries(${TEST_CHANNEL_CONTAINER}
                        optimized ${CMAKE_SOURCE_DIR}/yue/lib/libyue.a
                        debug ${CMAKE_SOURCE_DIR}/yue/lib/libyued.a
                        pthread dl ${GTK3_LIBRARIES} ${X11_LIBRARIES} ${CURL_LIBRARIES} ${WEBSOCKET_LIBRARIES} ${JSONC_LIBRARIES})
  target_compile_definitions(${APP_NAME} PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
  target_compile_definitions(${TEST_NAME} PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
  target_compile_definitions(${TEST_JSON_HANDLER} PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
  target_compile_definitions(${TEST_CHANNEL_CONTAINER} PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
  set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
  set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
  set_target_properties(${TEST_JSON_HANDLER} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
  set_target_properties(${TEST_CHANNEL_CONTAINER} PROPERTIES LINK_FLAGS
                        "-fdata-sections -ffunction-sections -Wl,--gc-section")
endif()
