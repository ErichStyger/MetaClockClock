# Add set(CONFIG_USE_middleware_usb_pd_phy_ptn5110 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_connect.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_hal.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_interface.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110/usb_pd_ptn5110_msg.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/usb_pd_timer.c
          ${CMAKE_CURRENT_LIST_DIR}/pd/phy_interface/usb_pd_i2c.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/pd
          ${CMAKE_CURRENT_LIST_DIR}/pd/ptn5110
          ${CMAKE_CURRENT_LIST_DIR}/pd/phy_interface
        )

  
