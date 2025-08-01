# CROSS COMPILER SETTING
set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10.0)

# THE VERSION NUMBER
SET (MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION 2)
SET (MCUXPRESSO_CMAKE_FORMAT_MINOR_VERSION 0)

include(ide_overrides.cmake OPTIONAL)

if(CMAKE_SCRIPT_MODE_FILE)
  message("${MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION}")
  return()
endif()


set(CMAKE_STATIC_LIBRARY_PREFIX)
set(CMAKE_STATIC_LIBRARY_SUFFIX)

set(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
set(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(tinyK22_Master)

enable_language(ASM)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME tinyK22_Master.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${ProjDirPath}/../../../../..)
endif()

#######################################################
SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=0")
#######################################################


include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

#######################################################
# Required for McuLib, to know the target system
set(
  MCULIB_TARGET MCUXPRESSO CACHE STRING 
  "Select McuLib target: MCUXPRESSO, RP2040 or ESP32"
)
set(PROJECT_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(MCULIB_DIR        "${PROJECT_ROOT_DIR}/../McuLib")
set(CLOCKCOMMON_DIR   "${PROJECT_ROOT_DIR}/../ClockCommon")
set(SDK_DIR           "${PROJECT_ROOT_DIR}/../sdks/tinyK22_Master_vs_code_sdk")
set(USBLIB_DIR        "${PROJECT_ROOT_DIR}/usb")

include_directories(
  ${PROJECT_ROOT_DIR}
  ${PROJECT_ROOT_DIR}/board
)
include_directories(
    ${SDK_DIR}/CMSIS/core/include
    ${SDK_DIR}/components/serial_manager
    ${SDK_DIR}/components/uart
    ${SDK_DIR}/devices/MK22F51212
    ${SDK_DIR}/devices/MK22F51212/periph20
    ${SDK_DIR}/devices/MK22F51212/drivers
    ${SDK_DIR}/devices/MK22F51212/utilities/debug_console
)
include_directories(${CLOCKCOMMON_DIR})

include_directories(
  ${PROJECT_ROOT_DIR}/usb/cdc
  ${PROJECT_ROOT_DIR}/usb/include
  ${PROJECT_ROOT_DIR}/usb/osa
  ${PROJECT_ROOT_DIR}/usb/device/include
  ${PROJECT_ROOT_DIR}/usb/device/source
  ${PROJECT_ROOT_DIR}/usb/device/source/khci
  ${PROJECT_ROOT_DIR}/usb/device/class
  ${PROJECT_ROOT_DIR}/usb/device/class/cdc
)

add_compile_options(
  -include "${PROJECT_ROOT_DIR}/IncludeMcuLibConfig.h"
  -DUSB_STACK_BM
)

add_subdirectory(${MCULIB_DIR}                McuLib)
add_subdirectory(${CLOCKCOMMON_DIR}           ClockCommonLib)
add_subdirectory(${USBLIB_DIR}                UsbLib)

message(STATUS "Clock common: ${CLOCKCOMMON_DIR}")
#######################################################

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../board/peripherals.c"
"${ProjDirPath}/../board/peripherals.h"
"${ProjDirPath}/../board/pin_mux.c"
"${ProjDirPath}/../board/pin_mux.h"
"${ProjDirPath}/../board/board.h"
"${ProjDirPath}/../board/board.c"
"${ProjDirPath}/../board/clock_config.h"
"${ProjDirPath}/../board/clock_config.c"
"${ProjDirPath}/../board/hardware_init.c"
"${ProjDirPath}/../mcux_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../board
)

include(${SdkRootDirPath}/devices/MK22F51212/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

#######################################################
target_link_libraries(
  ${MCUX_SDK_PROJECT_NAME}
  PUBLIC McuLib
  PUBLIC ClockCommonLib
  PUBLIC UsbLib
)
#######################################################


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/new_project.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/new_project.bin")

# wrap all libraries with -Wl,--start-group -Wl,--end-group to prevent link order issue
group_link_libraries()

