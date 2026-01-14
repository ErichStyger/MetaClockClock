# Add set(CONFIG_USE_component_tfa9xxx_adapter true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/port/tfa9xxx/fsl_codec_tfa9xxx_adapter.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/port/tfa9xxx
          ${CMAKE_CURRENT_LIST_DIR}/port
        )

  
