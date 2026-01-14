IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv4-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -DCPU_MK22FN512VLH12 \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DDEBUG \
    -DNDEBUG \
    -mthumb \
    -mcpu=cortex-m4 \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -DCPU_MK22FN512VLH12 \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DDEBUG \
    -DNDEBUG \
    -mthumb \
    -mcpu=cortex-m4 \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -include ${ProjDirPath}/../mcux_config.h \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -DCPU_MK22FN512VLH12 \
    -DFREEDOM \
    -DBL_TARGET_FLASH \
    -DFRDM_K22F \
    -DUSB_STACK_BM \
    -DFSL_OSA_BM_TIMER_CONFIG=FSL_OSA_BM_TIMER_NONE \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DUSE_RTOS=0 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -std=gnu99 \
    -mcpu=cortex-m4 \
    -ffreestanding \
    -g \
    -Os \
    -Wno-unused-variable \
    -MMD \
    -MP \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -include ${ProjDirPath}/../mcux_config.h \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -DCPU_MK22FN512VLH12 \
    -DFREEDOM \
    -DBL_TARGET_FLASH \
    -DFRDM_K22F \
    -DUSB_STACK_BM \
    -DFSL_OSA_BM_TIMER_CONFIG=FSL_OSA_BM_TIMER_NONE \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DUSE_RTOS=0 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -std=gnu99 \
    -mcpu=cortex-m4 \
    -ffreestanding \
    -g \
    -Os \
    -Wno-unused-variable \
    -MMD \
    -MP \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MK22FN512VLH12 \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DDEBUG \
    -DUSE_RTOS=0 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    -mcpu=cortex-m4 \
    -ffreestanding \
    -g \
    -Os \
    -MMD \
    -MP \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_DEBUG " \
    ${CMAKE_CXX_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MK22FN512VLH12 \
    -DMK22F51212_SERIES \
    -DOSA_USED \
    -DNDEBUG \
    -DUSE_RTOS=0 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    -mcpu=cortex-m4 \
    -ffreestanding \
    -g \
    -Os \
    -MMD \
    -MP \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -Xlinker \
    -Map=output.map \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Wl,--gc-sections \
    -Wl,-static \
    -Wl,--print-memory-usage \
    -mcpu=cortex-m4 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MK22FN512xxx12_application_0x0000.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    -Xlinker \
    -Map=output.map \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Wl,--gc-sections \
    -Wl,-static \
    -Wl,--print-memory-usage \
    -mcpu=cortex-m4 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MK22FN512xxx12_application_0x0000.ld\" -static \
")
