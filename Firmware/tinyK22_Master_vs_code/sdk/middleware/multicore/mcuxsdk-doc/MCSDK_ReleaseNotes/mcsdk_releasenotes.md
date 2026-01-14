# Multicore SDK (MCSDK) Release Notes

## Overview

These are the release notes for the NXP Multicore Software Development Kit (MCSDK) version 25.06.00.  
This software package contains components for efficient work with multicore devices as well as for the  
multiprocessor communication.

## What is new

- eRPC [CHANGELOG](https://github.com/EmbeddedRPC/erpc/blob/release/25.06.00/CHANGELOG.md)
- RPMsg-Lite [CHANGELOG](https://github.com/nxp-mcuxpresso/rpmsg-lite/blob/release/25.06.00/CHANGELOG.md)
- MCMgr [CHANGELOG](https://github.com/nxp-mcuxpresso/mcux-mcmgr/blob/release/25.06.00/CHANGELOG.md)

- Supported evaluation boards (multicore examples):
  - LPCXpresso55S69
  - FRDM-K32L3A6
  - MIMXRT1170-EVKB
  - MIMXRT1160-EVK
  - MIMXRT1180-EVK
  - MCX-N5XX-EVK
  - MCX-N9XX-EVK
  - FRDM-MCXN947
  - MIMXRT700-EVK
  - KW47-EVK
  - KW47-LOC
  - FRDM-MCXW72
  - MCX-W72-EVK

- Supported evaluation boards (multiprocessor examples):
  - LPCXpresso55S36
  - FRDM-K22F
  - FRDM-K32L2B
  - MIMXRT685-EVK
  - MIMXRT1170-EVKB
  - MIMXRT1180
  - FRDM-MCXN236
  - FRDM-MCXC242
  - FRDM-MCXC444
  - MCX-N9XX-EVK
  - FRDM-MCXN947
  - MIMXRT700-EVK

## Development tools

The Multicore SDK (MCSDK) was compiled and tested with development tools referred in: [Development tools](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/gsd/installation.html#toolchain)

## Release contents

This table describes the release contents. Not all MCUXpresso SDK packages contain the whole set of these components.

| Deliverable                              | Location                                                        |
| ---------------------------------------- | --------------------------------------------------------------- |
| Multicore SDK location `<MCSDK_dir>`     | `<MCUXpressoSDK_install_dir>/middleware/multicore/`             |
| Documentation                            | `<MCSDK_dir>/mcuxsdk-doc/`                                      |
| Embedded Remote Procedure Call component | `<MCSDK_dir>/erpc/`                                             |
| Multicore Manager component              | `<MCSDK_dir>/mcmgr/`                                            |
| RPMsg-Lite                               | `<MCSDK_dir>/rpmsg_lite/`                                       |
| Multicore demo applications              | `<MCUXpressoSDK_install_dir>/examples/multicore_examples/`      |
| Multiprocessor demo applications         | `<MCUXpressoSDK_install_dir>/examples/multiprocessor_examples/` |

## Multicore SDK release overview

Together, the Multicore SDK (MCSDK) and the MCUXpresso SDK (SDK) form a framework for the development of software for NXP multicore devices. The MCSDK release consists of the following elementary software components for multicore:

- Embedded Remote Procedure Call (eRPC)
- Multicore Manager (MCMGR) - included just in SDK for multicore devices
- Remote Processor Messaging - Lite (RPMsg-Lite) - included just in SDK for multicore devices

The MCSDK is also accompanied with documentation and several multicore and multiprocessor demo applications.

## Demo applications

The multicore demo applications demonstrate the usage of the MCSDK software components on supported multicore development boards.  
The following multicore demo applications are located together with other MCUXpresso SDK examples in  
the _`<MCUXpressoSDK_install_dir>/examples/multicore_examples`_ subdirectories.

- erpc_matrix_multiply_mu
- erpc_matrix_multiply_mu_rtos
- erpc_matrix_multiply_rpmsg
- erpc_matrix_multiply_rpmsg_rtos
- erpc_two_way_rpc_rpmsg_rtos
- freertos_message_buffers
- hello_world
- multicore_manager
- rpmsg_lite_pingpong
- rpmsg_lite_pingpong_rtos
- rpmsg_lite_pingpong_tzm

The eRPC multicore component can be leveraged for inter-processor communication and remote procedure calls between SoCs / development boards.  
The following multiprocessor demo applications are located together with other MCUXpresso SDK examples in  
the _`<MCUXpressoSDK_install_dir>/examples/multiprocessor_examples`_ subdirectories.

- erpc_client_matrix_multiply_spi
- erpc_server_matrix_multiply_spi
- erpc_client_matrix_multiply_uart
- erpc_server_matrix_multiply_uart
- erpc_server_dac_adc
- erpc_remote_control
