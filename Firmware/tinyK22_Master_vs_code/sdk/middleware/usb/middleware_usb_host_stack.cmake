# Add set(CONFIG_USE_middleware_usb_host_stack true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/host/usb_host_hci.c
          ${CMAKE_CURRENT_LIST_DIR}/host/usb_host_devices.c
          ${CMAKE_CURRENT_LIST_DIR}/host/usb_host_framework.c
          ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_hub.c
          ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_hub_app.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/host
          ${CMAKE_CURRENT_LIST_DIR}/host/class
        )

  
