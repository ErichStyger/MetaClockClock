# Add set(CONFIG_USE_middleware_mcu-boot_MK22F51212_sources true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/bl_clock_config_MK22F51212.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/hardware_init_MK22F51212.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/memory_map_MK22F51212.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/bl_peripherals_MK22F51212.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/pinmux_utility_common.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_dspi_irq_config_common.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_i2c_irq_config_common.c
          ${CMAKE_CURRENT_LIST_DIR}/targets/common/src/bl_uart_irq_config_common.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/scuart_peripheral_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/dspi_peripheral_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/i2c_peripheral_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/src/bootloader/src/usb_hid_msc_peripheral_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/src/memory/src/sram_init_cm4.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src
        )

  
