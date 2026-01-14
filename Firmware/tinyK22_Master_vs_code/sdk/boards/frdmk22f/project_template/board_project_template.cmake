# Add set(CONFIG_USE_board_project_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/board.c
        )

              add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.h ${CMAKE_CURRENT_LIST_DIR}/. board_project_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.c "" board_project_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.h ${CMAKE_CURRENT_LIST_DIR}/. board_project_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.c "" board_project_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.h ${CMAKE_CURRENT_LIST_DIR}/. board_project_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.c "" board_project_template)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

  
