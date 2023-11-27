#Common libs:
file(GLOB_RECURSE COMMON_LIBS "lib/glm/src/**.cpp" "lib/glm/src/**.c")


set(COMMON_LIBS_INCL "lib/glm/glm" CACHE INTERNAL "" FORCE)


#Common sources
set(COMMON_SOURCES
        "src/Devices/Battery.cpp"
        "src/Devices/Input.cpp"
        "src/Devices/SingleLED.cpp"
        "src/Devices/SinglePwmLED.cpp"
        CACHE INTERNAL "" FORCE)

file(GLOB periph_srcs "src/Periph/*.cpp")
list(REMOVE_ITEM periph_srcs "src/Periph/I2C.cpp")
list(APPEND COMMON_SOURCES ${periph_srcs})

file(GLOB services_srcs "src/Services/*.cpp")
list(REMOVE_ITEM services_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/UDPListener.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/Feed.cpp")
list(APPEND COMMON_SOURCES ${services_srcs})

file(GLOB util_srcs "src/Util/*.cpp")
list(APPEND COMMON_SOURCES ${util_srcs})


#Type-specific sources, libs
if (NOT DEFINED CTRL_TYPE OR "${CTRL_TYPE}" STREQUAL "MissionCtrl")
    file(GLOB_RECURSE SOURCES "src/**.cpp" "src/**.c")

    set(SPIFFSPATH "../spiffs_MissionCtrl" CACHE INTERNAL "" FORCE)
elseif ("${CTRL_TYPE}" STREQUAL "Basic")


    set(SPIFFSPATH "../spiffs_Basic" CACHE INTERNAL "" FORCE)
endif ()


list(APPEND SOURCES ${COMMON_SOURCES})
list(REMOVE_DUPLICATES SOURCES)

list(APPEND LIBS ${COMMON_LIBS})
list(REMOVE_DUPLICATES LIBS)
list(APPEND LIBS_INCL ${COMMON_LIBS_INCL})
list(REMOVE_DUPLICATES LIBS_INCL)


if (CONFIG_CM_BUILD_FIRMWARE)
    set(ENTRY "main.cpp" CACHE INTERNAL "" FORCE)
elseif (CONFIG_CM_HARDWARE_TEST)
    set(ENTRY "../examples/HWTest.cpp" CACHE INTERNAL "" FORCE)
endif ()

idf_component_register(SRCS ${ENTRY} ${SOURCES} ${LIBS} INCLUDE_DIRS "src" ${LIBS_INCL})

spiffs_create_partition_image(storage ${SPIFFSPATH} FLASH_IN_PROJECT)