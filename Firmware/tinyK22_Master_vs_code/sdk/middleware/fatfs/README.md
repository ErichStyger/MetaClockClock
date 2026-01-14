# MCUXpresso SDK : mcuxsdk-middleware-fatfs

## Overview
This repository is for FatFs middleware delivery and it contains the components officially provided in NXP MCUXpresso SDK. This repository is part of the MCUXpresso SDK overall delivery which is composed of several sub-repositories/projects. Navigate to the top/parent repository (mcuxsdk-manifests) for the complete delivery of MCUXpresso SDK.

## Documentation
Overall details can be reviewed here: [MCUXpresso SDK Online Documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/introduction/README.html)

Visit [FatFs - Documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/middleware/fatfs/index.html) to review details on the contents in this sub-repo.

## Setup
Instructions on how to install the MCUXpresso SDK provided from GitHub via west manifest [Getting Started with SDK - Detailed Installation Instructions](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/gsd/installation.html#installation)

## Contribution
Contributions are not currently accepted. Guidelines to contribute will be posted in the future.

## Repo Specific Content
This is MCUXpresso SDK fork of FatFs (FAT file system created by ChaN).
Official documentation is available at [http://elm-chan.org/fsw/ff/](http://elm-chan.org/fsw/ff/)

MCUXpresso version is extending original content by following hardware specific porting layers:

 - mmc_disk
 - nand_disk
 - ram_disk
 - sd_disk
 - sdspi_disk
 - usb_disk
