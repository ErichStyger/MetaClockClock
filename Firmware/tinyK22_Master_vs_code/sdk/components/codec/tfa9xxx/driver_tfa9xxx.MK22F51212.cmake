# Add set(CONFIG_USE_driver_tfa9xxx true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/fsl_tfa9xxx.c
          ${CMAKE_CURRENT_LIST_DIR}/fsl_tfa9xxx_IMX.c
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv/tfa2_container.c
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv/tfa2_dev.c
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv/tfa2_haptic.c
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv/tfa2_init.c
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv/tfa_container_crc32.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
          ${CMAKE_CURRENT_LIST_DIR}/vas_tfa_drv
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DCODEC_TFA9XXX_ENABLE
              )
  
  
  endif()

