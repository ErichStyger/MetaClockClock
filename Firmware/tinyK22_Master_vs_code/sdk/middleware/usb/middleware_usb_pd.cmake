# Add set(CONFIG_USE_middleware_usb_pd true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_connect.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_msg.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_policy.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_timer.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/pd
        )

  
