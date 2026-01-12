file(GLOB_RECURSE SOURCES ${EGL_ROOT_DIR}/src/*.cpp)

idf_component_register(SRCS ${SOURCES} 
    INCLUDE_DIRS ${EGL_ROOT_DIR} ${EGL_ROOT_DIR}/include ${EGL_ROOT_DIR}/../
    REQUIRES esp_timer fatfs)

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DEG_CONF_INCLUDE_SIMPLE")

if(CONFIG_EG_ATTRIBUTE_FAST_MEM_USE_IRAM)
  target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DEG_ATTRIBUTE_FAST_MEM=IRAM_ATTR")
endif()
