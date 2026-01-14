# Add set(CONFIG_USE_middleware_usb_pd_config_header true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/config/pd/usb_pd_config.h "" middleware_usb_pd_config_header)
    
  

