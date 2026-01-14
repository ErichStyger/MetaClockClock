# Add set(CONFIG_USE_middleware_usb_device_stack true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/device/usb_device_ch9.c
          ${CMAKE_CURRENT_LIST_DIR}/device/class/usb_device_class.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/device
          ${CMAKE_CURRENT_LIST_DIR}/device/class
        )

  
