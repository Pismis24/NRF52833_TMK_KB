# Nordic nRF52833的TMK键盘固件</br>TMK firmware for Nordic nRF52833

- - -  

## 概述

在Nordic nRF52833上运行的TMK键盘固件  
使用Segger Embedded Studio作为IDE
Nordic SDK版本：nRF5_SDK_17.0.2
固件是为我自己设计制作的键盘编写，部分代码为适应我的硬件设计而写（主要是灯和背光），硬件设计开源地址：

* 主控板：https://oshwhub.com/Pismis24/nrf52833-kb-control-board
* 按键板：https://oshwhub.com/Pismis24/nrf52keyboard-mainboard

- - -

## 目录结构

* nrf_sdk/ 放置nordic SDK文件文件夹  
* project/ ses项目文件目录  
* src/ 项目源码  
  * ble/ 蓝牙相关  
  * config/ 键盘设置、按键矩阵  
  * hardware_def/ 引脚定义与芯片外设定义  
  * keyboard/ 键盘主要功能  
  * protocol/ 按键发送协议与管理
  * tmk_glue/ 适配tmk相关  
  * usb/ USB相关
  * main.c 主程序
* tmk_core/ tmk_core项目代码  

- - -

## 编译方式

1. Clone或者将项目Download至电脑  
2. 下载nordic官方sdk：nRF5_SDK_17.0.2  
3. 解压后将SDK中的**components**、**external**、**integration**、**modules**四个文件夹放入项目目录下的**nrf_sdk**文件夹中  
4. 下载并安装**Segger Embedded Studio**（以下简称SES），使用SES打开项目目录下**project**文件夹中的**nrf52833_tmk_kb.emProject**项目工程文件
5. 摁F7进行编译，再通过Jlink将固件写入芯片（没试过用别的）  

- - -

## 项目当前状态

* 键盘基本功能已完成，现在我就在用这个键盘写readme  
* 蓝牙单模  
* 电量检测并通过蓝牙BAS服务发送至电脑  
* 支持编码器（EC11）  
* 支持发送鼠标按键、多媒体按键  
* 模式切换完成（尚未完成USB HID相关工作）
* 动态发射功率
* 键盘休眠唤醒（当前设定是10分钟无操作）

- - -

## 等待完成的功能

* USB有线连接  
* 功耗优化（有背光灯估计优化不到哪去）

- - -

## 已知问题

* ~~删除绑定后会报错卡死，看门狗将会重启系统，重新开始开机初始化到广播的流程（出错但还是能达到目的，曲线救国了属于是）~~  
  已解决，通过在**PM_EVT_PEERS_DELETE_SUCCEEDED**事件发生后直接重启系统解决

- - -
- - -

## Overview

A TMK Firmware for Nordic nRF52833
Using Segger Embedded Studio as IDE  
Nordic SDK verison：nRF5_SDK_17.0.2  
This firmware is written for my custom keyboard, some code was written specially for hardware (mainly in led and backlight part), the hardware design is open source on :  

* control board: https://oshwhub.com/Pismis24/nrf52833-kb-control-board
* main board: https://oshwhub.com/Pismis24/nrf52keyboard-mainboard

- - -

## Directory Structure

* nrf_sdk/ Folder for nRF SDK Files
* project/ **SES Project Files**  
* src/ **Firmware source code**  
  * ble/ **bluetooth related**  
  * config/ **Keyboard config and layout**  
  * hardware_def/ **Defination of GPIO and other peripherals**  
  * keyboard/ **Main functions of keyboard**  
  * protocol/ **Keyboard Protocol**
  * tmk_glue/ **TMK Core adapter**  
  * usb/ **USB Related**
  * main.c **Main Program**
* tmk_core/ **tmk_core source codes**  

- - -

## Compile Method

1. Clone or Download this repository to your computer
2. Download Nordic **nRF5 SDK version 17.0.2**
3. Unzip the file and move the four folders **components**、**external**、**integration**、**modules** to the **nrf_sdk** of the project
4. Download and install **Segger Embedded Studio**(so call SES), and open **nrf52833_tmk_kb.emProject** in the **project** folder
5. Press **F7** or click **Compile** to compile the firmware, then you can flash it to your nrf52833 chip via Jlink

- - -

## Current Status

* Basic functions of keyboard completed, I am now using this keyboard to write the readme
* Bluetooth single mode
* Battery level measure and upload via BLE_Battery Service
* Support Encoder (EC11)
* Support sending mouse keys and consumer keys
* Can manually switch connection mode (with usb hid related work to be done)
* Dynamic TX strength
* Auto Sleep after 10mins of no input

- - -

## Works to be done

* USB Connection
* Power optimization
  
- - -

## Known Issues

* ~~Error occur when manually delete bond, will cause crash and system reset by watchdog(Well, the bonding process will begin after the reset, still work but rather annoying)~~  
  Fixed by trigger system reset after **PM_EVT_PEERS_DELETE_SUCCEEDED** event happened.
