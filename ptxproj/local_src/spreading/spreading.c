//------------------------------------------------------------------------------
/// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
///
/// This program is free software: you can redistribute it and/or modify  
/// it under the terms of the GNU General Public License as published by  
/// the Free Software Foundation, version 2.
///
/// This program is distributed in the hope that it will be useful, but 
/// WITHOUT ANY WARRANTY; without even the implied warranty of 
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License 
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

///------------------------------------------------------------------------------
///
/// \file    spreading.c
///
/// \version $Id$
///
/// \brief   spreading module sets spread spectrum mode for PLL2
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
//------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/regmap.h>

//------------------------------------------------------------------------------
// Local macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local typedefs
//------------------------------------------------------------------------------
#define CCM_ANALOG_PLL_SYS_SS_OFFSET 	0x40
#define CCM_ANALOG_PLL_SYS_NUM_OFFSET	0x50 
#define CCM_ANALOG_PLL_SYS_DENOM_OFFSET 0x60
#define ANADIG_PLL_528_SYS_SS_ENABLE 	0x8000

#define TRUE 	1
#define FALSE	0

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
static struct regmap *anatop;

//------------------------------------------------------------------------------
// external functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------
static int enable_spread_spectrum(int state) {

	unsigned long sys_ss = 0x2EE0002;
	unsigned long denom = 0x4B0;
	u32 val;
	
	/*
	denom | sys_ss    | spread spectrum range | modulation frequency | frequency step
	0x190 | 0x0FA0001 | 15MHz                 | 48 kHz               | 60 kHz
	0x4B0 | 0x2EE0002 | 15MHz                 | XX kHz               | 40 kHz
	* sys_ss = (ssc_top=stop -> bit 31 - 16) | (enable bit 15 ) | (step bit 14-0)
	* frequency change = (stop/CCM_ANALOG_PLL_SYS_DENOM[B])*24MHz
	* frequency change step = (step/CCM_ANALOG_PLL_SYS_DENOM[B])*24MHz
	* step = (step/dnom)*24MHz
	*/

	anatop = syscon_regmap_lookup_by_compatible("fsl,imx6q-anatop");
	if (IS_ERR(anatop)) {
		pr_err("%s: failed to find imx6q-anatop regmap!\n", __func__);
	}

	/* deactivate spread spectrum mode */
	regmap_read(anatop,  CCM_ANALOG_PLL_SYS_SS_OFFSET, &val );
	regmap_write(anatop,  CCM_ANALOG_PLL_SYS_SS_OFFSET, val & ~ANADIG_PLL_528_SYS_SS_ENABLE);

	/* write new values */
	regmap_write(anatop,CCM_ANALOG_PLL_SYS_SS_OFFSET, sys_ss);
	regmap_write(anatop,CCM_ANALOG_PLL_SYS_DENOM_OFFSET, denom);
  
	if (state) {
		regmap_read(anatop,  CCM_ANALOG_PLL_SYS_SS_OFFSET, &val );
		regmap_write(anatop,  CCM_ANALOG_PLL_SYS_SS_OFFSET, val  | ANADIG_PLL_528_SYS_SS_ENABLE);
		printk("Spreading module loaded\n");
	}

	return 0;
}

static int __init spread_spectrum_init(void) {
	enable_spread_spectrum(TRUE);
	return 0;
}

static void __exit spread_spectrum_exit(void) {
	enable_spread_spectrum(FALSE);

}

MODULE_LICENSE("GPL");
module_init(spread_spectrum_init);
module_exit(spread_spectrum_exit);
