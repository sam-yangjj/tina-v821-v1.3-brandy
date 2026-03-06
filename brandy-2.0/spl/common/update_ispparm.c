/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the people's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <common.h>
#include <arch/gpio.h>
#include <spare_head.h>
#include <arch/gpio_new.h>
#include <libfdt.h>

#define ISP_DEBUG 0

// One camera accounts for 4k data
#pragma pack(1)
typedef struct tagsensor_isp_config_s {
	int sign; //id0: 0xAA66AA66, id1: 0xBB66BB66
	int crc; //checksum, crc32 check, if crc = 0, do check
	int ver; //version, 0x01
	int light_enable; //adc en, 1:use LIGHT_SENSOR_ATTR_S
	//ADC mode, 0:the brighter the u16LightValue more, 1:the brighter the u16LightValue smaller
	int adc_mode;
	//adc threshold: 1,adc_mode=0:smaller than it enter night mode, greater than it or equal enter day mode
	unsigned short light_def;
	//adc_mode=1:greater than it enter night mode, smaller than it or equal enter day mode;
	unsigned char ircut_state; //1:hold, 0:use ir_mode
	unsigned char res[4072];
	 /* noteFastboot specific for switching filesystems and systems */
	unsigned char switch_system_flag;
} SENSOR_ISP_CONFIGS_S;
#pragma pack()

#ifdef CFG_BOOT0_WRITE_IRSATTE_TO_ISP

#if !defined(CFG_SUNXI_PHY_KEY) && !defined(CFG_GPADC_KEY)
#error This function depends on gpadc
#endif

#define DEFAULT_COMPARISON_VALUE (1280)

#define DAY_STATE   0
#define NIGHT_STATE 1

#define CAMERA1_SIGN (0xAA66AA66)
#define CAMERA2_SIGN (0xBB66BB66)

#define CAMERA1_ADC_CHANNEL (0)
#define CAMERA2_ADC_CHANNEL (0)

enum Day_Time {
	enum_none = 0,
	enum_day,
	enum_night,
};

enum Led_Num {
	enum_ir_cut0 = 0, /* ir-cut- */
	enum_ir_cut1,     /* ir-cut+ */
	enum_ir_led,
	enum_ir_count,
};

enum Camera_Num {
	enum_camera0 = 0,
	enum_camera1,
	enum_camera_count,
};

struct Camera_Param {
	int sign;
	int addr;
	int adc_channel;
	normal_gpio_cfg gpio_info[enum_ir_count];
};

struct Camera_Param camera_param[] = {
	[enum_camera0] = {
		.sign = CAMERA1_SIGN,
		.addr = CFG_ISPPARAM_LOAD_ADDR,
		.adc_channel = CAMERA1_ADC_CHANNEL,
		.gpio_info = {
			[enum_ir_cut0] = {
				.port = SUNXI_GPIO_D,
				.port_num = 18,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level  = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			},
			[enum_ir_cut1] = {
				.port = SUNXI_GPIO_D,
				.port_num = 8,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			},
			[enum_ir_led] = {
				.port = PORT_NO_USE,
				.port_num = PORT_NUM_NO_USE,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			}
		}
	},
	[enum_camera1] = {
		.sign = CAMERA2_SIGN,
		.addr = CFG_ISPPARAM_LOAD_ADDR + sizeof(SENSOR_ISP_CONFIGS_S),
		.adc_channel = CAMERA2_ADC_CHANNEL,
		.gpio_info = {
			[enum_ir_cut0] = {
				.port = SUNXI_GPIO_D,
				.port_num = 18,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level  = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			},
			[enum_ir_cut1] = {
				.port = SUNXI_GPIO_D,
				.port_num = 8,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			},
			[enum_ir_led] = {
				.port = PORT_NO_USE,
				.port_num = PORT_NUM_NO_USE,
				.mul_sel = SUNXI_GPIO_OUTPUT,
				.pull = SUNXI_GPIO_PULL_DISABLE,
				.drv_level = SUNXI_GPIO_DRV_LEVEL1,
				.data = OUPUT_LOW_LEVEL,
			}
		}
	}

};

#ifdef CFG_SUNXI_FDT
extern int sprintf(char *buf, const char *fmt, ...);
#define FDT_PATH_ISP_INIT_GPIO "/soc/isp_boot0_gpio"
int select_isp_param_from_dts(void)
{
	int nodeoffset;
	char isp_gpioparam_str[16] = { 0 };
	printf("get info from dts:%d\n", __LINE__);
	nodeoffset = fdt_path_offset(working_fdt, FDT_PATH_ISP_INIT_GPIO);
	if (nodeoffset < 0) {
		return 0;
	}
	if (!fdtdec_get_is_enabled(working_fdt, nodeoffset)) {
		return 0;
	}

	for (int i = enum_camera0; i < enum_camera_count; i++) {
		sprintf(isp_gpioparam_str, "camera%d_cut0\0", enum_camera0);
		fdt_get_one_gpio(
			FDT_PATH_ISP_INIT_GPIO, isp_gpioparam_str,
			&camera_param[enum_camera0].gpio_info[enum_ir_cut0]);
		sprintf(isp_gpioparam_str, "camera%d_cut1\0", enum_camera0);
		fdt_get_one_gpio(
			FDT_PATH_ISP_INIT_GPIO, isp_gpioparam_str,
			&camera_param[enum_camera0].gpio_info[enum_ir_cut1]);
		sprintf(isp_gpioparam_str, "camera%d_led\0", enum_camera0);
		fdt_get_one_gpio(
			FDT_PATH_ISP_INIT_GPIO, isp_gpioparam_str,
			&camera_param[enum_camera0].gpio_info[enum_ir_count]);
	}
	return 0;
}
#endif

void ctrl_led_for_isp(normal_gpio_cfg gpio_info[], enum Day_Time day_time)
{
	switch (day_time) {
	case enum_none:
		gpio_info[enum_ir_cut0].data = OUPUT_LOW_LEVEL;
		gpio_info[enum_ir_cut1].data = OUPUT_HIGH_LEVEL;
		gpio_info[enum_ir_led].data = OUPUT_LOW_LEVEL;
		boot_set_gpio_new(gpio_info, enum_ir_count, 1);
		break;
	case enum_day:
		gpio_info[enum_ir_cut0].data = OUPUT_HIGH_LEVEL;
		gpio_info[enum_ir_cut1].data = OUPUT_LOW_LEVEL;
		gpio_info[enum_ir_led].data = OUPUT_LOW_LEVEL;
		boot_set_gpio_new(gpio_info, enum_ir_count, 1);
		break;
	case enum_night:
		gpio_info[enum_ir_cut0].data = OUPUT_LOW_LEVEL;
		gpio_info[enum_ir_cut1].data = OUPUT_HIGH_LEVEL;
		gpio_info[enum_ir_led].data = OUPUT_HIGH_LEVEL;
		boot_set_gpio_new(gpio_info, enum_ir_count, 1);
		break;
	default:
		printf("Input time error\n");
		break;
	}
}

enum Day_Time get_day_or_night(unsigned int vol, int adc_mode,
			       unsigned short light_def)
{
	int temp_light_def = light_def;
	enum Day_Time day_time;
	if (!temp_light_def)
		temp_light_def = DEFAULT_COMPARISON_VALUE;
	if (adc_mode) {
		// the brighter the u16LightValue smaller
		if (vol > temp_light_def) {
			day_time = enum_night;
		} else {
			day_time = enum_day;
		}
	} else {
		if (vol > temp_light_def) {
			day_time = enum_day;
		} else {
			day_time = enum_night;
		}
	}
	return day_time;
}

int update_camera_param(struct Camera_Param camera_param)
{
	/* day--1 night--2*/
	enum Day_Time day_time = 0;
	unsigned int vol       = 0;
	SENSOR_ISP_CONFIGS_S *ispconfig_param =
		(SENSOR_ISP_CONFIGS_S *)(camera_param.addr);
	//sign check
	if (ispconfig_param->sign != camera_param.sign) {
		printf("camera sign error:0x%x\n", ispconfig_param->sign);
		return -1;
	}
	if (ispconfig_param->light_enable != 1) {
		printf("no use isp light\n");
		return -1;
	}

	extern int sunxi_read_gpadc_vol(int channel);
	vol = sunxi_read_gpadc_vol(camera_param.adc_channel);
	printf("gpadc val:%d\n", vol);
	day_time = get_day_or_night(vol, ispconfig_param->adc_mode,
				    ispconfig_param->light_def);
	ctrl_led_for_isp(camera_param.gpio_info, day_time);

	ispconfig_param->ircut_state = day_time;

#if ISP_DEBUG
	printf("=======================================>\n");
	printf("ispconfig_param addr = 0x%x\n", ispconfig_param);
	printf("ispconfig_param->sign = 0x%x\n", ispconfig_param->sign);
	printf("ispconfig_param->light_enable = 0x%x\n",
	       ispconfig_param->light_enable);
	printf("ispconfig_param->adc_mode = 0x%x\n", ispconfig_param->adc_mode);
	printf("ispconfig_param->light_def = 0x%x\n",
	       ispconfig_param->light_def);
	printf("ispconfig_param->ircut_state = 0x%x\n",
	       ispconfig_param->ircut_state);
	printf("===========================================\n");
#endif
	return 0;
}

void update_ir_state(void)
{
	for (int i = enum_camera0; i < enum_camera_count; i++) {
		update_camera_param(camera_param[i]);
	}

#if ISP_DEBUG
	/* dump camera_param*/
	for (int i = enum_camera0; i < enum_camera_count; i++) {
		printf("camera %d\nsign:0x%x\n", i, camera_param[i].sign);
		printf("addr:0x%x\n", camera_param[i].sign);
		printf("adc_channel:0x%x\n", camera_param[i].adc_channel);
		printf("enum_ir_cut0\nport :0x%x port_num:0x%x\nmul_sel:0x%x "
				"pull:0x%x drv_level:0x%x\ndata:0x%x\n",
		       camera_param[i].gpio_info[enum_ir_cut0].port,
		       camera_param[i].gpio_info[enum_ir_cut0].port_num,
		       camera_param[i].gpio_info[enum_ir_cut0].mul_sel,
		       camera_param[i].gpio_info[enum_ir_cut0].pull,
		       camera_param[i].gpio_info[enum_ir_cut0].drv_level,
		       camera_param[i].gpio_info[enum_ir_cut0].data);
		printf("enum_ir_cut1\nport :0x%x port_num:0x%x\nmul_sel:0x%x "
				"pull:0x%x drv_level:0x%x\ndata:0x%x\n",
		       camera_param[i].gpio_info[enum_ir_cut1].port,
		       camera_param[i].gpio_info[enum_ir_cut1].port_num,
		       camera_param[i].gpio_info[enum_ir_cut1].mul_sel,
		       camera_param[i].gpio_info[enum_ir_cut1].pull,
		       camera_param[i].gpio_info[enum_ir_cut1].drv_level,
		       camera_param[i].gpio_info[enum_ir_cut1].data);
		printf("enum_ir_led\nport :0x%x port_num:0x%x\nmul_sel:0x%x "
				"pull:0x%x drv_level:0x%x\ndata:0x%x\n",
		       camera_param[i].gpio_info[enum_ir_led].port,
		       camera_param[i].gpio_info[enum_ir_led].port_num,
		       camera_param[i].gpio_info[enum_ir_led].mul_sel,
		       camera_param[i].gpio_info[enum_ir_led].pull,
		       camera_param[i].gpio_info[enum_ir_led].drv_level,
		       camera_param[i].gpio_info[enum_ir_led].data);
	}
#endif
}
#endif

#ifndef CFG_SUNXI_ENV
/* flag is placed in the last byte of the last region */
unsigned char read_switch_flag_from_kernel(void)
{
	/* camera 1*/
	SENSOR_ISP_CONFIGS_S *ispconfig_param =
		(SENSOR_ISP_CONFIGS_S *)(CFG_ISPPARAM_LOAD_ADDR + sizeof(SENSOR_ISP_CONFIGS_S));

	printf("switch_system_flag value:0x%x\n",
	       ispconfig_param->switch_system_flag);

	return ispconfig_param->switch_system_flag;
}
#endif


void update_isp_param(void)
{
#ifdef CFG_BOOT0_WRITE_IRSATTE_TO_ISP
#ifdef CFG_SUNXI_FDT
	select_isp_param_from_dts();
#endif //CFG_SUNXI_FDT
	update_ir_state();
#endif /*CFG_BOOT0_WRITE_IRSATTE_TO_ISP*/
}
