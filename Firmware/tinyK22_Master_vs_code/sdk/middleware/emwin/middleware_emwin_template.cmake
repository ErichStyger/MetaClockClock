# Add set(CONFIG_USE_middleware_emwin_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/emwin_support.c "" middleware_emwin_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/emwin_support.h ${CMAKE_CURRENT_LIST_DIR}/template middleware_emwin_template)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/emWin_Config
          ${CMAKE_CURRENT_LIST_DIR}/emWin_header
        )

  
