# MCUXpresso SDK : mcuxsdk-middleware-multicore

## Overview
This repository is for MCUXpresso SDK multicore middleware delivery and it contains multicore components officially provided in NXP MCUXpresso SDK. This repository is part of the MCUXpresso SDK overall delivery which is composed of several sub-repositories/projects. Navigate to the top/parent repository [mcuxsdk](https://github.com/nxp-mcuxpresso/mcuxsdk-manifests/) for the complete delivery of MCUXpresso SDK to be able to build and run multicore examples that are based on mcux-sdk-middleware-multicore components.

## Documentation
Overall details can be reviewed here: [MCUXpresso SDK Online Documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/introduction/README.html)

Visit [Multicore - Documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/middleware/multicore/index.html) to review details on the contents in this sub-repo.

## Setup
Instructions on how to install the MCUXpresso SDK provided from GitHub via west manifest [Getting Started with SDK - Detailed Installation Instructions](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/gsd/installation.html#installation)

## Contribution
We welcome and encourage the community to submit patches directly to the mcuxsdk-middleware-multicore project placed on github. Contributing can be managed via pull-requests. Before a pull-request is created the code should be tested and properly formatted.

---------------------------------
## Directory Structure

* **remoteproc** - General source code used for remote cores (from the Linux perspective) to allow easy remote core applications deployment/start/debug via the remoteproc Linux framework.

* **tools** - Folder contains tools and utilities for multicore like prebuilt versions of erpcgen and erpcsniffer.

* **mcuxsdk-doc** - Multicore SDK documentation
