# Add set(CONFIG_USE_middleware_sdmmc_sdhc_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/sdhc/sdmmc_config.c "" middleware_sdmmc_sdhc_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/sdhc/sdmmc_config.h ${CMAKE_CURRENT_LIST_DIR}/template/sdhc middleware_sdmmc_sdhc_template)
    
  

