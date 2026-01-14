/*
 * Copyright (c) 2015 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BL_RELIABLE_UPDATE_H__
#define __BL_RELIABLE_UPDATE_H__

#include <stdbool.h>
#include <stdint.h>
#include "property.h"

//! @addtogroup reliable_update
//! @{

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#if !defined(FOUR_CHAR_CODE)
#define FOUR_CHAR_CODE(a, b, c, d) (((d) << 24) | ((c) << 16) | ((b) << 8) | ((a)))
#endif

#define BOOTLOADER_META_TAG FOUR_CHAR_CODE('B', 'L', 'M', 'T')
#define SWAP_META_TAG FOUR_CHAR_CODE('S', 'W', 'A', 'P')
#define IMG_HDR_TAG FOUR_CHAR_CODE('I', 'H', 'D', 'R')

#define BL_IS_HARDWARE_SWAP_ENABLED (BL_FEATURE_HARDWARE_SWAP_UPDATE)

typedef enum _reliable_update_option
{
    kReliableUpdateOption_Normal = 0x0U, //!< Normal option, Update main application only if it is invalid
    kReliableUpdateOption_Swap = 0x1U,   //!< Swap option, Always update main application
} reliable_update_option_t;

typedef enum _specified_application_type
{
    kSpecifiedApplicationType_Main = 0x0U,
    kSpecifiedApplicationType_Backup = 0x1U,
} specified_application_type_t;

//! @brief Application CRC check status codes.
enum _reliable_update_status
{
    kStatus_ReliableUpdateSuccess = MAKE_STATUS(kStatusGroup_ReliableUpdate, 0), //!< Reliable Update succeeded.
    kStatus_ReliableUpdateFail = MAKE_STATUS(kStatusGroup_ReliableUpdate, 1),    //!< Reliable Update failed.
    kStatus_ReliableUpdateInactive =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 2), //!< Reliable Update Feature is inactive.
    kStatus_ReliableUpdateBackupApplicationInvalid =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 3), //!< Backup Application is invalid
    kStatus_ReliableUpdateStillInMainApplication =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 4), //!< Next boot will be still in Main Application
    kStatus_ReliableUpdateSwapSystemNotReady =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 5), //!< Cannot swap flash by default because swap system is not ready
    kStatus_ReliableUpdateBackupBootloaderNotReady =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 6), //!< Cannot swap flash because there is no valid backup bootloader
    kStatus_ReliableUpdateSwapIndicatorAddressInvalid =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 7), //!< Cannot swap flash because provided swap indicator is invalid
    kStatus_ReliableUpdateSwapSystemNotAvailable = MAKE_STATUS(kStatusGroup_ReliableUpdate, 8),
    kStatus_ReliableUpdateSwapTest =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 9), //!< Reliable update is under test mode
    kStatus_ReliableUpdateIntegrityCheckFailed =
        MAKE_STATUS(kStatusGroup_ReliableUpdate, 10), //!<  Image integrity check failed

};

#ifndef BL_OTA_BOOTLOADER_USER_DEFINED_PARTITION_NUM
#define BL_OTA_BOOTLOADER_USER_DEFINED_PARTITION_NUM 4
#endif
//!@brief Flash partition index
enum
{
    kPartition_Bootloader, //!< Partition Index of the Bootloader
    kPartition_Primary,    //!< Partition Index of the Primary application
    kPartition_Secondary,  //!< Partition Index of the Secondary application
    kPartition_UserData,   //!< Partition Index of the User Data
    kPartition_Max = kPartition_UserData + BL_OTA_BOOTLOADER_USER_DEFINED_PARTITION_NUM,
};

//!@brief Image Index defintions
enum
{
    kImageIndex_Primary,   //!< Primary Image Index
    kImageIndex_Secondary, //!< Secondary image Index
    kImageIndex_Max
};

//!@brief Swap Type definitions
enum
{
    kSwapType_None,         //!< Default value when there is no upgradable image
    kSwapType_ReadyForTest, //!< The application needs to program this when finishing the update operation
    kSwapType_Test,         //!< The bootloader needs to set the swap_info to this value before running the test image
    kSwapType_Permanent,    //!< The application needs to set the swap_info to this type when the self-test is okay
    kSwapType_Rollback,     //!< Roll back to the first valid application
    kSwapType_Fail,         //!< Swap failed
    kSwapType_Fatal,        //!< Fatal error happened during swapping
    kSwapType_Max,
};

//!@brief Partition information table definitions
typedef struct
{
    uint32_t start;        //!< Start address of the partition
    uint32_t size;         //!< Size of the partition
    uint32_t image_state;  //!< Active/ReadyForTest/UnderTest
    uint32_t attribute;    //!< Partition Attribute - Defined for futher use
    uint32_t reserved[12]; //!< Reserved for future use
} partition_t;

//!@brief Bootloader meta data structure
typedef struct
{
    struct
    {
        uint32_t wdTimeout;
        uint32_t periphDetectTimeout;
        uint32_t enabledPeripherals;
        uint32_t reserved[12];
    } features;

    partition_t partition[kPartition_Max];
    uint32_t meta_version;
    uint32_t patition_entries;
    uint32_t reserved0;
    uint32_t tag;
} bootloader_meta_t;

// Image header is platform specific
typedef struct
{
    uint32_t tag;        //!< Tag of the boot image header
    uint32_t load_addr;  //!< Load address of the image
    uint32_t image_type; //!< Image Type
    uint32_t image_size; //!< Image size
    uint32_t algorithm;  //!< Algorithm for Image validity check
    // Note: considering the address alignment requirement on the vector table, take RT1060 for example, the vector
    // table must be 1KB aligned
    //      the header_size can be set to 1KB in this case, and the actual image will follow the header
    uint32_t header_size;   //!< Image Header size
    uint32_t image_version; //!< Image version
    uint32_t reserved[1];   //!< Reserved for future use
    uint32_t checksum[32];  //!< Checksum field, can be CRC checksum, SHA256 checksum, etc
    uint32_t padding[];     //!< Padding field
} boot_image_header_t;

typedef boot_image_header_t image_header_t;

//!@brief Image Copy state
enum
{
    kCopyStatus_Inactive = 0,      //!< Copy operation is inactive
    kCopyStatus_InProgress = 0x66, //!< Copy operation is in progress
    kCopyStatus_Done = 0xf0,       //!< Copy operation was done
    kCopyStatus_Fail = 0x33,       //!< Copy operation failed
};

//!@brief Image Swapping stage
enum
{
    kSwapStage_NotStarted,     //!< Image swapping is not started
    kSwapStage_A_to_B_Scratch, //!< Image is swapping from Primary Partition to secondary Partition scratch area
    kSwapStage_B_to_A,         //!< Image is swapping from secondary partition to primary partition
    kSwapStage_Done,           //!< Image swapping was done
    kSwapStage_Max,
};

//!@brief Confirm state
enum
{
    kImageConfirm_Okay = 0xf0,    //!< Confirmation is okay
    kImageConfirm_Invalid = 0x33, //!< Confirmation is invalid
};

//!@brief Swap progress definitions
typedef struct
{
    uint32_t swap_offset;    //!< Current swap offset
    uint32_t scratch_size;   //!< The scratch area size during current swapping
    uint32_t swap_status;    // 1 : A -> B scratch,  2 : B -> A
    uint32_t remaining_size; //!< Remaining size to be swapped
} swap_progress_t;

typedef struct
{
    uint32_t size;
    uint32_t active_flag;
} image_info_t;

//!@brief Swap meta information, used for the swapping operation
typedef struct
{
    image_info_t image_info[2]; //!< Image info table
#if !defined(BL_FEATURE_HARDWARE_SWAP_SUPPORTED) || (BL_FEATURE_HARDWARE_SWAP_SUPPORTED == 0)
    swap_progress_t swap_progress; //!< Swap progress
#endif
    uint32_t swap_type;    //!< Swap type
    uint32_t copy_status;  //!< Copy status
    uint32_t confirm_info; //!< Confirm Information
    uint32_t meta_version; //!< Meta version
    uint32_t reserved[7];  //!< Reserved for future use
    uint32_t tag;          //!< Swap meta tag
} swap_meta_t;

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

#if __cplusplus
extern "C"
{
#endif

    //!@brief Load Swap Meta
    status_t load_swap_meta(swap_meta_t *swap_meta);

    //!@brief Load Boot Meta
    status_t load_boot_meta(bootloader_meta_t *boot_meta);

    //!@brief Do swap meta check
    status_t swap_meta_check(const swap_meta_t *meta);

    //!@brief Do boot meta check
    status_t boot_meta_check(const bootloader_meta_t *boot_meta);

    //!@brief Get Image header from the specified partition
    status_t get_image_header(uint32_t partition_id, image_header_t *img_hdr);

    //!@brief Do boot image check
    status_t boot_image_check(image_header_t *hdr, int32_t partition_id);

    //!@brief Get the update partition information
    status_t get_update_partition_info(partition_t *partition);

    //!@brief Update the image state
    status_t update_image_state(uint32_t state);

    //!@brief Perform reliable update as requested
    void bootloader_reliable_update_as_requested(reliable_update_option_t option, uint32_t address);
    //@}

#if __cplusplus
}
#endif

//! @}

#endif // __BL_RELIABLE_UPDATE_H__
