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
#ifndef __ARM_MODE_H__
#define __ARM_MODE_H__

//ARMV7
#define  ARMV7_USR_MODE           0x10
#define  ARMV7_FIQ_MODE           0x11
#define  ARMV7_IRQ_MODE           0x12
#define  ARMV7_SVC_MODE           0x13
#define  ARMV7_MON_MODE        	  0x16
#define  ARMV7_ABT_MODE           0x17
#define  ARMV7_UND_MODE           0x1b
#define  ARMV7_SYSTEM_MODE        0x1f
#define  ARMV7_MODE_MASK          0x1f
#define  ARMV7_FIQ_MASK           0x40
#define  ARMV7_IRQ_MASK           0x80

//ARM9
#define ARM9_USR26_MODE           0x00
#define ARM9_FIQ26_MODE           0x01
#define ARM9_IRQ26_MODE           0x02
#define ARM9_SVC26_MODE           0x03
#define ARM9_USR_MODE             0x10
#define ARM9_FIQ_MODE             0x11
#define ARM9_IRQ_MODE             0x12
#define ARM9_SVC_MODE             0x13
#define ARM9_ABT_MODE             0x17
#define ARM9_UND_MODE             0x1b
#define ARM9_SYSTEM_MODE          0x1f
#define ARM9_MODE_MASK            0x1f
#define ARM9_FIQ_MASK             0x40
#define ARM9_IRQ_MASK             0x80
#define ARM9_CC_V_BIT             (1 << 28)
#define ARM9_CC_C_BIT             (1 << 29)
#define ARM9_CC_Z_BIT             (1 << 30)
#define ARM9_CC_N_BIT             (1 << 31)


// coprocessor CP15
// C1
#define C1_M_BIT                  (1 << 0)
#define C1_A_BIT                  (1 << 1)
#define C1_C_BIT                  (1 << 2)
#define C1_W_BIT                  (1 << 3)
#define C1_P_BIT                  (1 << 4)
#define C1_D_BIT                  (1 << 5)
#define C1_L_BIT                  (1 << 6)
#define C1_B_BIT                  (1 << 7)
#define C1_S_BIT                  (1 << 8)
#define C1_R_BIT                  (1 << 9)
#define C1_F_BIT                  (1 << 10)
#define C1_Z_BIT                  (1 << 11)
#define C1_I_BIT                  (1 << 12)
#define C1_V_BIT                  (1 << 13)
#define C1_RR_BIT                 (1 << 14)
#define C1_L4_BIT                 (1 << 15)

#endif
