# Add set(CONFIG_USE_CMSIS_DSP_Include true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/Include
          ${CMAKE_CURRENT_LIST_DIR}/PrivateInclude
        )

  
