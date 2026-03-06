/*
 * (C) Copyright 2023
 * wujiayi <wujiayi@allwinnertech.com>
 */
#ifndef __HWSPINLOCK_SUN300IW1_H
#define __HWSPINLOCK_SUN300IW1_H

#define BIT_SPIN_LOCK_GATING	(4)
#define BIT_SPIN_LOCK_RST		(4)
#define SPIN_LOCK_CLK_REG		0x80
#define SPIN_LOCK_RST_REG		0x90
#define SPIN_LOCK_BASE			(0x43005000)
#define INIT_IN_ARM

#endif /* __HWSPINLOCK_SUN300IW1_H */
