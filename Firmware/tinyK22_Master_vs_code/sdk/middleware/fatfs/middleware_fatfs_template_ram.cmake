# Add set(CONFIG_USE_middleware_fatfs_template_ram true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/ram/ffconf_gen.h ${CMAKE_CURRENT_LIST_DIR}/template/ram middleware_fatfs_template_ram)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/ffconf.h ${CMAKE_CURRENT_LIST_DIR}/template middleware_fatfs_template_ram)
    
  

