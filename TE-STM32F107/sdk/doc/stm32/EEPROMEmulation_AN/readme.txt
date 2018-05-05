/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : readme.txt
* Author             : MCD Application Team
* Version            : V2.0.0
* Date               : 06/16/2008
* Description        : Description of the AN2594 "EEPROM emulation in STM32F101xx
*                      and STM32F103xx microcontrollers".
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

Example description
===================
This AN describes a software method for emulating EEPROM using the onchip Flash
memory of the STM32F101xx and STM32F103xx devices.


Directory contents
==================
  + include
    - platform_config.h    Evaluation board specific configuration file
    - stm32f10x_conf.h     Library Configuration file
    - stm32f10x_it.h       Interrupt handlers header file
    - eeprom.h             EEPROM emulation Header file

  + source
    - stm32f10x_it.c    Interrupt handlers
    - eeprom.c          EEPROM emulation firmware functions
    - main.c            Main program  


Hardware environment
====================
This example runs on STMicroelectronics STM3210B-EVAL and STM3210E-EVAL evaluation
boards and can be easily tailored to any other hardware.
To select the STMicroelectronics evaluation board used to run the example, uncomment
the corresponding line in platform_config.h file.


How to use it
=============
 + RVMDK
    - Open the EEPROM.Uv2 project
    - In the build toolbar select the project config:
        - STM3210B-EVAL: to configure the project for STM32 Medium-density devices
        - STM3210E-EVAL: to configure the project for STM32 High-density devices
    - Rebuild all files: Project->Rebuild all target files
    - Load project image: Debug->Start/Stop Debug Session
    - Run program: Debug->Run (F5)    

 + EWARM5
    - Open the EEPROM.eww workspace.
    - In the workspace toolbar select the project config:
        - STM3210B-EVAL: to configure the project for STM32 Medium-density devices
        - STM3210E-EVAL: to configure the project for STM32 High-density devices
    - Rebuild all files: Project->Rebuild all
    - Load project image: Project->Debug
    - Run program: Debug->Go(F5)

 + RIDE
    - Open the EEPROM.rprj project.
    - In the configuration toolbar(Project->properties) select the project config:
       - STM3210B-EVAL: to configure the project for STM32 Medium-density devices
       - STM3210E-EVAL: to configure the project for STM32 High-density devices
    - Rebuild all files: Project->build project
    - Load project image: Debug->start(ctrl+D)
    - Run program: Debug->Run(ctrl+F9)  

 + HiTOP
    - Open the HiTOP toolchain, a "using projects in HiTOP" window appears.
    - Select open an existing project.
    - Browse to open the EEPROM.htp:
        - under STM32F10B_EVAL directory: to select the project for STM32 Medium-density devices.
        - under STM32F10E_EVAL directory: to select the project for STM32 High-density devices
    - Rebuild all files: Project->Rebuild all
    - Click on ok in the "download project" window.
    - Run program: Debug->Go(F5).
    Note: 
          - When using High-density devices, it is mandatory to reset the target
            before loading the project into  target.
          - It is recommended to run the reset script (click on TR button in the
            toolbar menu) after loading the project into target.

NOTE:
 - Medium-density devices are STM32F101xx and STM32F103xx microcontrollers where
   the Flash memory density ranges between 32 and 128 Kbytes.
 - High-density devices are STM32F101xx and STM32F103xx microcontrollers where
   the Flash memory density ranges between 256 and 512 Kbytes.               

******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE******
