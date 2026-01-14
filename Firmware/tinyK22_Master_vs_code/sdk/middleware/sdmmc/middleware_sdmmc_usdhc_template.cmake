# Add set(CONFIG_USE_middleware_sdmmc_usdhc_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/usdhc/sdmmc_config.c "" middleware_sdmmc_usdhc_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/usdhc/sdmmc_config.h ${CMAKE_CURRENT_LIST_DIR}/template/usdhc middleware_sdmmc_usdhc_template)
    
  

