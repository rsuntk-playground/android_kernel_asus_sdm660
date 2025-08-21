/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2010 - 2018 Novatek, Inc.
 *
 * $Revision: 47247 $
 * $Date: 2019-07-10 10:41:36 +0800 (Wed, 10 Jul 2019) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#ifndef 	_LINUX_NVT_TOUCH_H
#define		_LINUX_NVT_TOUCH_H

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>
#include <uapi/linux/sched/types.h>
#include "nt36xxx_mem_map.h"

#define NVT_DEBUG 0

//---GPIO number---
#define NVTTOUCH_RST_PIN 980
#define NVTTOUCH_INT_PIN 943

/* Begin ASUS SD660/SD636 COMMON */
#define NVT_POWER_SOURCE_CUST_EN  1
#if NVT_POWER_SOURCE_CUST_EN
#define LCM_LAB_MIN_UV                      6000000
#define LCM_LAB_MAX_UV                      6000000
#define LCM_IBB_MIN_UV                      6000000
#define LCM_IBB_MAX_UV                      6000000
#endif

/* Begin ASUS X01BD CONFIG */
#ifdef CONFIG_MACH_ASUS_X01BD
#define TOUCH_DEFAULT_MAX_HEIGHT 2280
#define NVT_TOUCH_SUPPORT_HW_RST 0
#define BOOT_UPDATE_FIRMWARE_NAME "novatek_ts_fw_v8D.bin"
#define NVT_TOUCH_ESD_PROTECT 0
/* Begin ASUS X00TD CONFIG */
#elif defined(CONFIG_MACH_ASUS_X00TD)
#define TOUCH_DEFAULT_MAX_HEIGHT 2160
#define NVT_TOUCH_SUPPORT_HW_RST 1
#define DJ_BOOT_UPDATE_FIRMWARE_NAME "novatek_ts_fw_dj.bin"
#define TXD_BOOT_UPDATE_FIRMWARE_NAME "novatek_ts_fw_txd.bin"
#define NVT_TOUCH_ESD_PROTECT 1
#else
#error "Do not compile without CONFIG_MACH_ASUS_{X00TD, X01BD} !!"
#endif

//---INT trigger mode---
//#define IRQ_TYPE_EDGE_RISING 1
//#define IRQ_TYPE_EDGE_FALLING 2
#define INT_TRIGGER_TYPE IRQ_TYPE_EDGE_RISING

//---I2C driver info.---
#define NVT_I2C_NAME "NVT-ts"
#define I2C_BLDR_Address 0x01
#define I2C_FW_Address 0x01
#define I2C_HW_Address 0x62

//---Logging---
#if NVT_DEBUG
#define NVT_LOG(fmt, args...)    pr_err("[%s] %s %d: " fmt, NVT_I2C_NAME, __func__, __LINE__, ##args)
#else
#define NVT_LOG(fmt, args...)    pr_info("[%s] %s %d: " fmt, NVT_I2C_NAME, __func__, __LINE__, ##args)
#endif
#define NVT_ERR(fmt, args...)    pr_err("[%s] %s %d: " fmt, NVT_I2C_NAME, __func__, __LINE__, ##args)

//---Input device info.---
#define NVT_TS_NAME "NVTCapacitiveTouchScreen"

//---Touch info.---
#define TOUCH_MAX_FINGER_NUM 10
#define TOUCH_FORCE_NUM 1000
#define TOUCH_DEFAULT_MAX_WIDTH 1080

/* Enable only when module have tp reset pin and connected to host */

//---Customerized func.---
#define MT_PROTOCOL_B 1
#define WAKEUP_GESTURE 1
#if WAKEUP_GESTURE
extern const uint16_t gesture_key_array[];
#endif

#define BOOT_UPDATE_FIRMWARE 1

//---ESD Protect.---
#define NVT_TOUCH_ESD_CHECK_PERIOD 1500	/* ms */

struct nvt_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct delayed_work nvt_fwu_work;
	uint16_t addr;
	int8_t phys[32];
#ifdef CONFIG_FB
	struct notifier_block fb_notif;
#endif
	uint8_t fw_ver;
	uint8_t x_num;
	uint8_t y_num;
	uint16_t abs_x_max;
	uint16_t abs_y_max;
	uint8_t max_touch_num;
	uint32_t int_trigger_type;
	int32_t irq_gpio;
	uint32_t irq_flags;
	int32_t reset_gpio;
	uint32_t reset_flags;
	struct mutex lock;
	const struct nvt_ts_mem_map *mmap;
	uint8_t carrier_system;
	uint16_t nvt_pid;
	uint8_t xbuf[1025];
	struct mutex xbuf_lock;
	bool irq_enabled;
#if NVT_POWER_SOURCE_CUST_EN
	struct regulator *lcm_lab;
	struct regulator *lcm_ibb;
	atomic_t lcm_lab_power;
	atomic_t lcm_ibb_power;
#endif
};

typedef enum {
	RESET_STATE_INIT = 0xA0,// IC reset
	RESET_STATE_REK,        // ReK baseline
	RESET_STATE_REK_FINISH, // baseline is ready
	RESET_STATE_NORMAL_RUN, // normal run
	RESET_STATE_MAX  = 0xAF
} RST_COMPLETE_STATE;

typedef enum {
	EVENT_MAP_HOST_CMD                      = 0x50,
	EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE   = 0x51,
	EVENT_MAP_RESET_COMPLETE                = 0x60,
	EVENT_MAP_FWINFO                        = 0x78,
	EVENT_MAP_PROJECTID                     = 0x9A,
} I2C_EVENT_MAP;

//---extern structures---
extern struct nvt_ts_data *ts;

//---extern functions---
extern int32_t CTP_I2C_READ(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len);
extern int32_t CTP_I2C_WRITE(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len);
extern void nvt_bootloader_reset(void);
extern void nvt_sw_reset_idle(void);
extern int32_t nvt_check_fw_reset_state(RST_COMPLETE_STATE check_reset_state);
extern int32_t nvt_get_fw_info(void);
extern int32_t nvt_clear_fw_status(void);
extern int32_t nvt_check_fw_status(void);
extern int32_t nvt_set_page(uint16_t i2c_addr, uint32_t addr);
#if NVT_TOUCH_ESD_PROTECT
extern void nvt_esd_check_enable(uint8_t enable);
#endif /* #if NVT_TOUCH_ESD_PROTECT */
extern void nvt_stop_crc_reboot(void);

#endif /* _LINUX_NVT_TOUCH_H */
