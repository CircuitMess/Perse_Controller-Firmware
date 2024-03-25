#Target-specific sources (only used for that target)
file(GLOB_RECURSE GIF_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/GIF/*.cpp")
file(GLOB_RECURSE SCREENS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Screens/*.cpp")
file(GLOB_RECURSE UI_SYSTEM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/UISystem/*.cpp")

set(MISSION_CTRL_SOURCES
        "${GIF_SOURCES}"
        "${SCREENS_SOURCES}"
        "${UI_SYSTEM_SOURCES}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/AW9523.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/Display.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/Backlight.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/Encoders.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/Joystick.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/SingleExpanderLED.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/Potentiometers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Periph/I2C.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/UDPListener.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/Feed.cpp"
        CACHE INTERNAL "" FORCE
)

set(BASIC_CTRL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/StateMachine.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/States/PairState.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/States/DriveState.cpp"
        CACHE INTERNAL "" FORCE
)

#Common libs:
file(GLOB_RECURSE COMMON_LIBS
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/src/**.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/src/**.c"
)


set(COMMON_LIBS_INCL
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/glm"
        CACHE INTERNAL "" FORCE
)


#Common sources:
set(COMMON_SOURCES
        ""
        CACHE INTERNAL "" FORCE
)

file(GLOB PERIPH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Periph/*.cpp")
list(REMOVE_ITEM PERIPH_SOURCES ${MISSION_CTRL_SOURCES} ${BASIC_CTRL_SOURCES})
list(APPEND COMMON_SOURCES ${PERIPH_SOURCES})

file(GLOB SERVICES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Services/*.cpp")
list(REMOVE_ITEM SERVICES_SOURCES ${MISSION_CTRL_SOURCES} ${BASIC_CTRL_SOURCES})
list(APPEND COMMON_SOURCES ${SERVICES_SOURCES})

file(GLOB UTIL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Util/*.cpp")
list(REMOVE_ITEM UTIL_SOURCES ${MISSION_CTRL_SOURCES} ${BASIC_CTRL_SOURCES})
list(APPEND COMMON_SOURCES ${UTIL_SOURCES})

file(GLOB DEVICE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Devices/*.cpp")
list(REMOVE_ITEM DEVICE_SOURCES ${MISSION_CTRL_SOURCES} ${BASIC_CTRL_SOURCES})
list(APPEND COMMON_SOURCES ${DEVICE_SOURCES})

#Type-specific sources, libs
if ("${CTRL_TYPE}" STREQUAL "MissionCtrl")
    list(APPEND SOURCES ${MISSION_CTRL_SOURCES})

    set(SPIFFS_PATH
            "${CMAKE_CURRENT_SOURCE_DIR}/../spiffs_MissionCtrl"
            CACHE INTERNAL "" FORCE
    )

    spiffs_create_partition_image(storage ${SPIFFS_PATH} FLASH_IN_PROJECT)

elseif ("${CTRL_TYPE}" STREQUAL "Basic")
    list(APPEND SOURCES ${BASIC_CTRL_SOURCES})
endif ()


list(APPEND SOURCES ${COMMON_SOURCES})
list(REMOVE_DUPLICATES SOURCES)

list(APPEND LIBS ${COMMON_LIBS})
list(REMOVE_DUPLICATES LIBS)
list(APPEND LIBS_INCL ${COMMON_LIBS_INCL})
list(REMOVE_DUPLICATES LIBS_INCL)

if (CONFIG_CM_BUILD_FIRMWARE)
    set(ENTRY
            "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
            CACHE INTERNAL "" FORCE
    )

elseif (CONFIG_CM_HARDWARE_TEST)
    set(ENTRY
            "${CMAKE_CURRENT_SOURCE_DIR}/../examples/HWTest.cpp"
            CACHE INTERNAL "" FORCE
    )
endif ()

idf_component_register(SRCS
        ${ENTRY}
        ${SOURCES}
        ${LIBS}
        INCLUDE_DIRS
        "src"
        ${LIBS_INCL}
)
