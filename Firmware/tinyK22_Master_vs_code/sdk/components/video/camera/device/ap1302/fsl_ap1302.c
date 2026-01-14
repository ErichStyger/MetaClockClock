/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_video_common.h"
#include "fsl_camera.h"
#include "fsl_camera_device.h"
#include "fsl_video_i2c.h"
#include "fsl_ap1302.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AP1302_DelayMs                  VIDEO_DelayMs

#define AP1302_I2C_ADDR                 (uint8_t)(0x3CU)
#define AP1302_CHIP_ID                  (uint16_t)(0x265U)
#define AP1302_FW_WINDOW_OFFSET         (uint32_t)(0x8000U)
#define AP1302_FW_WINDOW_SIZE           (uint32_t)(0x2000U)

/* AP1302 registers */
#define AP1302_CHIP_VERSION             (uint16_t)(0x0000U)
#define AP1302_CHIP_REV                 (uint16_t)(0x0050U)
#define AP1302_MF_ID                    (uint16_t)(0x0004U)
#define AP1302_ERROR                    (uint16_t)(0x0006U)
#define AP1302_CTRL                     (uint16_t)(0x1000U)
#define AP1302_DZ_TGT_FCT               (uint16_t)(0x1010U)
#define AP1302_SFX_MODE                 (uint16_t)(0x1016U)
#define AP1302_SS_HEAD_PT0              (uint16_t)(0x1174U)
#define AP1302_ATOMIC                   (uint16_t)(0x1184U)
#define AP1302_PREVIEW_WIDTH            (uint16_t)(0x2000U)
#define AP1302_PREVIEW_HEIGHT           (uint16_t)(0x2002U)
#define AP1302_AE_BV_OFF                (uint16_t)(0x5014U)
#define AP1302_AE_BV_BIAS               (uint16_t)(0x5016U)
#define AP1302_AWB_CTRL                 (uint16_t)(0x5100U)
#define AP1302_FLICK_CTRL               (uint16_t)(0x5440U)
#define AP1302_SCENE_CTRL               (uint16_t)(0x5454U)
#define AP1302_BOOTDATA_STAGE           (uint16_t)(0x6002U)
#define AP1302_SENSOR_SELECT            (uint16_t)(0x600CU)
#define AP1302_SYS_START                (uint16_t)(0x601AU)
#define AP1302_BOOTDATA_CHECKSUM        (uint16_t)(0x6134U)
#define AP1302_SIP_CRC                  (uint16_t)(0xF052U)
#define AP1302_PLL_DIV                  (uint16_t)(0x602CU)

#define AP1302_WriteReg(handle, reg, size, val)                                                       \
    VIDEO_I2C_WriteReg(AP1302_I2C_ADDR, kVIDEO_RegAddr16Bit, (reg), (video_reg_width_t)(size), (val), \
                  ((ap1302_resource_t *)((handle)->resource))->i2cSendFunc)

#define AP1302_ReadReg(handle, reg, size, val)                                                       \
    VIDEO_I2C_ReadReg(AP1302_I2C_ADDR, kVIDEO_RegAddr16Bit, (reg), (video_reg_width_t)(size), (val), \
                 ((ap1302_resource_t *)((handle)->resource))->i2cReceiveFunc)

#define AP1302_CHECK_RET(x)            \
    do                                 \
    {                                  \
        status = (x);                  \
        if (kStatus_Success != status) \
        {                              \
            return status;             \
        }                              \
    } while (false)

typedef struct _ap1302_reg_val
{
    uint16_t regAddr; /*!<Register address. */
    uint8_t  size;    /*!<Register size. */
    uint32_t regVal;  /*!<Register value. */
} ap1302_reg_val_t;

struct ap1302_firmware {
    uint32_t crc;
    uint32_t checksum;
    uint32_t pll_init_size;
    uint32_t total_size;
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t AP1302_Init(camera_device_handle_t *handle, const camera_config_t *config);

status_t AP1302_Deinit(camera_device_handle_t *handle);

status_t AP1302_Start(camera_device_handle_t *handle);

/*******************************************************************************
 * Variables
 ******************************************************************************/

const camera_device_operations_t ap1302_ops = {
    .init     = AP1302_Init,
    .deinit   = AP1302_Deinit,
    .start    = AP1302_Start,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * When loading firmware, host writes firmware data from address 0x8000.
 * When the address reaches 0x9FFF, the next address should return to 0x8000.
 * This function handles this address window and load firmware data to AP1302.
 * win_pos indicates the offset within this window. Firmware loading procedure
 * may call this function several times. win_pos records the current position
 * that has been written to.
 */
static status_t AP1302_WriteFW(camera_device_handle_t *handle, uint8_t *fw_data, uint32_t *win_pos, uint32_t len)
{
    uint32_t pos;
    uint32_t sub_len;
    status_t status;

    for (pos = 0; pos < len; pos += sub_len)
    {
        if (len - pos < AP1302_FW_WINDOW_SIZE - *win_pos)
        {
            sub_len = len - pos;
        }
        else
        {
            sub_len = AP1302_FW_WINDOW_SIZE - *win_pos;
        }
        /* send firmware data into ap1302 memmory */
        status = ((ap1302_resource_t *)((handle)->resource))->i2cSendFunc32bit
        (AP1302_I2C_ADDR, (*win_pos + AP1302_FW_WINDOW_OFFSET), kVIDEO_RegAddr16Bit, fw_data + pos, sub_len);
        if (kStatus_Success != status)
        {
            return status;
        }
        *win_pos += sub_len;

        if (*win_pos >= AP1302_FW_WINDOW_SIZE)
        {
            *win_pos = 0;
        }
    }
    return status;
}

status_t AP1302_Init(camera_device_handle_t *handle, const camera_config_t *config)
{
    status_t status;
    uint16_t reg_val = 0;
    uint32_t win_pos = 0;
    uint8_t *fw_data = NULL;
    uint32_t reg_val32 = 0;
    ap1302_resource_t *resource = (ap1302_resource_t *)(handle->resource);
    struct ap1302_firmware_t *ap1302_fw;

    /* Verify the configuration and maxium support 4 data lanes. */
    if ((kCAMERA_InterfaceMIPI == config->interface) && (4U < config->csiLanes))
    {
        return kStatus_InvalidArgument;
    }

    /* Power Up sensor according into sequence */
    ((ap1302_resource_t *)(handle->resource))->powerUp(true);

    resource->pullResetPin(false);
    VIDEO_DelayMs(5);

    resource->pullResetPin(true);
    VIDEO_DelayMs(20);

    /* Check chip id */
    AP1302_CHECK_RET(AP1302_ReadReg(handle, AP1302_CHIP_VERSION, 2, &reg_val));
    if (reg_val != AP1302_CHIP_ID)
    {
         return kStatus_Fail;
    }

    AP1302_CHECK_RET(AP1302_ReadReg(handle, AP1302_SYS_START, 2, &reg_val));
    AP1302_CHECK_RET(AP1302_ReadReg(handle, AP1302_PLL_DIV, 4, &reg_val32));

    /*
     * it's esstential to load firmware, the firmware contains ap1302 configuration and tunning etc.
     * it use i2c bus to load bootdata context into ISP RAM, ISP load this firmware context to run.
     */
    ap1302_fw = (struct ap1302_firmware_t *)(config->fwaddress);

    /*
     * The fw binary contains a header of struct ap1302_firmware, following the header is the bootdata of AP1302.
     * The bootdata pointer can be referenced as &fw[1].
     */
    fw_data = (uint8_t *)&ap1302_fw[1];

    /* Clear crc register. */
    AP1302_CHECK_RET(AP1302_WriteReg(handle, AP1302_SIP_CRC, 2, 0xffff));

    status = AP1302_WriteFW(handle, fw_data, &win_pos, ap1302_fw->pll_init_size);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* Write 2 to bootdata_stage register to apply basic_init_hp settings and enable PLL. */
    AP1302_CHECK_RET(AP1302_WriteReg(handle, AP1302_BOOTDATA_STAGE, 2, 0x0002));

    /* Wait 20ms for PLL to lock. */
    VIDEO_DelayMs(20);

    status = AP1302_WriteFW(handle, fw_data + ap1302_fw->pll_init_size, &win_pos, ap1302_fw->total_size - ap1302_fw->pll_init_size);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* Write 0xFFFF to bootdata_stage register to indicate AP1302 that the whole bootdata content has been loaded. */
    AP1302_CHECK_RET(AP1302_WriteReg(handle, AP1302_BOOTDATA_STAGE, 2, 0xFFFF));
    VIDEO_DelayMs(50);

    AP1302_CHECK_RET(AP1302_ReadReg(handle, AP1302_BOOTDATA_CHECKSUM, 2, &reg_val));
    if (reg_val != ap1302_fw->checksum)
    {
        return kStatus_Fail;
    }

    AP1302_CHECK_RET(AP1302_WriteReg(handle, 0X6124, 2, 0x0001));

    /* disable auto focus */
    AP1302_CHECK_RET(AP1302_WriteReg(handle, 0x5058, 2, 0x0002));
    AP1302_CHECK_RET(AP1302_WriteReg(handle, 0x505C, 2, 0x0064));

    if ((uint32_t)kVIDEO_Resolution1080P == config->resolution)
    {
        AP1302_CHECK_RET(AP1302_WriteReg(handle, AP1302_PREVIEW_WIDTH,  2, 1920));
        AP1302_CHECK_RET(AP1302_WriteReg(handle, AP1302_PREVIEW_HEIGHT, 2, 1080));
    }

    return kStatus_Success;
}

status_t AP1302_Deinit(camera_device_handle_t *handle)
{
    ((ap1302_resource_t *)(handle->resource))->pullPowerDownPin(true);

    return kStatus_Success;
}

status_t AP1302_Start(camera_device_handle_t *handle)
{
    return AP1302_WriteReg(handle, AP1302_SYS_START, 2, 0x0080);
}
