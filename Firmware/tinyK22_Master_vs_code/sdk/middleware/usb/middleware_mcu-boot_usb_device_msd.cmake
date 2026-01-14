# Add set(CONFIG_USE_middleware_mcu-boot_usb_device_msd true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/device/class/usb_device_msc.c
          ${CMAKE_CURRENT_LIST_DIR}/device/class/usb_device_msc_ufi.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/device/class
        )

  
