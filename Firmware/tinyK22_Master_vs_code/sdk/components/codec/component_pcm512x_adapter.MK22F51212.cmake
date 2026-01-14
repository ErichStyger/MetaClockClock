# Add set(CONFIG_USE_component_pcm512x_adapter true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/port/pcm512x/fsl_codec_pcm512x_adapter.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/port/pcm512x
          ${CMAKE_CURRENT_LIST_DIR}/port
        )

  
