# Option to define EG_EGL_H_INCLUDE_SIMPLE, default: ON
option(EG_EGL_H_INCLUDE_SIMPLE
       "Use #include \"EGL.h\" instead of #include \"../../EGL.h\"" ON)

# Option to define EG_CONF_INCLUDE_SIMPLE, default: ON
option(EG_CONF_INCLUDE_SIMPLE
       "Simple include of \"EG_Config.h\" and \"lv_drv_conf.h\"" ON)

# Option to set EG_CONF_PATH, if set parent path EG_CONF_DIR is added to
# includes
option(EG_CONF_PATH "Path defined for EG_Config.h")
get_filename_component(EG_CONF_DIR ${EG_CONF_PATH} DIRECTORY)

# Option to build shared libraries (as opposed to static), default: OFF
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

file(GLOB_RECURSE SOURCES ${EGL_ROOT_DIR}/src/*.cpp)

if (BUILD_SHARED_LIBS)
  add_library(egl SHARED ${SOURCES})
else()
  add_library(egl STATIC ${SOURCES})
endif()

add_library(egl::egl ALIAS egl)

target_compile_definitions(
  egl PUBLIC $<$<BOOL:${EG_EGL_H_INCLUDE_SIMPLE}>:EG_EGL_H_INCLUDE_SIMPLE>
              $<$<BOOL:${EG_CONF_INCLUDE_SIMPLE}>:EG_CONF_INCLUDE_SIMPLE>)

# Include root and optional parent path of EG_CONF_PATH
target_include_directories(egl SYSTEM PUBLIC ${EGL_ROOT_DIR} ${EG_CONF_DIR})

# Lbrary and headers can be installed to system using make install
file(GLOB EGL_PUBLIC_HEADERS "${CMAKE_SOURCE_DIR}/EG_Config.h"
     "${CMAKE_SOURCE_DIR}/EGL.h")

if("${LIB_INSTALL_DIR}" STREQUAL "")
  set(LIB_INSTALL_DIR "lib")
endif()
if("${INC_INSTALL_DIR}" STREQUAL "")
  set(INC_INSTALL_DIR "include/egl")
endif()

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/src"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/"
  FILES_MATCHING
  PATTERN "*.h")

install(
  FILES "${EG_CONF_PATH}"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/../"
  RENAME "EG_Config.h"
  OPTIONAL)

set_target_properties(
  egl
  PROPERTIES OUTPUT_NAME egl
             ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             PUBLIC_HEADER "${EGL_PUBLIC_HEADERS}")

install(
  TARGETS egl
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
  RUNTIME DESTINATION "${LIB_INSTALL_DIR}"
  PUBLIC_HEADER DESTINATION "${INC_INSTALL_DIR}")
