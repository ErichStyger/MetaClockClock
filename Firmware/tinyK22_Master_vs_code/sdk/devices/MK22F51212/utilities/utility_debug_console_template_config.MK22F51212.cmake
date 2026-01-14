# Add set(CONFIG_USE_utility_debug_console_template_config true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/debug_console/config/fsl_debug_console_conf.h ${CMAKE_CURRENT_LIST_DIR}/debug_console/config utility_debug_console_template_config.MK22F51212)
    
  

