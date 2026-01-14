# Add set(CONFIG_USE_middleware_fatfs_template_sd true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/sd/ffconf_gen.h ${CMAKE_CURRENT_LIST_DIR}/template/sd middleware_fatfs_template_sd)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/ffconf.h ${CMAKE_CURRENT_LIST_DIR}/template middleware_fatfs_template_sd)
    
  

