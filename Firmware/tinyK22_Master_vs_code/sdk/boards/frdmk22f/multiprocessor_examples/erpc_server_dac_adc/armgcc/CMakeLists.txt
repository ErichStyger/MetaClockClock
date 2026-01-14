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


set(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
set(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
set(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(erpc_server_dac_adc)

enable_language(ASM)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME erpc_server_dac_adc.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${ProjDirPath}/../../../../..)
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../run.py"
"${ProjDirPath}/../service/erpc_dac_adc.erpc"
"${ProjDirPath}/../service/__init__.py"
"${ProjDirPath}/../service/erpc_dac_adc/__init__.py"
"${ProjDirPath}/../service/erpc_dac_adc/client.py"
"${ProjDirPath}/../service/erpc_dac_adc/common.py"
"${ProjDirPath}/../service/erpc_dac_adc/interface.py"
"${ProjDirPath}/../service/erpc_dac_adc/server.py"
"${ProjDirPath}/../service/erpc_dac_adc_server.cpp"
"${ProjDirPath}/../service/erpc_dac_adc_server.hpp"
"${ProjDirPath}/../service/erpc_dac_adc_common.h"
"${ProjDirPath}/../service/erpc_dac_adc_common.hpp"
"${ProjDirPath}/../service/erpc_dac_adc_interface.hpp"
"${ProjDirPath}/../service/erpc_dac_adc_interface.cpp"
"${ProjDirPath}/../service/c_erpc_dac_adc_server.h"
"${ProjDirPath}/../service/c_erpc_dac_adc_server.cpp"
"${ProjDirPath}/../erpc_dac_adc.erpc"
"${ProjDirPath}/../__init__.py"
"${ProjDirPath}/../RTE_Device.h"
"${ProjDirPath}/../fxos.c"
"${ProjDirPath}/../main_server.c"
"${ProjDirPath}/../client.py"
"${ProjDirPath}/../common.py"
"${ProjDirPath}/../interface.py"
"${ProjDirPath}/../server.py"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../hardware_init.c"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../erpc_config_gen.h"
"${ProjDirPath}/../mcux_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../service
)

set_source_files_properties("${ProjDirPath}/../RTE_Device.h" PROPERTIES COMPONENT_CONFIG_FILE "device_RTE.MK22F51212")

include(${SdkRootDirPath}/devices/MK22F51212/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

# wrap all libraries with -Wl,--start-group -Wl,--end-group to prevent link order issue
group_link_libraries()

