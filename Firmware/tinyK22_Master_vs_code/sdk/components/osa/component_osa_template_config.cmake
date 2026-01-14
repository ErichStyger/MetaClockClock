# Add set(CONFIG_USE_component_osa_template_config true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/config/fsl_os_abstraction_config.h ${CMAKE_CURRENT_LIST_DIR}/config component_osa_template_config)
    
  

