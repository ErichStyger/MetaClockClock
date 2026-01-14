# Add set(CONFIG_USE_middleware_mcu-boot_bootsources true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/autobaud/src/autobaud_irq.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_user_entry.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_main.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_context.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_exception_handler.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_tree_root.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_command.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_shutdown_cleanup.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_app_crc_check.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/bl_misc.c
          ${CMAKE_CURRENT_LIST_DIR}/src/crc/src/crc16.c
          ${CMAKE_CURRENT_LIST_DIR}/src/crc/src/crc32.c
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/memory.c
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/normal_memory.c
          ${CMAKE_CURRENT_LIST_DIR}/src/packet/src/serial_packet.c
          ${CMAKE_CURRENT_LIST_DIR}/src/sbloader/src/sbloader.c
          ${CMAKE_CURRENT_LIST_DIR}/src/utilities/src/fsl_assert.c
          ${CMAKE_CURRENT_LIST_DIR}/src/utilities/src/fsl_rtos_abstraction.c
          ${CMAKE_CURRENT_LIST_DIR}/src/utilities/src/debug_util.c
        )

        if(CONFIG_TOOLCHAIN STREQUAL iar)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/pattern_fill.s
        )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL mcux OR CONFIG_TOOLCHAIN STREQUAL armgcc))
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/pattern_fill_gcc.S
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL mdk)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/pattern_fill_mdk.s
        )
    endif()

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/src/autobaud
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader
          ${CMAKE_CURRENT_LIST_DIR}/src/crc
          ${CMAKE_CURRENT_LIST_DIR}/src/include
          ${CMAKE_CURRENT_LIST_DIR}/src/memory
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src
          ${CMAKE_CURRENT_LIST_DIR}/src/packet
          ${CMAKE_CURRENT_LIST_DIR}/src/property
          ${CMAKE_CURRENT_LIST_DIR}/src/sbloader
          ${CMAKE_CURRENT_LIST_DIR}/src/utilities
          ${CMAKE_CURRENT_LIST_DIR}/src/startup
        )

  
