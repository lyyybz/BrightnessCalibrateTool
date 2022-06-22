
#include<I2C.h>
#include<delay.h>
#include "config.h"
#include "camera_dev.h"


#define DELAY_FLAG        0xFE   // first parameter is 0xfe, then 2nd parameter is delay time count

#define WIN_Y0_HIGH_ADDR     0x05
#define WIN_Y0_LOW_ADDR      0x06
#define WIN_X0_HIGH_ADDR     0x07
#define WIN_X0_LOW_ADDR      0x08
#define WIN_WIDTH_HIGH_ADDR  0x9
#define WIN_WIDTH_LOW_ADDR   0xa
#define WIN_HEIGHT_HIGH_ADDR 0xb
#define WIN_HEIGHT_LOW_ADDR  0xc

static const unsigned char  RESET_TAB[] =
{
    0xfe,  0x80,
    0xfe,  0x00,

#if CAMERA_WINDOW_MODE
#if 0

    0x05, (WIN_DEFAULT_Y0)>>8, // row_start_high 128
                          0x06, (WIN_DEFAULT_Y0)&0xff, // row_start_low 128
                          0x07, (WIN_DEFAULT_X0)>>8, // col_start_high
                          0x08, (WIN_DEFAULT_X0)&0xff, // col_start_low
                          0x09, (WIN_DEFAULT_HEIGHT*2+8)>>8, //[8]cis_win_height  488
                          0x0a, (WIN_DEFAULT_HEIGHT*2+8)&0xff, //[7:0]cis_win_height
                          0x0b, (WIN_DEFAULT_CAPUTE_WIDTH*2 +8)>>8, //[9:8]cis_win_width 648
                          0x0c, (WIN_DEFAULT_CAPUTE_WIDTH*2 +8)&0xff, //[7:0]cis_win_width
#endif
#else
    0x05, 0x00, // row_start_high
    0x06, 0x00, // row_start_low
    0x07, 0x00, // col_start_high
    0x08, 0x00, // col_start_low
    0x09, 0x01, //[8]cis_win_height  488
    0x0a, 0xe8, //[7:0]cis_win_height
    0x0b, 0x02, //[9:8]cis_win_width 648
    0x0c, 0x88, //[7:0]cis_win_width
#endif
                          0x0d, 0x02,//vs_st
                          0x0e, 0x02,//vs_et
                          0x10, 0x26,//[7:4]restg_width, [3:0]sh_width
                          0x11, 0x0d,//fd//[7:4]tx_width, [3:0]space width,*2
                          0x12, 0x2a,//sh_delay
                          0x13, 0x00,//[3:0] row_tail_width
#ifdef 	ESSN_WCR_A
	0X14,0x10,//[7]hsync_always ,[6] NA,  [5:4] CFA sequence[3:2]NA,  [1]upside_down,  [0] mirror
#else
    #ifdef 	ESSN_WCR_B
	     0x14, 0x10,
	#else 
	     0x14, 0x13,
	#endif
#endif 				 
                          0x15, 0x0a,//[7:6]output_mode,,[5:4]restg_mode,[3:2]sdark_mode, [1]new exposure,[0]badframe_en
                          0x16, 0x05,//[7:5]NA, [4]capture_ad_data_edge, [3:0]Number of A/D pipe stages
                          0x17, 0x01,//[7:6]analog_opa_r,[5]coltest_en, [4]ad_test_enable,
                          //[3]tx_allow,[2]black sun correction,[1:0]black sun control reg
                          0x18, 0x44,//[7]NA,  [6:4]column gain ee, [3]NA, [2:0]column gain eo
                          0x19, 0x44,//[7]NA,  [6:4]column gain oe, [3]NA, [2:0]column gain oo
                          0x1a, 0x2a,//1e//[7]rsv1,[6]rsv0, [5:4]coln_r,
                          //[3:2]colg_r column gain opa bias current, [1]clk_delay, [0] apwd
                          0x1b, 0x00,//[7:2]NA, [1:0]BIN4 AND BIN2
                          0x1c, 0x49,//c1//[7]hrst_enbale, [6:4]da_rsg, [3]tx high enable, [2]NA, [1:0]da18_r
                          0x1d, 0x9a,//08//[7]vref_en, [6:4]da_vef, [3]da25_en, [2]NA, [1:0]da25_r,set da25 voltage
                          0x1e, 0x61,//60//[7]LP_MTD,[6:5]opa_r,ADC's operating current,  [4:2]NA, [1:0]sref
                          0x1f, 0x15,//[7:6]NA, [5:4]sync_drv, [3:2]data_drv, [1:0]pclk_drv

                          0x20, 0xff,//[7]bks[6]gamma[5]cc[4]ee[3]intp[2]dn[1]dd[0]lsc

                          0x21, 0xf8,//[7]na[6]na[5]skin_ee[4]cbcr_hue_en[3]y_as_en[2]auto_gray_en[1]y_gamma_en[0]na
                          0x22, 0x55,//[7]na [6]auto_dndd [5]auto_ee [4]auto_sa [3]na [2]abs [1]awb  [0]na
#if ONLY_MONO_CHANEL
                          0x24, 0xb5,//0xb1:y  0xb4: red  0xb5:green 0xb6:blue
#else
                          0x24, 0xa6,//0x24, 0xa6,  rgb565
#endif
                          0x25, 0x0f,
                          //output sync_mode
#if	CAPTURE_MAIN_CLK_108MHZ && ONLY_MONO_CHANEL
                          0x26, 0x8a,//
#elif 	CAPTURE_MAIN_CLK_108MHZ && (!ONLY_MONO_CHANEL)
                          0x26, 0x02,//02 数据滞后半个clock
#else
                          0x26, 0x82,//
#endif

                          0x28, 0x21, // 0x11  2 div 分频配置,0x32 1/4 分频
                          0x2f, 0x01,//debug mode3
                          //////////////////////////////////////////////////////////////
                          ///////////////////// grab     ////////////////////////////////
                          ///////////////////////////////////////////////////////////////
                          0x30, 0xf7,//blk mode [7]dark current mode:1 use exp rated dark ,0 use ndark row calculated
                          //[1]dark_current_en//[0]offset_en
                          0x31, 0x50,//blk_value limit.64 low align to 11bits;8 for 256 range
                          0x32, 0x00,//global offset
                          0x39, 0x04,// exp_ate_darkc
                          0x3a, 0x20,//{7:6}offset submode {5:0}offset ratio
                          0x3b, 0x20,//{7:6}darkc submode {5:0}dark current ratio
                          0x3c, 0x00,//manual g1 offset
                          0x3d, 0x00,//manual r offset
                          0x3e, 0x00,//manual b offset
                          0x3f, 0x00,//manual g2 offset
                          ///////////gain////////
                          0x50, 0x14,//10  //global gain

                          0x53, 0x80,//G
                          0x54, 0x80,//R channel gain
                          0x55, 0x80,//B channel gain
                          0x56, 0x80,
                          /////////////////////////////////////////////////////////
                          //////////////// LSC_t    ////////////////////////////////
                          //////////////////////////////////////////////////////////
                          0x8b, 0x20,//r2
                          0x8c, 0x20,//g2
                          0x8d, 0x20,//b2
                          0x8e, 0x14,//r4
                          0x8f, 0x10,//g4
                          0x90, 0x14,//b4
                          0x91, 0x3c,//[7]singed4 [6:0]row_cneter
                          0x92, 0x50,//col_center
                          0x5d, 0x12,//decrease 1
                          0x5e, 0x1a,//decrease 2
                          0x5f, 0x24,//decrease 3
                          //////////////////////////////////////////////////////////
                          //////////////// DNDD_t    ///////////////////////////////
                          //////////////////////////////////////////////////////////
                          0x60, 0x07,//[4]zero weight mode
                          //[3]share mode
                          //[,]c weight mode
                          //[,]lsc decrease mode
                          //[,]b mode
                          0x61, 0x15,//[7:6]na
                          //[5:4]c weight adap ratio
                          //[,:2]dn lsc ratio
                          //[,:0]b ratio
                          0x62, 0x08,//b base
                          //0x63,0x02,//b increase RO
                          0x64, 0x03,//[7:4]n base [3:0]c weight
                          //0x65,  ,//[7:4]n increase [3:0]c coeff
                          0x66, 0xe8,//dark_th ,bright_th
                          0x67, 0x86,//flat high, flat low
                          0x68, 0xa2,//[7:4]dd limit [1:0]dd ratio

//////////////////////////////////////////////////////////
//////////////// asde_t    ///////////////////////////////
//////////////////////////////////////////////////////////
                          0x69, 0x18,//gain high th
                          0x6a, 0x0f,//[7:4]dn_c slop          //[3]use post_gain [2]use pre_gain [1]use global gain [0]use col gain
                          0x6b, 0x00,//[7:4]dn_b slop [3:0]dn_n slop
                          0x6c, 0x5f,//[7:4]bright_th start [3:0]bright_th slop
                          0x6d, 0x8f,//[7:4]dd_limit_start[3:0]dd_limit slop
                          0x6e, 0x55,//[7:4]ee1 effect start [3:0]slope  broad
                          0x6f, 0x38,//[7:4]ee2 effect start [3:0]slope  narrow
                          0x70, 0x15,//saturation dec slope
                          0x71, 0x33,//[7:4]low limit,[3:0]saturation slope
                          /////////////////////////////////////////////////////////
                          /////////////// eeintp_t    ///////////////////////////////
                          /////////////////////////////////////////////////////////
                          0x72, 0xdc,//[7]edge_add_mode [6]new edge mode [5]edge2_mode [4]HP_mode
                          //[3]lp intp en [2]lp edge en [1:0]lp edge mode

                          0x73, 0x80,//[7]edge_add_mode2 [6]NA [5]only 2direction [4]fixed direction th
                          //[3]only defect map [2]intp_map dir [1]HP_acc [0]only edge map


                          //////for high resolution in light scene
                          0x74, 0x02,//direction th1
                          0x75, 0x3f,//direction th2
                          0x76, 0x02,//direction diff th      h>v+diff ; h>th1 ; v<th2
                          0x77, 0x36,//[7:4]edge1_effect [3:0]edge2_effect
                          0x78, 0x88,//[7:4]edge_pos_ratio  [3:0]edge neg ratio
                          0x79, 0x81,//edge1_max,edge1_min
                          0x7a, 0x81,//edge2_max,edge2_min
                          0x7b, 0x22,//edge1_th,edge2_th
                          0x7c, 0xff,//pos_edge_max,neg_edge_max
//////for high resolution in light scene

                          ///cct
                          0x93, 0x48,// <--40
                          0x94, 0x02,
                          0x95, 0x07,
                          0x96, 0xe0,
                          0x97, 0x40,
                          0x98, 0xf0,
//ycpt

                          0xb1, 0x46, //manual cb
                          0xb2, 0x46, //manual cr
                          0xb3, 0x42, //0x40
                          0xb6, 0xe0,
                          0xbd, 0x38,
                          0xbe, 0x36, // [5:4]gray mode 00:4&8  01:4&12 10:4&20  11:8$16   [3:0] auto_gray



////AECT
                          0xd0, 0xcb, //cb    exp is gc mode
                          0xd1, 0x10,//every N
#if 1
                          0xd2, 0x20, // 7 aec enable 5 clore y mode 4skin weight 3 weight drop mode
#else
                          0xd2, 0x90, // 7 aec enable 5 clore y mode 4skin weight 3 weight drop mode
#endif

                          0xd3, 0x4e, //Y_target and low pixel thd high X4 low X2   //0x48

                          0xb5, 0x00,
                          0xd5, 0xf2,//lhig
                          0xd6, 0x16,// ignore mode
                          0xdb, 0x92,
                          0xdc, 0xa5,//fast_margin  fast_ratio
                          0xdf, 0x23,// I_fram D_ratio

                          0xd9, 0x00,// colore offset in CAL ,now is too dark so set zero
                          0xda, 0x00,// GB offset
                          0xe0, 0x09,


                          0xed, 0x04, //  04minimum exposure low 8  bits
                          0xee, 0xa0, //max_post_dg_gain
                          0xef, 0x40, //max_pre_dg_gain
                          0x80, 0x03,

////abbt
                          0x80, 0x03,
                          ////RGBgama_m5
                          0x9F, 0x14,
                          0xA0, 0x28,
                          0xA1, 0x44,
                          0xA2, 0x5d,
                          0xA3, 0x72,
                          0xA4, 0x86,
                          0xA5, 0x95,
                          0xA6, 0xb1,
                          0xA7, 0xc6,
                          0xA8, 0xd5,
                          0xA9, 0xe1,
                          0xAA, 0xEa,
                          0xAB, 0xf1,
                          0xAC, 0xF5,
                          0xAD, 0xFb,
                          0xAE, 0xFe,
                          0xAF, 0xFF,
///wint
///Y_gamma
                          0xc0, 0x00,//Y_gamma_0
                          0xc1, 0x10,//Y_gamma_1
                          0xc2, 0x1C,//Y_gamma_2
                          0xc3, 0x30,//Y_gamma_3
                          0xc4, 0x43,//Y_gamma_4
                          0xc5, 0x54,//Y_gamma_5
                          0xc6, 0x65,//Y_gamma_6
                          0xc7, 0x75,//Y_gamma_7
                          0xc8, 0x93,//Y_gamma_8
                          0xc9, 0xB0,//Y_gamma_9
                          0xca, 0xCB,//Y_gamma_10
                          0xcb, 0xE6,//Y_gamma_11
                          0xcc, 0xFF,//Y_gamma_12
/////ABS
                          0xf0, 0x02,
                          0xf1, 0x01,
                          0xf2, 0x02,//manual stretch K
                          0xf3, 0x30,//the limit of Y_stretch

#if CAMERA_WINDOW_MODE
#if    0
                          0xf7, 0x12,
                          0xf8, (0*2)/4 + 2,
                          0xf9, 0x9f,
                          0xfa, ((WIN_DEFAULT_HEIGHT)*2)/4 -2,
#endif
#else
                          0xf7, 0x12,
                          0xf8, 0x0a,
                          0xf9, 0x9f,
                          0xfa, 0x78,
#endif
                          ////AWB
#if 1
                          0xfe, 0x01,
                          0x00, 0xf5,//high_low limit
                          0x02, 0x20,//y2c
                          0x04, 0x10,
                          0x05, 0x08,
                          0x06, 0x20,
                          0x08, 0x0a,
                          0x0a, 0xa0,// number limit
                          0x0b, 0x60,// skip_mode
                          0x0c, 0x08,
                          0x0e, 0x44, // width step
                          0x0f, 0x32, // height step
                          0x10, 0x41,
                          0x11, 0x37,  //0x3f
                          0x12, 0x22,
                          0x13, 0x19,//13//smooth 2
                          0x14, 0x44,//R_5k_gain_base
                          0x15, 0x44,//B_5k_gain_base
                          0x16, 0xc2, //c2//sinT
                          0x17, 0xa8, //ac//a8//a8//a8//cosT
                          0x18, 0x18,//X1 thd
                          0x19, 0x50,//X2 thd
                          0x1a, 0xd8,//e4//d0//Y1 thd
                          0x1b, 0xf5,//Y2 thd
                          0x70, 0x40, // A R2G low
                          0x71, 0x58, // A R2G high
                          0x72, 0x30, // A B2G low
                          0x73, 0x48, // A B2G high
                          0x74, 0x20, // A G low
                          0x75, 0x60, // A G high
                          0x77, 0x20,
                          0x78, 0x32,

                          ///hsp
                          0x30, 0x03,//[1]HSP_en [0]sa_curve_en
                          0x31, 0x40,
                          0x32, 0x10,
                          0x33, 0xe0,
                          0x34, 0xe0,
                          0x35, 0x00,
                          0x36, 0x80,
                          0x37, 0x00,
                          0x38, 0x04,//sat1, at8
                          0x39, 0x09,
                          0x3a, 0x12,
                          0x3b, 0x1C,
                          0x3c, 0x28,
                          0x3d, 0x31,
                          0x3e, 0x44,
                          0x3f, 0x57,
                          0x40, 0x6C,
                          0x41, 0x81,
                          0x42, 0x94,
                          0x43, 0xA7,
                          0x44, 0xB8,
                          0x45, 0xD6,
                          0x46, 0xEE, //sat15,at224
                          0x47, 0x0d,//blue_edge_dec_ratio


#endif
                          //输出1 /2 采样
                          0x53, 0x82,
                          0x54, 0x22,
                          0x56, 0x0,
                          0x57, 0x0,
                          0x58, 0x0,
                          0x59, 0x0,
                          0x55, 0x01,

                          ///out
                          0xfe, 0x00,

                          /*
                          	 0x01,0x32,   //    20M
                          	 0x02,0x70,
                          	 0x0f,0x01,
                          	 0xe2,0x00,
                          	 0xe3,0x64,
                          	 0xe4,0x02,
                          	 0xe5,0xbc,
                          	 0xe6,0x02,
                          	 0xe7,0xbc,
                          	 0xe8,0x02,
                          	 0xe9,0xbc,
                          	 0xea,0x03,
                          	 0xeb,0xe8,
                          */
#if 0
                          0x01,0x32,
                          0x02,0x70,
                          0x0f,0x01,
                          0xe2,0x00,
                          0xe3,0x64,
                          0xe4,0x02,
                          0xe5,0xbc,
                          0xe6,0x02,
                          0xe7,0xbc,
                          0xe8,0x02,
                          0xe9,0xbc,
                          0xea,0x03,
                          0xeb,0xe8,
#else //2010-08-22 kim 28M 25fps
                          0x01,0x32,
                          0x02,0x48,
                          0x0f,0x01,
                          0xe2,0x00,
#if CAMERA_WINDOW_MODE
                          0xe3, 0x8c / 4,
#else
                          0xe3,0x8c,
#endif
                          0xe4,0x02,
                          0xe5,0x30,
                          0xe6,0x02,
                          0xe7,0x30,
                          0xe8,0x02,
                          0xe9,0x30,
                          0xea,0x03,
                          0xeb,0xd4,
#endif

                          0xec, 0x20, //[5:4]exp_level  [3:0]minimum exposure high 4 bits
                          /*
                          0x03,0xc1,   //    26M
                          0x04,0x2c,*/
                          0x03,0x02,   //    28M
                          0x04,0xbc,



                          0x5a,0x56,//for AWB can adjust back
                          0x5b,0x40,
                          0x5c,0x4a,

                          0x23,0x00,
                          0x2d,0x0a,
                          0x20,0xff,
													
													// aec disable yrm 
                          0xd2,0x20, 
													0x51,0x33,  // 值越大，光晕范围越大，为0的话，没有画面了
													0x52,0x44,  // 值越大, 核心区域亮度越高，为0的话，就剩下一个圈了
													
                          0x73,0x00,
                          0x77,0x54,

                          0xb3,0x40,
                          0xb4,0x80,
                          0xba,0x00,
                          0xbb,0x00,
                          END_PARAMETER, END_PARAMETER,

};


int cmos_gc0308_init(unsigned char dev_addr)
{
    int i = 0;
    while (END_PARAMETER != RESET_TAB[i])
    {
        if (0 == write_camera_reg(dev_addr, RESET_TAB[i],RESET_TAB[i+1]))
        {
            return(0);
        }
        i+=2;
    }
    return(1);
}

