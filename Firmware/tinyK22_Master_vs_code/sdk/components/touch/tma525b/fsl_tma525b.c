/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_tma525b.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TMA525B_TOUCH_POINT_TEST (0)

typedef struct _tma525b_touch_point
{
    uint8_t Touch_Type; /* 0 for standard standard finger or glove, 1 for proximity sensing of conductive or different
                           dielectric properties objects. */
    uint8_t Event_ID;   /* Bit 0-4 for touch ID of the touch event, bit 5-6 for touch event. */
    uint8_t XL;
    uint8_t XH;
    uint8_t YL;
    uint8_t YH;
    uint8_t Pressure;       /* Touch intensity. */
    uint8_t Major_Axis_Len; /* The length of the major axis(in mm) of the ellipse of contact between the finger and the
                               panel. */
    uint8_t Minor_Axis_Len; /* The length of the minor axis(in mm) of the ellipse of contact between the finger and the
                               panel. */
    uint8_t Orientation;    /* The angle between the panel vertical axis and the major axis of the contact ellipse. */
} tma525b_touch_point_t;

typedef struct _tma525b_touch_data
{
    uint8_t LENGTH_L;          /* Packet length. */
    uint8_t LENGTH_H;          /* Packet length. */
    uint8_t REPORT_ID;
    uint8_t TIMESTAMP_100US_L; /* Timestamp in the unit of 100us. */
    uint8_t TIMESTAMP_100US_H; /* Timestamp in the unit of 100us. */
    uint8_t NUM_TOUCH;         /* How many touch point detected. */
    uint8_t REPORT_NOISE;      /* High 2 bits are report counter, low 3 bits are noise effects. */
    tma525b_touch_point_t TOUCH[TMA525B_MAX_TOUCHES];
} tma525b_touch_data_t;

#define TMA525B_TOUCH_DATA_LEN_BYTES (0x2)

#define TMA525B_TOUCH_DATA_GET_ID(T)     ((T).REPORT_ID)
#define TMA525B_TOUCH_DATA_GET_LENGTH(T) (uint16_t)((((uint16_t)(T).LENGTH_H) << 8) | (uint16_t)(T).LENGTH_L)
#define TMA525B_TOUCH_DATA_GET_COUNT(T)  ((T).NUM_TOUCH)

#define TMA525B_TOUCH_POINT_GET_ID(T)    ((T).Event_ID & 0x1F)
#define TMA525B_TOUCH_POINT_GET_EVENT(T) ((touch_event_t)(uint8_t)(((T).Event_ID & 0x60) >> 5U))
#define TMA525B_TOUCH_POINT_GET_X(T)     (int)(uint16_t)((((uint16_t)(T).XH & 0x0fU) << 8) | (uint16_t)(T).XL)
#define TMA525B_TOUCH_POINT_GET_Y(T)     (int)(uint16_t)((((uint16_t)(T).YH & 0x0fU) << 8) | (uint16_t)(T).YL)

#define TMA525B_SUBADDR_SIZE (1U)

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t TMA525B_Init(tma525b_handle_t *handle, const tma525b_config_t *config)
{
    status_t status;
    uint8_t touch_data0, touch_data1;
    bool app_mode = false;

    if ((NULL == handle) || (NULL == config))
    {
        return kStatus_InvalidArgument;
    }

    (void)memset(handle, 0, sizeof(*handle));

    handle->I2C_SendFunc     = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc  = config->I2C_ReceiveFunc;
    handle->pullResetPinFunc = config->pullResetPinFunc;
    handle->pullPowerPinFunc = config->pullPowerPinFunc;

    /* Power on sequence */
    if (NULL != handle->pullPowerPinFunc)
    {
        handle->pullPowerPinFunc(true);
    }

    /* Reset the controller. */
    if (NULL != handle->pullResetPinFunc)
    {
        handle->pullResetPinFunc(false);
        config->timeDelayMsFunc(5U);
        handle->pullResetPinFunc(true);
    }

    config->timeDelayMsFunc(300U);

    /* Enter application mode. */
    while (!app_mode)
    {
        status = handle->I2C_ReceiveFunc(TMA525B_I2C_ADDRESS, 1U, TMA525B_SUBADDR_SIZE, handle->touchBuf, 2U);
        if (status == kStatus_Success)
        {
            touch_data0 = handle->touchBuf[0];
            touch_data1 = handle->touchBuf[1];

            if (((touch_data0 == 0x02U) && (touch_data1 == 0x00U)) || (touch_data1 == 0xFFU))
            {
                app_mode = true;
            }
        }
        config->timeDelayMsFunc(300U);
    }

    return kStatus_Success;
}

status_t TMA525B_Deinit(tma525b_handle_t *handle)
{
    status_t status;

    if (NULL == handle)
    {
        status = kStatus_InvalidArgument;
    }
    else
    {
        if (NULL != handle->pullResetPinFunc)
        {
            handle->pullResetPinFunc(false);
        }

        if (NULL != handle->pullPowerPinFunc)
        {
            handle->pullPowerPinFunc(true);
        }

        status = kStatus_Success;
    }

    return status;
}

status_t TMA525B_GetSingleTouch(tma525b_handle_t *handle, touch_event_t *touch_event, int *touch_x, int *touch_y)
{
    status_t status;
    touch_event_t touch_event_local;

    /* Read first two bytes(TMA525B_TOUCH_DATA_LEN_BYTES) to check the current touch data length. */
    status = handle->I2C_ReceiveFunc(TMA525B_I2C_ADDRESS, 1U, TMA525B_SUBADDR_SIZE, handle->touchBuf,
                                     TMA525B_TOUCH_DATA_LEN_BYTES);
    if (status == kStatus_Success)
    {
        tma525b_touch_data_t *touch_data = (tma525b_touch_data_t *)(void *)(handle->touchBuf);
        uint16_t touch_data_len          = TMA525B_TOUCH_DATA_GET_LENGTH(*touch_data);

        /* Validate touch data. */
        if ((touch_data_len <= 0x02) || (touch_data_len > TMA525B_TOUCH_DATA_LEN))
        {
            return kStatus_Fail;
        }

        /* Read the touch data again according to length. */
        status =
            handle->I2C_ReceiveFunc(TMA525B_I2C_ADDRESS, 1U, TMA525B_SUBADDR_SIZE, handle->touchBuf, touch_data_len);
        if (status == kStatus_Success)
        {
            uint8_t touch_report_id = TMA525B_TOUCH_DATA_GET_ID(*touch_data);

            /* Only when report ID is 0x01 the touch data is valid. */
            if (touch_report_id != 0x01U)
            {
                return kStatus_Fail;
            }

            touch_event_local = TMA525B_TOUCH_POINT_GET_EVENT(touch_data->TOUCH[0]);

            /* Update coordinates only if there is touch detected. */
            if (touch_event_local != kTouch_Reserved)
            {
                if (NULL != touch_x)
                {
                    *touch_x = TMA525B_TOUCH_POINT_GET_X(touch_data->TOUCH[0]);
                }
                if (NULL != touch_y)
                {
                    *touch_y = TMA525B_TOUCH_POINT_GET_Y(touch_data->TOUCH[0]);
                }
            }

            if (touch_event != NULL)
            {
                *touch_event = touch_event_local;
            }
        }
    }

    return status;
}

status_t TMA525B_GetMultiTouch(tma525b_handle_t *handle,
                               int *touch_count,
                               touch_point_t touch_array[TMA525B_MAX_TOUCHES])
{
    status_t status;
    uint32_t i;

    /* Read first two bytes(TMA525B_TOUCH_DATA_LEN_BYTES) to check the current touch data length. */
    status = handle->I2C_ReceiveFunc(TMA525B_I2C_ADDRESS, 1U, TMA525B_SUBADDR_SIZE, handle->touchBuf,
                                     TMA525B_TOUCH_DATA_LEN_BYTES);
    if (status == kStatus_Success)
    {
        tma525b_touch_data_t *touch_data = (tma525b_touch_data_t *)(void *)(handle->touchBuf);
        uint16_t touch_data_len          = TMA525B_TOUCH_DATA_GET_LENGTH(*touch_data);

        /* Validate touch data. */
        if ((touch_data_len <= 0x02) || (touch_data_len > TMA525B_TOUCH_DATA_LEN))
        {
            return kStatus_Fail;
        }

        /* Read the touch data again according to length. */
        status =
            handle->I2C_ReceiveFunc(TMA525B_I2C_ADDRESS, 1U, TMA525B_SUBADDR_SIZE, handle->touchBuf, touch_data_len);
        if (status == kStatus_Success)
        {
            uint8_t touch_report_id = TMA525B_TOUCH_DATA_GET_ID(*touch_data);

            /* Only when report ID is 0x01 the touch data is valid. */
            if (touch_report_id != 0x01U)
            {
                return kStatus_Fail;
            }

            /* Decode number of touches */
            if (NULL != touch_count)
            {
                *touch_count = (int)TMA525B_TOUCH_DATA_GET_COUNT(*touch_data);
            }

            /* Decode valid touch points */
            for (i = 0; i < TMA525B_TOUCH_DATA_GET_COUNT(*touch_data); i++)
            {
                touch_array[i].TOUCH_ID    = TMA525B_TOUCH_POINT_GET_ID(touch_data->TOUCH[i]);
                touch_array[i].TOUCH_EVENT = TMA525B_TOUCH_POINT_GET_EVENT(touch_data->TOUCH[i]);
                touch_array[i].TOUCH_X     = TMA525B_TOUCH_POINT_GET_X(touch_data->TOUCH[i]);
                touch_array[i].TOUCH_Y     = TMA525B_TOUCH_POINT_GET_Y(touch_data->TOUCH[i]);
            }
            /* Clear vacant elements of touch_array */
            for (; i < TMA525B_MAX_TOUCHES; i++)
            {
                touch_array[i].TOUCH_ID    = 0;
                touch_array[i].TOUCH_EVENT = kTouch_Reserved;
                touch_array[i].TOUCH_X     = 0;
                touch_array[i].TOUCH_Y     = 0;
            }
        }
    }
    return status;
}
