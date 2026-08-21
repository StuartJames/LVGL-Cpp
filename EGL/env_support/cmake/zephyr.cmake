if(CONFIG_EGL)

  message(STATUS "Compiling Zephyr")
  
  zephyr_include_directories(${ZEPHYR_BASE}/lib/egl)

  target_include_directories(EGL INTERFACE ${EGL_ROOT_DIR})

  zephyr_compile_definitions(EG_CONF_KCONFIG_EXTERNAL_INCLUDE=<autoconf.h>)

  zephyr_library()

  file(GLOB_RECURSE SOURCES ${EGL_ROOT_DIR}/src/*.cpp)
  zephyr_library_sources(${SOURCES})

endif(CONFIG_EGL)
