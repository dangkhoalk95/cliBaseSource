/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

// For Register AT command handler
// System head file
#include "hal_feature_config.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "at_command.h"
#include "hal_pmu.h"
/*
 * sample code
*/

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd);
static int htoi(char s[]);
/*
AT+EPMUREG=<op>	            |   "OK"
AT+EPMUREG=?                |   "+EPMUREG=(0,1)","OK"


*/
// AT command handler
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd)
{			
    atci_response_t response = {{0}};
    uint16_t read_value = 0;
    int input_addr = 0;
    int input_value = 0;
    char *end_pos = NULL;
    
    printf("atci_cmd_hdlr_pmu \r\n");


    response.response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif


    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+EPMUREG=?
            strcpy((char *)response.response_buf, "+EPMUREG=(0,1)\r\nOK\r\n");
			
            response.response_len = strlen((const char *)response.response_buf);
            atci_send_response(&response);
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EPMUREG=<op>  the handler need to parse the parameters

            if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=0,", strlen("AT+EPMUREG=0,")) == 0) {
                /*command: AT+EPMUREG=0,1234*/
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos ++;

                input_addr = htoi(end_pos);
                end_pos = NULL;
                printf("read register address:0x%x\r\n", input_addr);

                /* read data of input register address */
                read_value = pmu_get_register_value(input_addr, 0xFF, 0);

                snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE , "+EPMUREG:0x%x,0x%x\r\n", input_addr, read_value);
                /* ATCI will help append "OK" at the end of response buffer  */
                response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response.response_len = strlen((const char *)response.response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=1,", strlen("AT+EPMUREG=1,")) == 0) {

                /*command: AT+EPMUREG=1,1234,456*/

                char *mid_pos = NULL;
                char str[20] = {0};
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos ++;
                end_pos = strchr(mid_pos, ',');
                memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                input_addr = htoi(mid_pos);
                end_pos ++;
                input_value = htoi(end_pos);

                mid_pos = NULL;
                end_pos = NULL;
                printf("register address:0x%x, set register value:0x%x\r\n", input_addr, input_value);

                /* write input data to input register address*/
                pmu_set_register_value(input_addr, 0xFF, 0, input_value);

                /* ATCI will help append "OK" at the end of response buffer	*/
                response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response.response_len = strlen((const char *)response.response_buf);

            } else {
                /*invalide parameter, return "ERROR"*/
                strcpy((char *)response.response_buf, "ERROR\r\n");
                response.response_len = strlen((const char *)response.response_buf);
            };

            atci_send_response(&response);

            break;

        default :
            /* others are invalid command format */
            strcpy((char *)response.response_buf, "ERROR\r\n");
            response.response_len = strlen((const char *)response.response_buf);
            atci_send_response(&response);
            break;
    }
    return ATCI_STATUS_OK;
}


// change the data with hex type in string to data with dec type
int htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((int)s[i]) > '9') {
            n = 16 * n + (10 + tolower((int)s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((int)s[i]) - '0');
        }
    }
    return n;
}


#endif
