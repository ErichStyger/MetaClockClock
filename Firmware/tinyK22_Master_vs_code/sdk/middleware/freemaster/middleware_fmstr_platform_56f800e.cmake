# Add set(CONFIG_USE_middleware_fmstr_platform_56f800e true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/can/freemaster_flexcan_dsc.c
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/can/freemaster_mscan_dsc.c
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/serial/freemaster_serial_qsci.c
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/serial/freemaster_56f800e_eonce.c
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/serial/freemaster_serial_usb.c
        )

              add_config_file(${CMAKE_CURRENT_LIST_DIR}/src/template/56f800e/freemaster_cfg.h ${CMAKE_CURRENT_LIST_DIR}/src/template/56f800e middleware_fmstr_platform_56f800e)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/src/platforms/56f800e
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/can
          ${CMAKE_CURRENT_LIST_DIR}/src/drivers/mcuxsdk/serial
        )

  
