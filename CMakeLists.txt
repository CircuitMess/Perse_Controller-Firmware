cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEFAULT_CTRL_TYPE
        "MissionCtrl"
        CACHE INTERNAL "" FORCE
)

if (NOT DEFINED CTRL_TYPE)
    set(CTRL_TYPE
            "${DEFAULT_CTRL_TYPE}"
            CACHE INTERNAL "" FORCE
    )
endif ()

if ("${CTRL_TYPE}" STREQUAL "MissionCtrl")
    set(IDF_TARGET esp32s3)
    add_compile_definitions("CTRL_TYPE_MISSIONCTRL")
    set(SDKCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.missionctrl")
    add_compile_definitions("LV_TICK_CUSTOM_SYS_TIME_EXPR=(esp_timer_get_time() / 1000LL)")

elseif ("${CTRL_TYPE}" STREQUAL "Basic")
    set(IDF_TARGET esp32c3)
    add_compile_definitions("CTRL_TYPE_BASIC")
    set(SDKCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.basic")
    set(EXCLUDE_COMPONENTS JPEGDEC LovyanGFX)

else ()
    message(ERROR "Perse-Ctrl: CTRL_TYPE was set to an invalid value. Valid values are MissionCtrl & Basic.")
endif ()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Perse-Ctrl)
