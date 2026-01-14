# Add set(CONFIG_USE_middleware_sdmmc_sdif_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/sdif/sdmmc_config.c "" middleware_sdmmc_sdif_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/sdif/sdmmc_config.h ${CMAKE_CURRENT_LIST_DIR}/template/sdif middleware_sdmmc_sdif_template)
    
  

