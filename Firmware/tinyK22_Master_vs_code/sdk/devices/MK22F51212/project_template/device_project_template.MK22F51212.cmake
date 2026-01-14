# Add set(CONFIG_USE_device_project_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/board.h "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/board.c "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.h "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.c "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.h "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.c "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.h "" device_project_template.MK22F51212)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.c "" device_project_template.MK22F51212)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

  
