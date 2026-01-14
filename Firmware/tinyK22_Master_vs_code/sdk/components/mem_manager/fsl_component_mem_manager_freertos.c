/*! *********************************************************************************
 * Copyright 2024-2025 NXP
 *
 * \file
 *
 * This is the source file for the Memory Manager.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "fsl_common.h"
#include "fsl_component_mem_manager.h"
#include "FreeRTOS.h"
#include "portable.h"

/* -------------------------------------------------------------------------- */
/*                                Private types                               */
/* -------------------------------------------------------------------------- */

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address.
 * Note: This structure is based on the heap_4.c implementation, FreeRTOS doesn't
 * expose an API to retrieve the buffer size so we have to re-define the structure
 * here */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /**< The next free block in the list. */
    size_t xBlockSize;                     /**< The size of the free block. */
} BlockLink_t;

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

mem_status_t MEM_Init(void)
{
    return kStatus_MemSuccess;
}

void *MEM_BufferAllocWithId(uint32_t numBytes, uint8_t poolId)
{
    (void)poolId;
    return pvPortMalloc((size_t)numBytes);
}

void *MEM_CallocAlt(size_t len, size_t val)
{
    return pvPortCalloc(len, val);
}

mem_status_t MEM_BufferFree(void *buffer)
{
    vPortFree(buffer);

    return kStatus_MemSuccess;
}

uint32_t MEM_GetFreeHeapSizeByAreaId(uint8_t area_id)
{
    (void)area_id;
    return (uint32_t)xPortGetFreeHeapSize();
}

uint32_t MEM_GetFreeHeapSize(void)
{
    return MEM_GetFreeHeapSizeByAreaId(0U);
}

mem_status_t MEM_RegisterExtendedArea(memAreaCfg_t *area_desc, uint8_t *p_area_id, uint16_t flags)
{
    /* NOT IMPLEMENTED */
    (void)area_desc;
    (void)p_area_id;
    (void)flags;
    return kStatus_MemUnknownError;
}

mem_status_t MEM_UnRegisterExtendedArea(uint8_t area_id)
{
    /* NOT IMPLEMENTED */
    (void)area_id;
    return kStatus_MemUnknownError;
}

mem_status_t MEM_BufferFreeAllWithId(uint8_t poolId)
{
    /* NOT IMPLEMENTED */
    (void)poolId;
    return kStatus_MemFreeError;
}

mem_status_t MEM_BufferCheck(void *buffer, uint32_t size)
{
    /* NOT IMPLEMENTED */
    (void)buffer;
    (void)size;
    return kStatus_MemUnknownError;
}

uint32_t MEM_GetHeapUpperLimitByAreaId(uint8_t area_id)
{
    /* NOT IMPLEMENTED */
    (void)area_id;
    return 0U;
}

uint32_t MEM_GetHeapUpperLimit(void)
{
    return MEM_GetHeapUpperLimitByAreaId(0u);
}

uint32_t MEM_GetFreeHeapSizeLowWaterMarkByAreaId(uint8_t area_id)
{
    /* NOT IMPLEMENTED */
    (void)area_id;
    return 0U;
}

uint32_t MEM_GetFreeHeapSizeLowWaterMark(void)
{
    return MEM_GetFreeHeapSizeLowWaterMarkByAreaId(0u);
}

uint32_t MEM_ResetFreeHeapSizeLowWaterMarkByAreaId(uint8_t area_id)
{
    /* NOT IMPLEMENTED */
    (void)area_id;
    return 0U;
}

uint32_t MEM_ResetFreeHeapSizeLowWaterMark(void)
{
    return MEM_ResetFreeHeapSizeLowWaterMarkByAreaId(0u);
}

uint16_t MEM_BufferGetSize(void *buffer)
{
    uint8_t *pBlock = (uint8_t *)buffer - sizeof(BlockLink_t);

    return (uint16_t)((((BlockLink_t *)pBlock)->xBlockSize - sizeof(BlockLink_t)) & 0xFFFFU);
}

void *MEM_BufferRealloc(void *buffer, uint32_t new_size)
{
    /* NOT IMPLEMENTED */
    (void)buffer;
    (void)new_size;
    return NULL;
}

__attribute__((weak)) void MEM_ReinitRamBank(uint32_t startAddress, uint32_t endAddress)
{
    /* To be implemented by the platform */
    (void)startAddress;
    (void)endAddress;
}
