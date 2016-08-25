/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


#ifdef BUILD_LK
#else
#include <linux/string.h>
#if defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#endif
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#define LCM_ID (0x9881)

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
static LCM_UTIL_FUNCS lcm_util = {0};

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int *rawdata);

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,0x03,{0x98,0x81,0x03}},

//GIP_1
{0x01,0x01,{0x00}},
{0x02,0x01,{0x00}},
{0x03,0x01,{0x72}},
{0x04,0x01,{0x00}},
{0x05,0x01,{0x00}},
{0x06,0x01,{0x09}},
{0x07,0x01,{0x00}},
{0x08,0x01,{0x00}},
{0x09,0x01,{0x01}},
{0x0A,0x01,{0x00}},
{0x0B,0x01,{0x00}},
{0x0C,0x01,{0x01}},
{0x0D,0x01,{0x00}},
{0x0E,0x01,{0x00}},
{0x0F,0x01,{0x14}},
{0x10,0x01,{0x14}}, 
{0x11,0x01,{0x00}},
{0x12,0x01,{0x00}},
{0x13,0x01,{0x00}},
{0x14,0x01,{0x00}},
{0x15,0x01,{0x00}},
{0x16,0x01,{0x00}}, 
{0x17,0x01,{0x00}},
{0x18,0x01,{0x00}},
{0x19,0x01,{0x00}},
{0x1A,0x01,{0x00}},
{0x1B,0x01,{0x00}},
{0x1C,0x01,{0x00}},
{0x1D,0x01,{0x00}},
{0x1E,0x01,{0x40}},
{0x1F,0x01,{0x80}},
{0x20,0x01,{0x05}},
{0x21,0x01,{0x02}},
{0x22,0x01,{0x00}},
{0x23,0x01,{0x00}},
{0x24,0x01,{0x00}},
{0x25,0x01,{0x00}},
{0x26,0x01,{0x00}},
{0x27,0x01,{0x00}},
{0x28,0x01,{0x33}},
{0x29,0x01,{0x02}},
{0x2A,0x01,{0x00}},
{0x2B,0x01,{0x00}},
{0x2C,0x01,{0x00}},
{0x2D,0x01,{0x00}},
{0x2E,0x01,{0x00}},
{0x2F,0x01,{0x00}},
{0x30,0x01,{0x00}},
{0x31,0x01,{0x00}},
{0x32,0x01,{0x00}},
{0x33,0x01,{0x00}},
{0x34,0x01,{0x04}},
{0x35,0x01,{0x00}},
{0x36,0x01,{0x00}},
{0x37,0x01,{0x00}},
{0x38,0x01,{0x78}},
{0x39,0x01,{0x00}},
{0x3A,0x01,{0x40}}, 
{0x3B,0x01,{0x40}},
{0x3C,0x01,{0x00}},
{0x3D,0x01,{0x00}},
{0x3E,0x01,{0x00}},
{0x3F,0x01,{0x00}},
{0x40,0x01,{0x00}},
{0x41,0x01,{0x00}},
{0x42,0x01,{0x00}},
{0x43,0x01,{0x00}},
{0x44,0x01,{0x00}},



//GIP_2
{0x50,0x01,{0x01}},
{0x51,0x01,{0x23}},
{0x52,0x01,{0x45}},
{0x53,0x01,{0x67}},
{0x54,0x01,{0x89}},
{0x55,0x01,{0xAB}},
{0x56,0x01,{0x01}},
{0x57,0x01,{0x23}},
{0x58,0x01,{0x45}},
{0x59,0x01,{0x67}},
{0x5A,0x01,{0x89}},
{0x5B,0x01,{0xAB}},
{0x5C,0x01,{0xCD}},
{0x5D,0x01,{0xEF}},

//GIP_3
{0x5E,0x01,{0x11}},
{0x5F,0x01,{0x01}},
{0x60,0x01,{0x00}},
{0x61,0x01,{0x15}},
{0x62,0x01,{0x14}},
{0x63,0x01,{0x0E}},
{0x64,0x01,{0x0F}},
{0x65,0x01,{0x0C}},
{0x66,0x01,{0x0D}},
{0x67,0x01,{0x06}},
{0x68,0x01,{0x02}},
{0x69,0x01,{0x07}},
{0x6A,0x01,{0x02}},
{0x6B,0x01,{0x02}},
{0x6C,0x01,{0x02}},
{0x6D,0x01,{0x02}},
{0x6E,0x01,{0x02}},
{0x6F,0x01,{0x02}},
{0x70,0x01,{0x02}},
{0x71,0x01,{0x02}},
{0x72,0x01,{0x02}},
{0x73,0x01,{0x02}},
{0x74,0x01,{0x02}},
{0x75,0x01,{0x01}},
{0x76,0x01,{0x00}},
{0x77,0x01,{0x14}},
{0x78,0x01,{0x15}},
{0x79,0x01,{0x0E}},
{0x7A,0x01,{0x0F}},
{0x7B,0x01,{0x0C}},
{0x7C,0x01,{0x0D}},
{0x7D,0x01,{0x06}},
{0x7E,0x01,{0x02}},
{0x7F,0x01,{0x07}},
{0x80,0x01,{0x02}},
{0x81,0x01,{0x02}},
{0x82,0x01,{0x02}},
{0x83,0x01,{0x02}},
{0x84,0x01,{0x02}},
{0x85,0x01,{0x02}},
{0x86,0x01,{0x02}},
{0x87,0x01,{0x02}},
{0x88,0x01,{0x02}},
{0x89,0x01,{0x02}},
{0x8A,0x01,{0x02}},

//CMD_PAgE 4
{0xFF,0x03,{0x98,0x81,0x04}},
{0x00,0x01,{0x80}},               ///3 lane     80Ϊ��ͨ��
{0x6C,0x01,{0x15}},               //SEt VCORE voltAgE =1.5V
{0x6E,0x01,{0x2A}},               //Di_pwr_rEg=0 For powEr moDE 2A //VGH ClAmp 15V
{0x6F,0x01,{0x33}},               // rEg vCl + pumping rAtio VGH=3x VGL=-2.5x
{0x3A,0x01,{0x94}},               //POWER SAVING
{0x8D,0x01,{0x1A}},               //VGL ClAmp -11V
{0x87,0x01,{0xBA}},               //ESD               
{0x26,0x01,{0x76}},            
{0xB2,0x01,{0xD1}},
{0xB5,0x01,{0x06}},
{0x17,0x01,{0x0C}},


//CMD_PAgE 1
{0xFF,0x03,{0x98,0x81,0x01}},
{0x22,0x01,{0x0A}},		            //BGR, SS
{0x31,0x01,{0x00}},		            //Column invErsion
{0x53,0x01,{0x8C}},		            //VCOM1
{0x55,0x01,{0x8F}},		            //VCOM2
{0x50,0x01,{0xC0}},		            //VREG1OUT=5V
{0x51,0x01,{0xC0}},		            //VREG2OUT=-5V
{0x60,0x01,{0x08}},               //SDT

{0xA0,0x01,{0x08}},		            //VP255	GAmmA P
{0xA1,0x01,{0x19}},               //VP251        
{0xA2,0x01,{0x26}},               //VP247        
{0xA3,0x01,{0x1A}},               //VP243        
{0xA4,0x01,{0x1D}},               //VP239        
{0xA5,0x01,{0x2C}},               //VP231        
{0xA6,0x01,{0x21}},               //VP219        
{0xA7,0x01,{0x22}},               //VP203        
{0xA8,0x01,{0x7C}},               //VP175        
{0xA9,0x01,{0x21}},               //VP144        
{0xAA,0x01,{0x2E}},               //VP111        
{0xAB,0x01,{0x66}},               //VP80         
{0xAC,0x01,{0x1C}},               //VP52         
{0xAD,0x01,{0x18}},               //VP36         
{0xAE,0x01,{0x4E}},               //VP24         
{0xAF,0x01,{0x1A}},               //VP16         
{0xB0,0x01,{0x22}},               //VP12         
{0xB1,0x01,{0x49}},               //VP8          
{0xB2,0x01,{0x56}},               //VP4          
{0xB3,0x01,{0x39}},               //VP0          
                                               
{0xC0,0x01,{0x08}},               //VN255 GAMMA N
{0xC1,0x01,{0x1A}},               //VN251        
{0xC2,0x01,{0x26}},               //VN247        
{0xC3,0x01,{0x0B}},               //VN243        
{0xC4,0x01,{0x0E}},               //VN239        
{0xC5,0x01,{0x24}},               //VN231        
{0xC6,0x01,{0x18}},               //VN219        
{0xC7,0x01,{0x1B}},               //VN203        
{0xC8,0x01,{0x85}},               //VN175        
{0xC9,0x01,{0x17}},               //VN144        
{0xCA,0x01,{0x23}},               //VN111        
{0xCB,0x01,{0x79}},               //VN80         
{0xCC,0x01,{0x1C}},               //VN52         
{0xCD,0x01,{0x1F}},               //VN36         
{0xCE,0x01,{0x50}},               //VN24         
{0xCF,0x01,{0x2D}},               //VN16         
{0xD0,0x01,{0x31}},               //VN12         
{0xD1,0x01,{0x49}},               //VN8          
{0xD2,0x01,{0x57}},               //VN4          
{0xD3,0x01,{0x39}},               //VN0

//CMD_PAgE 0
{0xFF,0x03,{0x98,0x81,0x00}},
{0x35,0x01,{0x00}},	//TE on
{0x11,0x01,{0x00}},	//slEEp out
{REGFLAG_DELAY, 120, {}},
{0x29,0x01,{0x00}},	//DisplAy on

};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy((void*)&lcm_util, (void*)util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset((void*)params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
    	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;

		params->dsi.vertical_sync_active				= 5;//10
		params->dsi.vertical_backporch					= 85;//12
		params->dsi.vertical_frontporch					= 85;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 150;//100
		params->dsi.horizontal_frontporch				= 150;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// params->dsi.horizontal_blanking_pixel				 = 60;

		// Bit rate calculation
		//params->dsi.pll_div1=29;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		//params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

#if 0
        params->dsi.pll_div1=1;     // div1=0,1,2,3;div1_real=1,2,4,4
        params->dsi.pll_div2=1;     // div2=0,1,2,3;div1_real=1,2,4,4
        params->dsi.fbk_div =30;     // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#else
		params->dsi.PLL_CLOCK = 260;//260;//250;//403;
#endif
		//params->dsi.ssc_disable = 1;  // ssc disable control

		params->dsi.esd_check_enable = 1;
		params->dsi.customization_esd_check_enable = 1;
		params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
		params->dsi.lcm_esd_check_table[0].count        = 1;
		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

}


static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(1);//(20);
    SET_RESET_PIN(0);
    MDELAY(10);//(40);
    SET_RESET_PIN(1);
    MDELAY(120);//(80);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);

}
#endif


static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[4];
	int id = 0;

	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(40);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0]=0x00063902;
	array[1]=0x8198FFFF; //enable CMD2
	array[2]=0x0000010C;
	dsi_set_cmdq(&array, 3, 1);
	MDELAY(10);

	array[0] = 0x00033700; // return byte
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x00, &buffer[0], 1); // 0x98

	read_reg_v2(0x01, &buffer[1], 1); // 0x81

	read_reg_v2(0x02, &buffer[2], 1); // 0x0C

	id = (buffer[0]<<8) | buffer[1];

#if defined(BUILD_LK)
	printf("ili9881c id djn lk = 0x%02x,buf[2] = 0x%02x,0x%02x,0x%02x\n", id, buffer[0],buffer[1],buffer[2]);
#else
	printk("ili9881c id djn lk = 0x%02x,buf[2] = 0x%02x,0x%02x,0x%02x\n", id, buffer[0],buffer[1],buffer[2]);
#endif
	
	if((id == LCM_ID) && ( 0x04 == buffer[2]))
	{
#if 0
		// get ADC value to distribute different LCM size
		int adcdata[4];
		int rawdata=0;

		IMM_GetOneChannelValue(2,adcdata,&rawdata);
		rawdata = rawdata * 1500/4096;
		#if defined(BUILD_LK)
		printf("ili9881c djn_hd720 channelValue=%d\n",rawdata);
		#else
		printk("ili9881c djn_hd720 channelValue=%d\n",rawdata);
		#endif
		if(rawdata > 500 && rawdata < 900) //djn 0.7V
#endif
			return 1;
	}
	return 0;
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    unsigned char buffer[1];
    unsigned int array[16];
    
    array[0] = 0x00013700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);

    read_reg_v2(0x0A, buffer, 1);

    printk("lcm_esd_check  0x0A = %x\n",buffer[0]);

    if(buffer[0] != 0x9C)  
    {
        return TRUE;
    }
#endif
    return FALSE;
}

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK

  unsigned int data_array[16];

    printk("lcm_esd_recover enter \n");
    
    lcm_init();

    data_array[0]=0x00110500;
    dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(50);
    
    data_array[0]=0x00290500;
    dsi_set_cmdq(&data_array, 1, 1);
    
    data_array[0]= 0x00023902;
    data_array[1]= 0xFF51;
    dsi_set_cmdq(&data_array, 2, 1);
    MDELAY(10);
#endif

    return TRUE;
}


LCM_DRIVER ili9881c_hd_dsi_vdo_djn_lcm_drv = 
{
    .name			= "ili9881c_hd_dsi_vdo_djn",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id		= lcm_compare_id,	
	//.esd_check   = lcm_esd_check,
  	//.esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};

