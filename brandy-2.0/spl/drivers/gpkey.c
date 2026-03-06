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
#include <asm/io.h>
#include <arch/physical_key.h>
#include <arch/clock.h>

int sunxi_clock_init_gpadc(void);

int sunxi_gpadc_init(void)
{
	uint reg_val = 0;

	sunxi_clock_init_gpadc();

	/*ADC sample ferquency divider*ferquency=CLK_IN/(n+1), 24000000/(0x5dbf+1)=1000Hz
	* ADC acquire time
	* time=CLK_IN/(n+1) ,24000000/(0x2f+1)=500000Hz
	*/
	reg_val = readl(GP_SR_CON);
	reg_val &= ~(0xffff << 16);
	reg_val |= (0x5dbf << 16);
	writel(reg_val, GP_SR_CON);


	/*choose continue work mode*/
	reg_val = readl(GP_CTRL);
	reg_val &= ~(1<<18);
	reg_val |= (1 << 19);
	writel(reg_val, GP_CTRL);

	/*enable ADC*/
	reg_val = readl(GP_CTRL);
	reg_val |= (1 << 16);
	writel(reg_val, GP_CTRL);

	/* disable all key irq */
	writel(0, GP_DATA_INTC);
	writel(1, GP_DATA_INTS);


	return 0;
}


int sunxi_read_gpadc(int channel)
{

	u32 ints;
	u32 adc_val = 0;

	/*choose channel */
	setbits_le32(GP_CS_EN, 0x01 << channel);
	udelay(1500);

	ints = readl(GP_DATA_INTS);

	/* clear the pending status */
	writel((ints & (0x1 << channel)), GP_DATA_INTS);
	//writel((ints & 0x7), GP_DATA_INTS);

	/* if there is already data pending,read it .
	 * gpadc_data = adc_val/Vref*4095, and Vref=1.8v.
	 * adc_val should be 0~1.8v.
	 */
	if (ints & (GPADC0_DATA_PENDING << channel)) {
		adc_val = readl(GP_CH0_DATA + channel*4);
		//key = adc_val > 16 ? 0 : (readl(GP_CH0_DATA + channel*4)*63/4095);
	}

	return adc_val;
}

#define ADC_NUM 3
u32 get_sys_ticks(void);
int sunxi_read_gpadc_vol(int channel)
{
	int i, sum, temp_gpadc[ADC_NUM];
	int old_time = get_sys_ticks();
	while (1) {
		sum = 0;
		for (i = 0; i < ADC_NUM; i++) {
			temp_gpadc[i] = sunxi_read_gpadc(channel)*1800/4095;
			sum += temp_gpadc[i];
		/* printf("sum:%d\ttemp_gpadc[%d]:%d\n", sum, i, temp_gpadc[i]); */
		}
		/* printf("sum:%d\ttemp_gpadc[0]:%d\n", sum/ADC_NUM, temp_gpadc[0]); */
		if (abs(sum/ADC_NUM - temp_gpadc[0]) < 5) {
			break;
		} else if (get_sys_ticks() - old_time >= 2000) {
			printf("get adc time out!!!\n");
			break;
		}
	}
	return sum/ADC_NUM;
}

int sunxi_read_gpadc_key(int channel)
{
	int adc_val = sunxi_read_gpadc(channel);
	return adc_val > 16 ? 0 : adc_val*63/4095;
}
