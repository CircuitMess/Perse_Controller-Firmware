file(GLOB_RECURSE SOURCES "src/**.cpp")

file(GLOB_RECURSE LIBS "lib/*/src/**.cpp" "lib/*/src/**.c")
set(LIBS_INCL "lib/glm/glm")

idf_component_register(SRCS "main.cpp" ${SOURCES} ${LIBS} INCLUDE_DIRS "src" ${LIBS_INCL})

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)