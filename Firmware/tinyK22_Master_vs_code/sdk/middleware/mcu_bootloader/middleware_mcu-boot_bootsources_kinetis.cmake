# Add set(CONFIG_USE_middleware_mcu-boot_bootsources_kinetis true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/smc/smc.c
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/flash_memory.c
          ${CMAKE_CURRENT_LIST_DIR}/src/property/src/property_kinetis.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_reliable_update.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/smc
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src
          ${CMAKE_CURRENT_LIST_DIR}/src/utilities
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader
        )

  
