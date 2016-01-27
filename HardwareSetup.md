This page describes the hardware setup needed for enabling Android Open Accessory Development Kit (ADK) on STM32-based kits. The wirings refer in particular to iNEMO board but can be easily adapted to whatever STM32 based board.

## Introduction ##
Hardware needed
  * iNEMOv2 board (or equivalent STM32-based board)
  * SparkFun USB Host shield (or equivalent Maxim 3421e breakout board)
  * UART interface (e.g.: FTDI TTL-232Rg-VREG3V3-WE)


## USB Shield connections ##
Following table shows the connections between SparkFun USB shield and iNEMO 2x5 "Extended connector" (J8)
| **J8 Pin, signal** | USB Shield Pin, signal |
|:-------------------|:-----------------------|
| 1, GND             | N.C.                   |
| 2, 3V3             | D7, Reset              |
| 3, PB0             | N.C.                   |
| 4, CS              | N.C.                   |
| 5, SCK             | D13, SCK               |
| 6, MOSI            | D11, MOSI              |
| 7, MISO            | D12, MISO              |
| 8, PC7             | D9, INT                |
| 9, PA8             | N.C.                   |
| 10, PC3            | D10, SS                |

In addition, the shield must be powered by means of an external power supply (e.g.: 9V) on VIn pin.


## UART connections ##
Just connect RX pin of your UART TTL interface to Pin 2 of iNEMO connector J4.