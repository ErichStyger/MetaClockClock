# Add set(CONFIG_USE_middleware_mcu-boot_flashloader_loader_image true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/..
        )

        if(CONFIG_TOOLCHAIN STREQUAL armgcc)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1040/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkcmimxrt1060/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1015/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018m/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1010/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1064/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1180/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1024/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk32l2a4s/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/twrkm35z75m/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk22f/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1170/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1020/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke16z/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbimxrt1050/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1160/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1060/bootloader_examples/flashloader/armgcc
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z512/bootloader_examples/flashloader/armgcc
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1040/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkcmimxrt1060/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1015/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018m/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1010/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1064/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1180/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1024/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk32l2a4s/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/twrkm35z75m/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk22f/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1170/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1020/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke16z/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbimxrt1050/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1160/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1060/bootloader_examples/flashloader/iar
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z512/bootloader_examples/flashloader/iar
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL mdk)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1040/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkcmimxrt1060/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1015/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018m/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1010/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1064/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1180/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1024/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk32l2a4s/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/twrkm35z75m/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmk22f/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1170/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1020/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke16z/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/lpcxpresso54s018/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbimxrt1050/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkmimxrt1160/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/evkbmimxrt1060/bootloader_examples/flashloader/mdk
          ${CMAKE_CURRENT_LIST_DIR}/../../boards/frdmke17z512/bootloader_examples/flashloader/mdk
        )
    endif()

  
