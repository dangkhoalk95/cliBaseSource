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
 
/**
 * This file contains the root of all CLI commands. They are divided into
 * three modes: NORMAL, TEST, EXAMPLE.
 *
 * Normal and test are enabled by default. TEST mode commands can be turned
 * off by setting MTK_CLI_TEST_MODE_ENABLE to 'n' in feature.mk. Example
 * mode CLI commands can be included by setting MTK_CLI_EXAMPLE_MODE_ENABLE
 * to 'y' in feature.mk.
 */

/****************************************************************************
 *
 * Header files.
 *
 ****************************************************************************/

#if defined(MTK_MINICLI_ENABLE)


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>

#include "cli.h"
#include "cli_def.h"
#include "task_def.h"

#include <toi.h>
#include <type_def.h>
#include <os.h>
#include <os_util.h>

#include "io_def.h"

#include "cli_cmds.h"


GETCHAR_PROTOTYPE;
PUTCHAR_PROTOTYPE;


static cli_t _cli_cb = {
    .state  = 1,
    .echo   = 0,
    .get    = __io_getchar,
    .put    = __io_putchar,
};


/****************************************************************************
 * Private Functions
 ****************************************************************************/


#if !defined(CLI_DISABLE_LINE_EDIT) && !defined(CLI_DISABLE_HISTORY)
#ifdef MTK_SINGLE_SKU_SUPPORT
//Signle_SKU item data len is large than 798 bytes.
#define HISTORY_LINE_MAX    (800)
#else
#define HISTORY_LINE_MAX    (128)
#endif
#define HISTORY_LINES       (5)
#endif /* !CLI_DISABLE_LINE_EDIT && !CLI_DISABLE_HISTORY */


#if !defined(CLI_DISABLE_LINE_EDIT) && !defined(CLI_DISABLE_HISTORY)

static char s_history_lines[ HISTORY_LINES ][ HISTORY_LINE_MAX ];
static char *s_history_ptrs[ HISTORY_LINES ];

static char s_history_input[ HISTORY_LINE_MAX ];
static char s_history_parse_token[ HISTORY_LINE_MAX ];

#endif /* !CLI_DISABLE_LINE_EDIT && !CLI_DISABLE_HISTORY */


/****************************************************************************
 * Public Functions
 ****************************************************************************/


void cli_def_create(void)
{
#if !defined(CLI_DISABLE_LINE_EDIT) && !defined(CLI_DISABLE_HISTORY)
    cli_history_t *hist = &_cli_cb.history;
    int i;

    for (i = 0; i < HISTORY_LINES; i++) {
        s_history_ptrs[i] = s_history_lines[i];
    }
    hist->history           = &s_history_ptrs[0];

    hist->input             = s_history_input;
    hist->parse_token       = s_history_parse_token;
    hist->history_max       = HISTORY_LINES;
    hist->line_max          = HISTORY_LINE_MAX;
    hist->index             = 0;
    hist->position          = 0;
    hist->full              = 0;
#endif /* !CLI_DISABLE_LINE_EDIT && !CLI_DISABLE_HISTORY */

    cli_cmds_init(&_cli_cb);
    cli_init(&_cli_cb);
}


void cli_def_task(void *param)
{
    while (1) {
        cli_task();
    }
}


int cli_task_create(void)
{
    if (xTaskCreate(cli_def_task,
                    MINICLI_TASK_NAME,
                    MINICLI_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                    NULL,
                    MINICLI_TASK_PRIO,
                    NULL) != pdPASS)
    {
        LOG_E(common, "xTaskCreate fail");
        return -1;
    }

    return 0;
}


#endif /* #if defined(MTK_MINICLI_ENABLE) */

