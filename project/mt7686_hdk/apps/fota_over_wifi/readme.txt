�?* Copyright Statement:
 *
 * (C) 2005-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its
 * licensors. Without the prior written permission of MediaTek and/or its
 * licensors, any reproduction, modification, use or disclosure of MediaTek
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute
 * (as applicable) MediaTek Software if you have agreed to and been bound by
 * the applicable license agreement with MediaTek ("License Agreement") and
 * been granted explicit permission to do so within the License Agreement
 * ("Permitted User").  If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately.
 */

/**
 * @addtogroup mt7686_hdk mt7686_hdk
 * @{
 * @addtogroup mt7686_hdk_apps apps
 * @{
 * @addtogroup mt7686_hdk_apps_fota_over_wifi fota_over_wifi
 * @{

@par Overview
  - Example project description
    - This example demonstrates the basic workflow of full binary FOTA
      update, including how to use Wi-Fi to download FOTA package based on 
      the LinkIt 7686 HDK through the following:
      - How to use the Wi-Fi profile settings stored in the NVDM to initialize
        the Wi-Fi.
      - How to use CLI command to operate FOTA process.
  - Application features
    - Act as a Wi-Fi station to connect to a Wi-Fi network.
    - Use CLI command to trigger FOTA download and update process.

@par Hardware and software environment
  - Supported platform
    - MediaTek LinkIt 7686 HDK.
  - HDK switches and pin configuration
    - J36 provides the pins for GPIOs, PWMs, SPI master chip select 0, SPI
      master, UART1 RX/TX.
    - J35 provides the pins for GPIOs, PWMs, UART2 RX/TX, UART1 RTS/CTS, SPI
      master chip select 1, IR TX and IR RX.
    - J34 provides the pins for GPIOs, PWMs, UART2 RTS/CTS, I2S, SPI slave, and
      I2C0.
    - J33 provides the pins for GPIOs, PWMs, I2C0, ADC0~3.
    - J32 provides the pins for GND, 5V, 3.3V and reset pin.
    - J25 set the HDK to either Flash Normal mode or Flash Recovery mode.
      To update the firmware on the LinkIt 7686 HDK:
       - Set the jumper J25 to FLASH Recovery mode, the jumpers J23, J26, J27
         and J30 should be on.
       - In this mode, if the power is on, the board will load ROM code and start
         the ATE Daemon or Firmware Upgrade Daemon 
         according to the MT76x7 Flash Tool’s behavior on the PC.
      To run the project on the LinkIt 7686 HDK:
       - Set the jumper J25 off to switch to FLASH Normal mode, the jumpers J23,
         J26, J27 and J30 should be on.
       - In this mode, if the power is on, the board will load firmware from the
         flash and reboot.
    - There are three buttons on the board:
      - RST - reset.
      - EINT - external interrupt trigger.
      - RTC_INT - RTC interrupt trigger.
  - Environment configuration
    - A serial tool is required, such as hyper terminal or TeraTerm for UART
      logging.
    - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1, parity:
      none and flow control: off.

@par Directory contents
  - Source and header files
    - \b src/cli_cmds.c            CLI commands of this project.
    - \b src/cli_def.c             CLI initialize sequence code.
    - \b src/ept_eint_var.c        EINT configuration file generated by
                                        Easy Pinmux Tool(EPT). Please do
                                        not edit the file.
    - \b src/ept_gpio_var.c        GPIO configuration file generated by
                                        Easy Pinmux Tool(EPT). Please do not
                                        edit the file.
    - \b src/main.c                Entry point of the application program.
    - \b src/wifi_nvdm_config.c    Default user configuration file.
    - \b src/sys_init.c            Aggregated initialization routines.
    - \b src/system_mt7686.c       MT7686 system clock configuration file.
    - \b src/wifi_lwip.c           LwIP configuration.
    - \b inc/cli_cmds.h            Declares the reference point of CLI
                                        commands of cli_cmds.c. To be used
                                        by cli_def.c
    - \b inc/FreeRTOSConfig.h      MT7686 FreeRTOS configuration file.
    - \b inc/ept_eint_drv.h        The EINT configuration file generated
                                        by Easy Pinmux Tool(EPT). Please do
                                        not edit the file.
    - \b inc/ept_gpio_drv.h        The GPIO configuration file generated
                                        by Easy Pinmux Tool(EPT). Please do
                                        not edit the file.
    - \b inc/flash_map.h           MT7686 memory layout symbol file.
    - \b inc/hal_feature_config.h  MT7686 HAL API feature configuration
                                        file.
    - \b inc/lwipopts.h            LwIP configuration.
    - \b inc/sys_init.h            Prototype declaration for \b src/sys_init.c.
    - \b inc/task_def.h            The configuration of running tasks of
                                        the project.
    - \b inc/wifi_lwip.h           Prototype declaration for \b src/wifi_lwip.c.
    - \b inc/wifi_nvdm_config.h    Prototype declaration for default user
                                        configuration file.

  - Project configuration files using GCC.
    - \b GCC/Makefile              GNU Makefile for this project.
    - \b GCC/feature.mk            Generic feature options configuration
                                        file.
    - \b GCC/mt7686_flash.ld       Linker script.
    - \b GCC/startup_mt7686.s      MT7686 startup file.
    - \b GCC/syscalls.c            MT7686 syscalls implementation.

  - Project configuration files using IAR.
    - \b EWARM/flash.icf           Linker configuration file.
    - \b EWARM/fota_over_wifi.ewd         Debugger settings file.
    - \b EWARM/fota_over_wifi.ewp         Project file
    - \b EWARM/fota_over_wifi.eww         Workspace file
    - \b EWARM/startup_mt7686.s    MT7686 startup file.
    - \b EWARM/extract_rtos_bin.py        python script to extract binary file to generate FOTA package.
@par Run the examples
  - Connect the board to a PC with a USB cable.
  - Build the example project and download the binary file to the LinkIt
    7686 HDK.
  - Reboot the HDK, the console will show "FreeRTOS Running" message to
    indicate the HDK is booting up.
  - Use '?' and enter to query the available command line options.
    Note that the command line options are still under development and subject
    to change without notice.
  - Use below command to connect to Wi-Fi access point.
    wifi config set ssid 0 ap_name
    wifi config set psk 0 password
    wifi config set reload
  - After ip address retrieved and shown in log, the connection has been established.
  - Use below command to trigger FOTA download:
    fota httpdl url
    e.g. fota httpdl http://192.168.0.198/image.bin
  - Then the downloading process will be shown in log.
  - After download finished, please input this command to trigger update.
    fota go
    reboot
  - Then device will reboot and enter bootloader phase, bootloader will enter FOTA update flow.
  - Notice: if use IAR compiler, the built out binary is a combination of bootloader, RTOS and PwrTX.
    So user should use extract_rtos_bin.py to get the actual rtos binary file from the iar image, and
    then use rtos binary to generate FOTA package by FOTA packaging tool.

*/
/**
 * @}
 * @}
 * @}
 */
