# Changelog FatFs

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)

## [R0.15_rev0]
- Upgraded to version 0.15
- Applied patches from http://elm-chan.org/fsw/ff/patches.html

## [R0.14b_rev1]
- Applied patches from http://elm-chan.org/fsw/ff/patches.html

## [R0.14b_rev0]
- Upgraded to version 0.14b

## [R0.14a_rev0]
- Upgraded to version 0.14a
- Applied patch ff14a_p1.diff and ff14a_p2.diff

## [R0.14_rev0]
- Upgraded to version 0.14
- Applied patch ff14_p1.diff and ff14_p2.diff

## [R0.13c_rev0]
- Upgraded to version 0.13c
- Applied patches ff_13c_p1.diff,ff_13c_p2.diff, ff_13c_p3.diff and ff_13c_p4.diff.

## [R0.13b_rev0]
- Upgraded to version 0.13b

## [R0.13a_rev0]
- Upgraded to version 0.13a. Added patch ff_13a_p1.diff.

## [R0.12c_rev1]
- Add NAND disk support.

## [R0.12c_rev0]
- Upgraded to version 0.12c and applied patches ff_12c_p1.diff and ff_12c_p2.diff.

## [R0.12b_rev0]
- Upgraded to version 0.12b.

## [R0.11a]
- Added glue functions for low-level drivers (SDHC, SDSPI, RAM, MMC). Modified diskio.c.
- Added RTOS wrappers to make FatFs thread safe. Modified syscall.c.
- Renamed ffconf.h to ffconf_template.h. Each application should contain its own ffconf.h.
- Included ffconf.h into diskio.c to enable the selection of physical disk from ffconf.h by macro definition.
- Conditional compilation of physical disk interfaces in diskio.c.
