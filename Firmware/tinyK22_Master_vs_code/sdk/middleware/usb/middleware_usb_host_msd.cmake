# Add set(CONFIG_USE_middleware_usb_host_msd true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_msd.c
          ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_msd_ufi.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/host/class
        )

  
