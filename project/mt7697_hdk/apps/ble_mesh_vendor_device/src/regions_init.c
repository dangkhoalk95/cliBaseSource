/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
 
#include "exception_handler.h"

/******************************************************************************/
/*            Memory Regions Definition                                       */
/******************************************************************************/
#if defined(__GNUC__)

extern unsigned int __FLASH_segment_start__[];
extern unsigned int __FLASH_segment_end__[];
extern unsigned int __SRAM_segment_start__[];
extern unsigned int __SRAM_segment_end__[];
extern unsigned int __ramtext_start__[];
extern unsigned int __ramtext_end__[];
extern unsigned int __tcmbss_start__[];
extern unsigned int __tcmbss_end__[];

const memory_region_type memory_regions[] =
{
    {"flash", __FLASH_segment_start__, __FLASH_segment_end__, 0},
    {"sram",  __SRAM_segment_start__, __SRAM_segment_end__, 1},
    {"ramtext", __ramtext_start__, __ramtext_end__, 1},
    {"tcmbss", __tcmbss_start__, __tcmbss_end__, 1},
    {"scs",  (unsigned int*)SCS_BASE, (unsigned int*)(SCS_BASE + 0x1000), 1},
    {0}
};

#endif /* __GNUC__ */

#if defined (__CC_ARM)

extern unsigned int Image$$TEXT$$Base[];
extern unsigned int Image$$TEXT$$Limit[];
extern unsigned int Image$$DATA$$Base[];
extern unsigned int Image$$DATA$$ZI$$Limit[];
extern unsigned int Image$$TCM$$Base[];
extern unsigned int Image$$TCM$$ZI$$Limit[];
extern unsigned int Image$$STACK$$Base[];
extern unsigned int Image$$STACK$$ZI$$Limit[];

const memory_region_type memory_regions[] =
{
    {"text", Image$$TEXT$$Base, Image$$TEXT$$Limit, 0},
    {"data", Image$$DATA$$Base, Image$$DATA$$ZI$$Limit, 1},
    {"tcm", Image$$TCM$$Base, Image$$TCM$$ZI$$Limit, 1},
    {"stack", Image$$STACK$$Base, Image$$STACK$$ZI$$Limit, 1},
    {"scs",  (unsigned int *)SCS_BASE, (unsigned int *)(SCS_BASE + 0x1000), 1},
    {0}
};

#endif /* __CC_ARM */


#if defined(__ICCARM__)

extern unsigned int RAM_BLOCK$$Base[];
extern unsigned int RAM_BLOCK$$Limit[];
extern unsigned int TCM_BLOCK$$Base[];
extern unsigned int TCM_BLOCK$$Limit[];
extern unsigned int CSTACK$$Base[];
extern unsigned int CSTACK$$Limit[];

const memory_region_type memory_regions[] =
{
    {"ram", RAM_BLOCK$$Base, RAM_BLOCK$$Limit, 1},
    {"tcm", TCM_BLOCK$$Base, TCM_BLOCK$$Limit, 1},
    {"stack", CSTACK$$Base, CSTACK$$Limit, 1},
    {"scs", (unsigned int*)SCS_BASE, (unsigned int*)(SCS_BASE + 0x1000), 1},
    {0}
};


#endif /* __ICCARM__ */


