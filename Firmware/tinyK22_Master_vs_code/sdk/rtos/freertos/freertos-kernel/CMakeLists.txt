#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: MIT
#

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel)
    mcux_add_include(
        INCLUDES include
    )
    mcux_add_include(
        INCLUDES include
    )
    mcux_add_include(
        INCLUDES portable/GCC/ARM_CA9
        TOOLCHAINS armgcc mcux
        CORES ca7
    )
    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM0
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm0p
    )
    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM3
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm4 cm3
    )
    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM4F
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm4f cm7f
    )
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CA9
        TOOLCHAINS iar
        CORES ca7
    )
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM0
        TOOLCHAINS iar
        CORES cm0p
    )
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM3
        TOOLCHAINS iar
        CORES cm4 cm3
    )
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM4F
        TOOLCHAINS iar
        CORES cm4f cm7f
    )
    mcux_add_include(
        INCLUDES portable/RVDS/ARM_CA9
        TOOLCHAINS mdk
        CORES ca7
    )
    mcux_add_include(
        INCLUDES portable/RVDS/ARM_CM0
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm0p
    )
    mcux_add_include(
        INCLUDES portable/RVDS/ARM_CM3
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm4 cm3
    )
    mcux_add_include(
        INCLUDES portable/RVDS/ARM_CM4F
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm4f cm7f
    )
    mcux_add_source(
        SOURCES croutine.c
                event_groups.c
                list.c
                queue.c
                stream_buffer.c
                tasks.c
                timers.c
                include/atomic.h
                include/croutine.h
                include/deprecated_definitions.h
                include/event_groups.h
                include/FreeRTOS.h
                include/list.h
                include/message_buffer.h
                include/mpu_prototypes.h
                include/mpu_syscall_numbers.h
                include/mpu_wrappers.h
                include/portable.h
                include/projdefs.h
                include/queue.h
                include/semphr.h
                include/stack_macros.h
                include/StackMacros.h
                include/stdint.readme
                include/stream_buffer.h
                include/task.h
                include/timers.h
                portable/MemMang/ReadMe.url
                portable/readme.txt
    )
    mcux_add_source(
        SOURCES portable/GCC/ARM_CA9/port.c
                portable/GCC/ARM_CA9/portASM.S
                portable/GCC/ARM_CA9/portmacro.h
        TOOLCHAINS armgcc mcux
        CORES ca7
    )
    mcux_add_source(
        SOURCES portable/GCC/ARM_CM0/port.c
                portable/GCC/ARM_CM0/portmacro.h
                portable/GCC/ARM_CM0/portasm.h
                portable/GCC/ARM_CM0/portasm.c
                portable/GCC/ARM_CM0/mpu_wrappers_v2_asm.c
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm0p
    )
    mcux_add_source(
        SOURCES portable/GCC/ARM_CM3/port.c
                portable/GCC/ARM_CM3/portmacro.h
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm4 cm3
    )
    mcux_add_source(
        SOURCES portable/GCC/ARM_CM4F/port.c
                portable/GCC/ARM_CM4F/portmacro.h
        COMPILERS armclang gcc
        TOOLCHAINS armgcc mcux mdk
        CORES cm4f cm7f
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CA9/port.c
                portable/IAR/ARM_CA9/portASM.h
                portable/IAR/ARM_CA9/portASM.s
                portable/IAR/ARM_CA9/portmacro.h
        TOOLCHAINS iar
        CORES ca7
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CM0/port.c
                portable/IAR/ARM_CM0/portasm.s
                portable/IAR/ARM_CM0/portmacro.h
        TOOLCHAINS iar
        CORES cm0p
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CM3/port.c
                portable/IAR/ARM_CM3/portasm.s
                portable/IAR/ARM_CM3/portmacro.h
        TOOLCHAINS iar
        CORES cm4 cm3
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CM4F/port.c
                portable/IAR/ARM_CM4F/portasm.s
                portable/IAR/ARM_CM4F/portmacro.h
        TOOLCHAINS iar
        CORES cm4f cm7f
    )
    mcux_add_source(
        SOURCES portable/RVDS/ARM_CA9/port.c
                portable/RVDS/ARM_CA9/portASM.s
                portable/RVDS/ARM_CA9/portmacro.h
                portable/RVDS/ARM_CA9/portmacro.inc
        TOOLCHAINS mdk
        CORES ca7
    )
    mcux_add_source(
        SOURCES portable/RVDS/ARM_CM0/port.c
                portable/RVDS/ARM_CM0/portmacro.h
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm0p
    )
    mcux_add_source(
        SOURCES portable/RVDS/ARM_CM3/port.c
                portable/RVDS/ARM_CM3/portmacro.h
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm4 cm3
    )
    mcux_add_source(
        SOURCES portable/RVDS/ARM_CM4F/port.c
                portable/RVDS/ARM_CM4F/portmacro.h
        COMPILERS armcc
        TOOLCHAINS mdk
        CORES cm4f cm7f
    )

    mcux_add_configuration(
        CC "-DSDK_OS_FREE_RTOS"
        CX "-DSDK_OS_FREE_RTOS"
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.cm33_non_trustzone)
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM33_NTZ/non_secure
        TOOLCHAINS iar
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )
    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM33_NTZ/non_secure
        TOOLCHAINS armgcc mcux mdk
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )
    mcux_add_source(
        SOURCES portable/GCC/ARM_CM33_NTZ/non_secure/mpu_wrappers_v2_asm.c
                portable/GCC/ARM_CM33_NTZ/non_secure/port.c
                portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
                portable/GCC/ARM_CM33_NTZ/non_secure/portasm.h
                portable/GCC/ARM_CM33_NTZ/non_secure/portmacro.h
                portable/GCC/ARM_CM33_NTZ/non_secure/portmacrocommon.h
        TOOLCHAINS armgcc mcux mdk
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CM33_NTZ/non_secure/mpu_wrappers_v2_asm.S
                portable/IAR/ARM_CM33_NTZ/non_secure/port.c
                portable/IAR/ARM_CM33_NTZ/non_secure/portasm.h
                portable/IAR/ARM_CM33_NTZ/non_secure/portasm.s
                portable/IAR/ARM_CM33_NTZ/non_secure/portmacro.h
                portable/IAR/ARM_CM33_NTZ/non_secure/portmacrocommon.h
        TOOLCHAINS iar
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )

endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.cm33_trustzone.non_secure)
    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM33/non_secure
                 portable/IAR/ARM_CM33/secure
        TOOLCHAINS iar
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )

    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM33/non_secure
                 portable/GCC/ARM_CM33/secure
        TOOLCHAINS armgcc mcux mdk
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )

    mcux_add_source(
        SOURCES portable/GCC/ARM_CM33/non_secure/mpu_wrappers_v2_asm.c
                portable/GCC/ARM_CM33/non_secure/port.c
                portable/GCC/ARM_CM33/non_secure/portasm.c
                portable/GCC/ARM_CM33/non_secure/portasm.h
                portable/GCC/ARM_CM33/non_secure/portmacro.h
                portable/GCC/ARM_CM33/non_secure/portmacrocommon.h
                portable/GCC/ARM_CM33/secure/secure_context.h
                portable/GCC/ARM_CM33/secure/secure_init.h
        TOOLCHAINS armgcc mcux mdk
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )
    mcux_add_source(
        SOURCES portable/IAR/ARM_CM33/non_secure/mpu_wrappers_v2_asm.S
                portable/IAR/ARM_CM33/non_secure/port.c
                portable/IAR/ARM_CM33/non_secure/portasm.h
                portable/IAR/ARM_CM33/non_secure/portasm.s
                portable/IAR/ARM_CM33/non_secure/portmacro.h
                portable/IAR/ARM_CM33/non_secure/portmacrocommon.h
                portable/IAR/ARM_CM33/secure/secure_context.h
                portable/IAR/ARM_CM33/secure/secure_init.h
        TOOLCHAINS iar
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.cm33_trustzone.secure)

    mcux_add_include(
        INCLUDES portable/GCC/ARM_CM33/secure
                 portable/GCC/ARM_CM33/non_secure
        TOOLCHAINS armgcc mcux mdk
    )

    mcux_add_source(
        SOURCES portable/GCC/ARM_CM33/secure/secure_context.c
                portable/GCC/ARM_CM33/secure/secure_context.h
                portable/GCC/ARM_CM33/secure/secure_context_port.c
                portable/GCC/ARM_CM33/secure/secure_heap.c
                portable/GCC/ARM_CM33/secure/secure_heap.h
                portable/GCC/ARM_CM33/secure/secure_init.c
                portable/GCC/ARM_CM33/secure/secure_init.h
                portable/GCC/ARM_CM33/secure/secure_port_macros.h
        TOOLCHAINS armgcc mcux mdk
        CORES cm33 cm33f
        FPU NO_FPU SP_FPU
    )

    mcux_add_include(
        INCLUDES portable/IAR/ARM_CM33/secure
                 portable/IAR/ARM_CM33/non_secure
        TOOLCHAINS iar
    )

    mcux_add_source(
        SOURCES portable/IAR/ARM_CM33/secure/secure_context.c
                portable/IAR/ARM_CM33/secure/secure_context.h
                portable/IAR/ARM_CM33/secure/secure_context_port_asm.s
                portable/IAR/ARM_CM33/secure/secure_heap.c
                portable/IAR/ARM_CM33/secure/secure_heap.h
                portable/IAR/ARM_CM33/secure/secure_init.c
                portable/IAR/ARM_CM33/secure/secure_init.h
                portable/IAR/ARM_CM33/secure/secure_port_macros.h
        TOOLCHAINS iar
        CORES cm33 cm33f
    )

endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.tfm_ns)
    mcux_add_source(
        SOURCES portable/ThirdParty/GCC/ARM_TFM/os_wrapper_freertos.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.extension)
    mcux_add_include(
        INCLUDES include
    )
    mcux_add_source(
        SOURCES include/freertos_tasks_c_additions.h
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.use_newlib_reentrant)
    mcux_add_include(
        INCLUDES include
    )
    mcux_add_source(
        SOURCES include/newlib-freertos.h
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.use_picolibc_tls)
    mcux_add_include(
        INCLUDES include
    )
    mcux_add_source(
        SOURCES include/picolibc-freertos.h
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.heap_1)
    mcux_add_source(
        SOURCES portable/MemMang/heap_1.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.heap_2)
    mcux_add_source(
        SOURCES portable/MemMang/heap_2.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.heap_3)
    mcux_add_source(
        SOURCES portable/MemMang/heap_3.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.heap_4)
    mcux_add_source(
        SOURCES portable/MemMang/heap_4.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.heap_5)
    mcux_add_source(
        SOURCES portable/MemMang/heap_5.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.mpu_wrappers_v2)
    mcux_add_source(
        SOURCES portable/Common/mpu_wrappers_v2.c
    )
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.mpu_wrappers)
    mcux_add_source(
        SOURCES portable/Common/mpu_wrappers.c
    )
endif ()

if (CONFIG_MCUX_PRJSEG_middleware.freertos-kernel.generated_config)
endif ()

if (CONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config)
    mcux_add_include(
        INCLUDES template
        TARGET_FILES FreeRTOSConfig.h
    )
    mcux_add_include(
        INCLUDES template/ARM_CA9
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES ca7
    )
    mcux_add_include(
        INCLUDES template/ARM_CM0
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm0p
    )
    mcux_add_include(
        INCLUDES template/ARM_CM3_3_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm4 cm3
        DEVICE_IDS K32W041A K32W041AM K32W061 K32W041 QN9090 QN9030
    )
    mcux_add_include(
        INCLUDES template/ARM_CM3_4_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm4 cm3
        DEVICE_IDS MK10DN512xxx10 MK10DX128xxx10 MK10DX256xxx10 MK11DN512xxx5 MK11DX128xxx5 MK11DX256xxx5 MK11DN512xxx5 MK11DX128xxx5 MK11DX256xxx5WS MK11DN512Axxx5 MK11DX128Axxx5 MK11DX256Axxx5 MK11DN512Axxx5 MK11DX128Axxx5 MK11DX256Axxx5WS MK12DN512xxx5 MK12DX128xxx5 MK12DX256xxx5 MK20DN512xxx10 MK20DX128xxx10 MK20DX256xxx10 MK20DN32xxx5 MK20DN64xxx5 MK20DN128xxx5 MK20DX32xxx5 MK20DX64xxx5 MK20DX128xxx5 MK21DN512xxx5 MK21DX128xxx5 MK21DX256xxx5 MK21DN512xxx5 MK21DX128xxx5 MK21DX256xxx5WS MK21DN512Axxx5 MK21DX128Axxx5 MK21DX256Axxx5 MK21DN512Axxx5 MK21DX128Axxx5 MK21DX256Axxx5WS MK22DN512xxx5 MK22DX128xxx5 MK22DX256xxx5 MK30DN512xxx10 MK30DX128xxx10 MK30DX256xxx10 MK40DN512xxx10 MK40DX128xxx10 MK40DX256xxx10 MK50DN512xxx10 MK50DX256xxx10 MK51DN256xxx10 MK51DN512xxx10 MK51DX256xxx10 MK52DN512xxx10 MK53DN512xxx10 MK53DX256xxx10 MK60DN256xxx10 MK60DN512xxx10 MK60DX256xxx10 MKW21D256xxx5 MKW21D512xxx5 MKW21D256xxx5 MKW21D512xxx5WS MKW22D512xxx5 MKW22D512xxx5WS MKW24D512xxx5 MKW24D512xxx5WS
    )
    mcux_add_include(
        INCLUDES template/ARM_CM33_3_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm33 cm33f
        DEVICE_IDS K32W1480xxxA K32W232H KW45B41Z52xxxA KW45B41Z53xxxA KW45B41Z82xxxA KW45B41Z83xxxA KW45Z41052xxxA KW45Z41053xxxA KW45Z41082xxxA KW45Z41083xxxA KW47B42Z83xxxA KW47B42Z96xxxA KW47B42Z97xxxA KW47B42ZB2xxxA KW47B42ZB3xxxA KW47B42ZB6xxxA KW47B42ZB7xxxA KW47Z42082xxxA KW47Z42092xxxA KW47Z420B2xxxA KW47Z420B3xxxA LPC5502 LPC5502CPXXXX LPC5504 LPC5504CPXXXX LPC5506 LPC5506CPXXXX LPC5512 LPC5514 LPC5516 LPC5524 LPC5526 LPC5528 LPC5534 LPC5536 LPC55S04 LPC55S06 LPC55S14 LPC55S16 LPC55S26 LPC55S28 LPC55S36 LPC55S66 LPC55S66 LPC55S69 LPC55S69 MCXW235 MCXW236 MCXN546 MCXN547 MCXN946 MCXN947 MCXW343 MCXW345 MCXW727AxxxA MCXW727CxxxA MCXW727DxxxA MCXW748V MCXW744V MCXW748V MCXW748V MCXW748V MIMX8UD3xxx08 MIMX8UD3xxx10 MCXW748UV MCXW744UV MCXW745UV MCXW743UV MCXA153 MCXA152 MCXA143 MCXA142 MCXA133 MCXA132 MCXL144 MCXL143 MCXL142 MCXA156 MCXA155 MCXA154 MCXA146 MCXA145 MCXA144 MCXA276 MCXA275 MCXA256 MCXA255 MCXA345 MCXA346 MCXA166 MCXA165 MCXN235 MCXN236 MCXL253 MCXL254 MCXL255 MIMX8UD5xxx08 MIMX8UD5xxx10 MIMX8UD7xxx10 MIMX8US3xxx08 MIMX8US3xxx10 MIMX8US5xxx08 MIMX8US5xxx10 MIMX9311xxxxM MIMX9311xxxxK MIMX9312xxxxM MIMX9312xxxxK MIMX9321xxxxM MIMX9321xxxxK MIMX9322xxxxM MIMX9322xxxxK MIMX9331xxxxM MIMX9331xxxxK MIMX9332xxxxM MIMX9332xxxxK MIMX9351xxxxM MIMX9351xxxxK MIMX9352xxxxM MIMX9352xxxxK MIMXRT1181xxxxx MIMXRT1182xxxxx MIMXRT1187xxxxx MIMXRT1189xxxxx MIMXRT1189xxxxA MIMXRT533S MIMXRT555S MIMXRT595S MIMXRT633S MIMXRT685S MIMXRT798S MIMXRT735S MIMXRT758S PN7640 PN7642 RW610 RW612 MCXW716CxxxA MCXW716AxxxA
        FPU NO_FPU SP_FPU
    )
    mcux_add_include(
        INCLUDES template/ARM_CM33_4_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm33 cm33f
        DEVICE_IDS K32W133G K32W1X3H
        FPU NO_FPU SP_FPU
    )
    mcux_add_include(
        INCLUDES template/ARM_CM4F_3_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm4f
        DEVICE_IDS K32W032S1M2xxx LPC54005 LPC54016 LPC54018 LPC54018J2M LPC54018J4M LPC54101J512 LPC54101J256 LPC54102J512 LPC54102J256 LPC54113J256 LPC54113J128 LPC54114J256 LPC54605J512 LPC54605J256 LPC54606J512 LPC54606J256 LPC54607J256 LPC54607J512 LPC54608J512 LPC54616J512 LPC54616J256 LPC54618J512 LPC54628J512 LPC54S005 LPC54S016 LPC54S018 LPC54S018J2M LPC54S018J4M QN908XC
    )
    mcux_add_include(
        INCLUDES template/ARM_CM4F_4_priority_bits
        TARGET_FILES FreeRTOSConfig_Gen.h
        CORES cm4f cm7f
        DEVICE_IDS K32L3A60xxx MCIMX7U3xxxxx MCIMX7U5xxxxx MIMX8DL1xxxFZ MIMX8DX1xxxFZ MIMX8DX2xxxFZ MIMX8DX3xxxFZ MIMX8DX4xxxFZ MIMX8DX5xxxFZ MIMX8DX5xxxDZ MIMX8DX6xxxFZ MIMX8DX6xxxDZ MIMX8MD6xxxJZ MIMX8MD6xxxHZ MIMX8MD7xxxJZ MIMX8MD7xxxHZ MIMX8MM1xxxLZ MIMX8MM1xxxKZ MIMX8MM2xxxLZ MIMX8MM2xxxKZ MIMX8MM3xxxLZ MIMX8MM3xxxKZ MIMX8MM4xxxLZ MIMX8MM4xxxKZ MIMX8MM5xxxLZ MIMX8MM5xxxKZ MIMX8MM6xxxLZ MIMX8MM6xxxKZ MIMX8MQ5xxxJZ MIMX8MQ5xxxHZ MIMX8MQ6xxxJZ MIMX8MQ6xxxHZ MIMX8MQ7xxxJZ MIMX8MQ7xxxHZ MIMX8QM6xxxFF MIMX8QX1xxxFZ MIMX8QX2xxxFZ MIMX8QX3xxxFZ MIMX8QX4xxxFZ MIMX8QX5xxxFZ MIMX8QX5xxxDZ MIMX8QX6xxxFZ MIMX8QX6xxxDZ MIMX8SL1xxxFZ MIMX8UX5xxxFZ MIMX8UX5xxxDZ MIMX8UX6xxxFZ MIMX8UX6xxxDZ MIMXRT1165xxxxx MIMXRT1166xxxxx MIMXRT1173xxxxx MIMXRT1175xxxxx MIMXRT1176xxxxx MK02FN128xxx10 MK02FN64xxx10 MK21FN1M0xxx12 MK21FX512xxx12 MK21FN1M0xxx12 MK21FX512xxx12WS MK21FN1M0Axxx12 MK21FX512Axxx12 MK21FN1M0Axxx12 MK21FX512Axxx12WS MK22FX512xxx12 MK22FN1M0xxx12 MK22FN128xxx10 MK22FN128xxx12 MK22FN256xxx12 MK22FN512xxx12 MK22FN1M0Axxx12 MK22FX512Axxx12 MK24FN1M0xxx12 MK24FN256xxx12 MK26FN2M0xxx18 MK27FN2M0xxx15 MK27FN2M0Axxx15 MK28FN2M0xxx15 MK28FN2M0Axxx15 MK63FN1M0xxx12 MK63FN1M0xxx12WS MK64FN1M0xxx12 MK64FX512xxx12 MK65FN2M0xxx18 MK65FX1M0xxx18 MK65FN2M0xxx18 MK65FX1M0xxx18WS MK66FN2M0xxx18 MK66FX1M0xxx18 MK80FN256xxx15 MK81FN256xxx15 MK82FN256xxx15 MKE14F512xxx16 MKE14F256xxx16 MKE16F512xxx16 MKE16F256xxx16 MKE18F512xxx16 MKE18F256xxx16 MKS20FN128xxx12 MKS20FN256xxx12 MKS22FN128xxx12 MKS22FN256xxx12 MKV30F128xxx10 MKV30F64xxx10 MKV31F128xxx10 MKV31F256xxx12 MKV31F512xxx12 MKV42F128xxx16 MKV42F256xxx16 MKV42F64xxx16 MKV44F128xxx16 MKV44F256xxx16 MKV44F64xxx16 MKV46F128xxx16 MKV46F256xxx16 PKE18F512xxx15 QN908XC K32H844P000xxxx MIMX8ML3xxxLZ MIMX8ML3xxxKZ MIMX8ML4xxxLZ MIMX8ML4xxxKZ MIMX8ML6xxxLZ MIMX8ML6xxxKZ MIMX8ML8xxxLZ MIMX8ML8xxxKZ MIMX8MN1xxxJZ MIMX8MN1xxxIZ MIMX8MN2xxxJZ MIMX8MN2xxxIZ MIMX8MN3xxxJZ MIMX8MN3xxxIZ MIMX8MN4xxxJZ MIMX8MN4xxxIZ MIMX8MN5xxxJZ MIMX8MN5xxxIZ MIMX8MN6xxxJZ MIMX8MN6xxxIZ MIMXRT1011xxxxx MIMXRT1015xxxxx MIMXRT1021xxxxx MIMXRT1024xxxxx MIMXRT1041xxxxB MIMXRT1042xxxxB MIMXRT1051xxxxB MIMXRT1052xxxxB MIMXRT1061xxxxA MIMXRT1061xxxxB MIMXRT1062xxxxA MIMXRT1062xxxxB MIMXRT1064xxxxA MIMXRT1165xxxxx MIMXRT1166xxxxx MIMXRT1171xxxxx MIMXRT1172xxxxx MIMXRT1173xxxxx MIMXRT1175xxxxx MIMXRT1176xxxxx MIMXRT1187xxxxx MIMXRT1189xxxxx MIMXRT1189xxxxA MKV56F1M0xxx24 MKV56F512xxx24 MKV58F1M0xxx24 MKV58F512xxx24 MCXE245 MCXE246 MCXE247 MCXE31B
    )
    mcux_add_source(
        SOURCES template/FreeRTOSConfig.h
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CA9/FreeRTOSConfig_Gen.h
        CORES ca7
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM0/FreeRTOSConfig_Gen.h
        CORES cm0p
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM3_3_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm4 cm3
        DEVICE_IDS K32W041A K32W041AM K32W061 K32W041 QN9090 QN9030
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM3_4_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm4 cm3
        DEVICE_IDS MK10DN512xxx10 MK10DX128xxx10 MK10DX256xxx10 MK11DN512xxx5 MK11DX128xxx5 MK11DX256xxx5 MK11DN512xxx5 MK11DX128xxx5 MK11DX256xxx5WS MK11DN512Axxx5 MK11DX128Axxx5 MK11DX256Axxx5 MK11DN512Axxx5 MK11DX128Axxx5 MK11DX256Axxx5WS MK12DN512xxx5 MK12DX128xxx5 MK12DX256xxx5 MK20DN512xxx10 MK20DX128xxx10 MK20DX256xxx10 MK20DN32xxx5 MK20DN64xxx5 MK20DN128xxx5 MK20DX32xxx5 MK20DX64xxx5 MK20DX128xxx5 MK21DN512xxx5 MK21DX128xxx5 MK21DX256xxx5 MK21DN512xxx5 MK21DX128xxx5 MK21DX256xxx5WS MK21DN512Axxx5 MK21DX128Axxx5 MK21DX256Axxx5 MK21DN512Axxx5 MK21DX128Axxx5 MK21DX256Axxx5WS MK22DN512xxx5 MK22DX128xxx5 MK22DX256xxx5 MK30DN512xxx10 MK30DX128xxx10 MK30DX256xxx10 MK40DN512xxx10 MK40DX128xxx10 MK40DX256xxx10 MK50DN512xxx10 MK50DX256xxx10 MK51DN256xxx10 MK51DN512xxx10 MK51DX256xxx10 MK52DN512xxx10 MK53DN512xxx10 MK53DX256xxx10 MK60DN256xxx10 MK60DN512xxx10 MK60DX256xxx10 MKW21D256xxx5 MKW21D512xxx5 MKW21D256xxx5 MKW21D512xxx5WS MKW22D512xxx5 MKW22D512xxx5WS MKW24D512xxx5 MKW24D512xxx5WS
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM33_3_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm33 cm33f
        DEVICE_IDS K32W1480xxxA K32W232H KW45B41Z52xxxA KW45B41Z53xxxA KW45B41Z82xxxA KW45B41Z83xxxA KW45Z41052xxxA KW45Z41053xxxA KW45Z41082xxxA KW45Z41083xxxA KW47B42Z83xxxA KW47B42Z96xxxA KW47B42Z97xxxA KW47B42ZB2xxxA KW47B42ZB3xxxA KW47B42ZB6xxxA KW47B42ZB7xxxA KW47Z42082xxxA KW47Z42092xxxA KW47Z420B2xxxA KW47Z420B3xxxA LPC5502 LPC5502CPXXXX LPC5504 LPC5504CPXXXX LPC5506 LPC5506CPXXXX LPC5512 LPC5514 LPC5516 LPC5524 LPC5526 LPC5528 LPC5534 LPC5536 LPC55S04 LPC55S06 LPC55S14 LPC55S16 LPC55S26 LPC55S28 LPC55S36 LPC55S66 LPC55S66 LPC55S69 LPC55S69 MCXW235 MCXW236 MCXN546 MCXN547 MCXN946 MCXN947 MCXW343 MCXW345 MCXW727AxxxA MCXW727CxxxA MCXW727DxxxA MCXW748V MCXW744V MCXW748V MCXW748V MCXW748V MIMX8UD3xxx08 MIMX8UD3xxx10 MCXW748UV MCXW744UV MCXW745UV MCXW743UV MCXA153 MCXA152 MCXA143 MCXA142 MCXA133 MCXA132 MCXL144 MCXL143 MCXL142 MCXA156 MCXA155 MCXA154 MCXA146 MCXA145 MCXA144 MCXA276 MCXA275 MCXA256 MCXA255 MCXA345 MCXA346 MCXA166 MCXA165 MCXN235 MCXN236 MCXL253 MCXL254 MCXL255 MIMX8UD5xxx08 MIMX8UD5xxx10 MIMX8UD7xxx10 MIMX8US3xxx08 MIMX8US3xxx10 MIMX8US5xxx08 MIMX8US5xxx10 MIMX9311xxxxM MIMX9311xxxxK MIMX9312xxxxM MIMX9312xxxxK MIMX9321xxxxM MIMX9321xxxxK MIMX9322xxxxM MIMX9322xxxxK MIMX9331xxxxM MIMX9331xxxxK MIMX9332xxxxM MIMX9332xxxxK MIMX9351xxxxM MIMX9351xxxxK MIMX9352xxxxM MIMX9352xxxxK MIMXRT1181xxxxx MIMXRT1182xxxxx MIMXRT1187xxxxx MIMXRT1189xxxxx MIMXRT1189xxxxA MIMXRT533S MIMXRT555S MIMXRT595S MIMXRT633S MIMXRT685S MIMXRT798S MIMXRT735S MIMXRT758S PN7640 PN7642 RW610 RW612 MCXW716CxxxA MCXW716AxxxA
        FPU NO_FPU SP_FPU
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM33_4_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm33 cm33f
        DEVICE_IDS K32W133G K32W1X3H
        FPU NO_FPU SP_FPU
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM4F_3_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm4f
        DEVICE_IDS K32W032S1M2xxx LPC54005 LPC54016 LPC54018 LPC54018J2M LPC54018J4M LPC54101J512 LPC54101J256 LPC54102J512 LPC54102J256 LPC54113J256 LPC54113J128 LPC54114J256 LPC54605J512 LPC54605J256 LPC54606J512 LPC54606J256 LPC54607J256 LPC54607J512 LPC54608J512 LPC54616J512 LPC54616J256 LPC54618J512 LPC54628J512 LPC54S005 LPC54S016 LPC54S018 LPC54S018J2M LPC54S018J4M QN908XC
        CONFIG TRUE
    )
    mcux_add_source(
        SOURCES template/ARM_CM4F_4_priority_bits/FreeRTOSConfig_Gen.h
        CORES cm4f cm7f
        DEVICE_IDS K32L3A60xxx MCIMX7U3xxxxx MCIMX7U5xxxxx MIMX8DL1xxxFZ MIMX8DX1xxxFZ MIMX8DX2xxxFZ MIMX8DX3xxxFZ MIMX8DX4xxxFZ MIMX8DX5xxxFZ MIMX8DX5xxxDZ MIMX8DX6xxxFZ MIMX8DX6xxxDZ MIMX8MD6xxxJZ MIMX8MD6xxxHZ MIMX8MD7xxxJZ MIMX8MD7xxxHZ MIMX8MM1xxxLZ MIMX8MM1xxxKZ MIMX8MM2xxxLZ MIMX8MM2xxxKZ MIMX8MM3xxxLZ MIMX8MM3xxxKZ MIMX8MM4xxxLZ MIMX8MM4xxxKZ MIMX8MM5xxxLZ MIMX8MM5xxxKZ MIMX8MM6xxxLZ MIMX8MM6xxxKZ MIMX8MQ5xxxJZ MIMX8MQ5xxxHZ MIMX8MQ6xxxJZ MIMX8MQ6xxxHZ MIMX8MQ7xxxJZ MIMX8MQ7xxxHZ MIMX8QM6xxxFF MIMX8QX1xxxFZ MIMX8QX2xxxFZ MIMX8QX3xxxFZ MIMX8QX4xxxFZ MIMX8QX5xxxFZ MIMX8QX5xxxDZ MIMX8QX6xxxFZ MIMX8QX6xxxDZ MIMX8SL1xxxFZ MIMX8UX5xxxFZ MIMX8UX5xxxDZ MIMX8UX6xxxFZ MIMX8UX6xxxDZ MIMXRT1165xxxxx MIMXRT1166xxxxx MIMXRT1173xxxxx MIMXRT1175xxxxx MIMXRT1176xxxxx MK02FN128xxx10 MK02FN64xxx10 MK21FN1M0xxx12 MK21FX512xxx12 MK21FN1M0xxx12 MK21FX512xxx12WS MK21FN1M0Axxx12 MK21FX512Axxx12 MK21FN1M0Axxx12 MK21FX512Axxx12WS MK22FX512xxx12 MK22FN1M0xxx12 MK22FN128xxx10 MK22FN128xxx12 MK22FN256xxx12 MK22FN512xxx12 MK22FN1M0Axxx12 MK22FX512Axxx12 MK24FN1M0xxx12 MK24FN256xxx12 MK26FN2M0xxx18 MK27FN2M0xxx15 MK27FN2M0Axxx15 MK28FN2M0xxx15 MK28FN2M0Axxx15 MK63FN1M0xxx12 MK63FN1M0xxx12WS MK64FN1M0xxx12 MK64FX512xxx12 MK65FN2M0xxx18 MK65FX1M0xxx18 MK65FN2M0xxx18 MK65FX1M0xxx18WS MK66FN2M0xxx18 MK66FX1M0xxx18 MK80FN256xxx15 MK81FN256xxx15 MK82FN256xxx15 MKE14F512xxx16 MKE14F256xxx16 MKE16F512xxx16 MKE16F256xxx16 MKE18F512xxx16 MKE18F256xxx16 MKS20FN128xxx12 MKS20FN256xxx12 MKS22FN128xxx12 MKS22FN256xxx12 MKV30F128xxx10 MKV30F64xxx10 MKV31F128xxx10 MKV31F256xxx12 MKV31F512xxx12 MKV42F128xxx16 MKV42F256xxx16 MKV42F64xxx16 MKV44F128xxx16 MKV44F256xxx16 MKV44F64xxx16 MKV46F128xxx16 MKV46F256xxx16 PKE18F512xxx15 QN908XC K32H844P000xxxx MIMX8ML3xxxLZ MIMX8ML3xxxKZ MIMX8ML4xxxLZ MIMX8ML4xxxKZ MIMX8ML6xxxLZ MIMX8ML6xxxKZ MIMX8ML8xxxLZ MIMX8ML8xxxKZ MIMX8MN1xxxJZ MIMX8MN1xxxIZ MIMX8MN2xxxJZ MIMX8MN2xxxIZ MIMX8MN3xxxJZ MIMX8MN3xxxIZ MIMX8MN4xxxJZ MIMX8MN4xxxIZ MIMX8MN5xxxJZ MIMX8MN5xxxIZ MIMX8MN6xxxJZ MIMX8MN6xxxIZ MIMXRT1011xxxxx MIMXRT1015xxxxx MIMXRT1021xxxxx MIMXRT1024xxxxx MIMXRT1041xxxxB MIMXRT1042xxxxB MIMXRT1051xxxxB MIMXRT1052xxxxB MIMXRT1061xxxxA MIMXRT1061xxxxB MIMXRT1062xxxxA MIMXRT1062xxxxB MIMXRT1064xxxxA MIMXRT1165xxxxx MIMXRT1166xxxxx MIMXRT1171xxxxx MIMXRT1172xxxxx MIMXRT1173xxxxx MIMXRT1175xxxxx MIMXRT1176xxxxx MIMXRT1187xxxxx MIMXRT1189xxxxx MIMXRT1189xxxxA MKV56F1M0xxx24 MKV56F512xxx24 MKV58F1M0xxx24 MKV58F512xxx24 MCXE245 MCXE246 MCXE247 MCXE31B
        CONFIG TRUE
    )
endif ()
