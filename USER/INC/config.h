
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif	

#define KEIL_DEBUG
#ifdef KEIL_DEBUG
	  #define  CONFIG_DEBUG   (1)
	  #define  CONFIG_BOOT    (0)
#else
    #define  CONFIG_DEBUG   (0)
	#define  CONFIG_BOOT    (1)
#endif


#define  CONFIG_RELEASE (!CONFIG_DEBUG)
#define  CONFIG_PERFORM (0)
#define  CONFIG_DATAFLASH_FAST  (1)	
#define  CONFIG_TEST_MBUS   (1)
#define  CONFIG_SEVEN_DIGITS (0)
#define  CONFIG_LINYANG_CONVERTET  (0)  //林洋转换器最大字节间延时为1s，所以需要在识别过程中发送一些报文保证字节间不会超时



#ifdef KEIL 
	#define FORCE_INLINE  //__forceinline
#else 
	#define FORCE_INLINE 
#endif 


#ifdef KEIL
	#define ASM_SPEEDUP        1
#else
	#define ASM_SPEEDUP           0
#endif



#define CMAERA_MAX_WIDTH   (640/2)
#define CMAERA_MAX_HEGITH  (480/2)

#define STEP_UINITS    (25)

#define CAMERA_WINDOW_MODE     (1)

#define WIN_DEFAULT_Y0           (0)
#define WIN_DEFAULT_X0           (0)
#define WIN_DEFAULT_HEIGHT       (240)
#define WIN_DEFAULT_CAPUTE_WIDTH (320)

#define ONLY_MONO_CHANEL     (0)
#define CAPTURE_MAIN_CLK_108MHZ   (1)


#if ONLY_MONO_CHANEL
  #define DMA_BUF_SZ    (CMAERA_MAX_WIDTH/2)
#else
  #define DMA_BUF_SZ     (CMAERA_MAX_WIDTH)
#endif 


#define CAMERA_MAX_LINE_BUFF   (CMAERA_MAX_WIDTH)

#define MAX_DIGIT_CNT     8
#define MAX_POINTER_COUNT 6 
#define POINTER_WIDTH   0X003c //默认指针宽度  60
#ifdef ESSN_WCR_P
	#define DIGIT_HEIGHT    48
	#define DIGIT_WIDTH     48

#else 
	#define DIGIT_HEIGHT    60
	#define DIGIT_WIDTH     32
#endif

#define DIGIT_TOP       (WIN_DEFAULT_Y0) 



// 0x4000 为256K   0x4000 16K  最小的可独立擦除的扇区为0x1000 4k
#define   SIZE_1K    (0x400)
#define   SIZE_1M    (0x100000)


/*Flash的使用分布概述:
  0-1M作为正常使用的数据区
  1M-2M 升级备份程序存储区

  0-1M将固定大小的参数安排在前面，大小变化的参数安排的后面，这样电脑上生成的bin文件最小。
  embedPara.bin 里面包含 tanh table , exp table ,cnn1 paras, cnn2 paras。
*/
#define  TANH_ADDR            (0)
#define  TANH_SIZE            (16*SIZE_1K)
#define  EXP_MAP_ADDR         (TANH_ADDR + TANH_SIZE)
#define  EXP_MAP_SIZE         (32*SIZE_1K)
#define  CNN_NET_ADDR         (EXP_MAP_ADDR+EXP_MAP_SIZE)
#define  CNN_VERSION_ADDR     (CNN_NET_ADDR)
#define  CNN1_SIZE            (320*SIZE_1K)
#define  CNN2_NET_ADDR        (CNN1_SIZE+CNN_NET_ADDR)
#define  CNN2_SIZE            (500*SIZE_1K)
#define  EMBED_PARA_ADDR      (TANH_ADDR)
#define  EMBED_PARA_SIZE      (TANH_SIZE+EXP_MAP_SIZE+CNN1_SIZE+CNN2_SIZE)

#define  DEV_INFO_ADDR        (CNN2_NET_ADDR+CNN2_SIZE)
#define  DEV_INFO_SIZE        (4*SIZE_1K)
#define  DEV_INFO_BACKUP_ADDR (DEV_INFO_ADDR+DEV_INFO_SIZE)
#define  DEV_INFO_BACKUP_SIZE (DEV_INFO_SIZE)       


/*捕捉图像暂存区域*/
#define  RECGONIZE_IMAGE_ADDR (DEV_INFO_BACKUP_ADDR+DEV_INFO_BACKUP_SIZE)            
#define  RECGONIZE_IMAGE_SIZE (SIZE_1K*16) 
#define  JPEG_IMG_ADDR        (RECGONIZE_IMAGE_ADDR+RECGONIZE_IMAGE_SIZE)
#define  JPEG_IMG_SIZE        (20*SIZE_1K)
#define  TEST_IMG_ADDR        (JPEG_IMG_ADDR+JPEG_IMG_SIZE)//912K 0xe4000
#define  TEST_IMG_SIZE        (32*SIZE_1K)


#define BACK_UP_ADDR         (SIZE_1M)
#define PROGRAM_END_ADDRESS      0x1FFFF0  //0x77 is ok
#define SPI_PROGRAM_ADDRESS      0x100000
#define PACKAGE_STATUS_ADDRESS   (0x100000 +800*1024)

 
#define MATH_IN_FLASH   1


#define COPY_PARAMS_TO_ROM   1

#if COPY_PARAMS_TO_ROM
#define CNN_RSV_SZ              (0x400*6) //6K
#define CNN2_INNER_FLASH_POS          (0x10000 - CNN_RSV_SZ) //
#define CNN1_INNER_FLASH_POS          (0x08000000+CNN2_INNER_FLASH_POS - CNN_RSV_SZ)
#endif


#define CONFIG_LOW_POWER_470   0

#ifdef __cplusplus
}
#endif

#endif
