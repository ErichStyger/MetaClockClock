Files in this folder implement low-level transport layer of FreeMASTER communication driver.

There are different driver implementations for each kind of communication module (UART, LPUART,
SCI, FlexCAN, MCAN, msCAN, ..). Also there are multiple implementations depending on what SDK
is used to access peripheral registers and other processor resources.

The top level folder group the drivers by the SDK selected:

 - ampxsdk/  .. AMP SDK targeting the S32K family of microcontrollers
 - mcuxsdk/  .. MCUXpresso SDK targeting Kinetis, LPC and i.MX-RT families (http://mcuxpresso.nxp.com)
 - dreg/     .. no dependency on any SDK, the drivers in this folder use direct register access
 - zephyr/   .. FreeMASTER drivers built on top of Zephyr abstract uart, can and other drivers

 Use the FMSTR_SERIAL_DRV or FMSTR_CAN_DRV configuration option in application-specific freemaster_cfg.h
 file to select what low-level driver is actually linked and used in your application.
