#ifndef __S5PC210FB_H__
#define __S5PC210FB_H__

#ifdef CONFIG_EmbedSky_LCD

#define TFT240320				1
#define TFT320240				2
#define TFT480272				3//T43(天嵌4.3寸屏)
#define VGA640480				4
#define TFT800480				5//A70(群创7.0寸屏)
#define TFT800600				6//A104(友达10.4寸屏)
#define VGA1024768				7//VGA1024x768
#define DYNAMIC_CONFIG			8//动态配置LCD

#define LCD_TFT					DYNAMIC_CONFIG//TFT480272


#if(LCD_TFT == TFT240320)
#define	CONFIG_LCD_VBPD				1
#define	CONFIG_LCD_VFPD				2
#define	CONFIG_LCD_VSPW				1
#define	CONFIG_LCD_HBPD				6
#define	CONFIG_LCD_HFPD				3
#define	CONFIG_LCD_HSPW				1
#define	CONFIG_LCD_CLKVAL			2
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				240
#define LCD_YSIZE_TFT				320

#elif(LCD_TFT == TFT320240)
#define	CONFIG_LCD_VBPD				12
#define	CONFIG_LCD_VFPD				4
#define	CONFIG_LCD_VSPW				5
#define	CONFIG_LCD_HBPD				21
#define	CONFIG_LCD_HFPD				32
#define	CONFIG_LCD_HSPW				44
#define	CONFIG_LCD_CLKVAL			6
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				240
#define LCD_YSIZE_TFT				320

#elif(LCD_TFT == TFT480272)
#define	CONFIG_LCD_VBPD				2
#define	CONFIG_LCD_VFPD				2
#define	CONFIG_LCD_VSPW				10
#define	CONFIG_LCD_HBPD				2	
#define	CONFIG_LCD_HFPD				2	
#define	CONFIG_LCD_HSPW				40
#define	CONFIG_LCD_CLKVAL			7 
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				480
#define LCD_YSIZE_TFT				272

#elif(LCD_TFT == VGA640480)
#define	CONFIG_LCD_VBPD				29
#define	CONFIG_LCD_VFPD				5
#define	CONFIG_LCD_VSPW				4
#define	CONFIG_LCD_HBPD				71
#define	CONFIG_LCD_HFPD				40
#define	CONFIG_LCD_HSPW				31
#define	CONFIG_LCD_CLKVAL			1
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				640
#define LCD_YSIZE_TFT				480

#elif(LCD_TFT == TFT800480)
#define	CONFIG_LCD_VBPD				4
#define	CONFIG_LCD_VFPD				2
#define	CONFIG_LCD_VSPW				2
#define	CONFIG_LCD_HBPD				27
#define	CONFIG_LCD_HFPD				14
#define	CONFIG_LCD_HSPW				10
#define	CONFIG_LCD_CLKVAL			3
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				800
#define LCD_YSIZE_TFT				480

#elif(LCD_TFT == TFT800600)
#define	CONFIG_LCD_VBPD				4
#define	CONFIG_LCD_VFPD				2
#define	CONFIG_LCD_VSPW				2
#define	CONFIG_LCD_HBPD				27
#define	CONFIG_LCD_HFPD				14
#define	CONFIG_LCD_HSPW				8
#define	CONFIG_LCD_CLKVAL			4
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				800
#define LCD_YSIZE_TFT				600

#elif(LCD_TFT == VGA1024768)
#define	CONFIG_LCD_VBPD				5
#define	CONFIG_LCD_VFPD				3
#define	CONFIG_LCD_VSPW				3
#define	CONFIG_LCD_HBPD				27 
#define	CONFIG_LCD_HFPD				14 
#define	CONFIG_LCD_HSPW				10 
#define	CONFIG_LCD_CLKVAL			4
#define CONFIG_LCD_BPP				24//16
#define LCD_XSIZE_TFT				1024
#define LCD_YSIZE_TFT				768

#elif(LCD_TFT == DYNAMIC_CONFIG)
typedef struct __LCD_CONFIG{
	u8 lcd_vbpd;
	u8 lcd_vfpd;
	u8 lcd_vspw;
	u8 lcd_hbpd;
	u8 lcd_hfpd;
	u8 lcd_hspw;
	u8 lcd_clkval;
	u8 lcd_bpp;
	u16 lcd_xsize;
	u16 lcd_ysize;
}lcd_config_info;
typedef enum _LCD_TYPE{
	X240Y320=0,
	X320Y240,
	X480Y272,
	X640Y480,
	X800Y480,
	X800Y600,
	X1024Y768,
	VGA7213,
	X1280Y720,
	X1920Y1080,
	LCD_TYPE_MAX,
}LCD_TYPE;
//#define LCD_TYPE_MAX 				8
extern lcd_config_info *curren_info;
extern const lcd_config_info tq_lcd_config_info[LCD_TYPE_MAX];
#define	CONFIG_LCD_VBPD				(curren_info->lcd_vbpd)
#define	CONFIG_LCD_VFPD				(curren_info->lcd_vfpd)
#define	CONFIG_LCD_VSPW				(curren_info->lcd_vspw)
#define	CONFIG_LCD_HBPD				(curren_info->lcd_hbpd) 
#define	CONFIG_LCD_HFPD				(curren_info->lcd_hfpd)
#define	CONFIG_LCD_HSPW				(curren_info->lcd_hspw)
#define	CONFIG_LCD_CLKVAL			(curren_info->lcd_clkval)
#define CONFIG_LCD_BPP				(24)//(curren_info->lcd_bpp)
#define LCD_XSIZE_TFT				(curren_info->lcd_xsize)
#define LCD_YSIZE_TFT				(curren_info->lcd_ysize)
#define CONFIG_LCD_VFPE				(curren_info->lcd_vfpe)
#define CONFIG_LCD_VBPE				(curren_info->lcd_vbpe)
#endif

#define NAND_CONTROL_ENABLE()		(NFCONT_REG |= (1 << 0))
#define NAND_DISABLE_CE()			(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE()			(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()				do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

#define SCR_XSIZE_TFT	 			(LCD_XSIZE_TFT)
#define SCR_YSIZE_TFT	 			(LCD_YSIZE_TFT)

#define HOZVAL_TFT					(LCD_XSIZE_TFT - 1 )
#define LINEVAL_TFT					(LCD_YSIZE_TFT - 1 )

#define LOGO_ZONE_OFFSET			0x100000
#define VIDEO_MEM_SIZE				0x500000					/*1366*768*4 = 0x400800Byte */

#define LCD_VIDEO_ADDR				0x3B000000
#define LOGO_BMP_ADDR				(LCD_VIDEO_ADDR + VIDEO_MEM_SIZE)
#if(CONFIG_LCD_BPP == 16)
#define RGB(r,g,b)  				(u16)(((r)&0x1f)<<11 | ((g)&0x3f)<<5 | ((b)&0x1f)<<0)
#elif(CONFIG_LCD_BPP == 24)
#define RGB(r,g,b)					(u32)((r)<<16 | (g)<<8 | (b)<<0)
#endif


#define LCD_WIDE_BAR				LCD_YSIZE_TFT/8   //定义进度条框架的宽度
	
#define LCD_BAR_WIDTH				(LCD_WIDE_BAR-16)*2/5   //定义进度条的宽度

#define LCD_BAR_NOTE				(LCD_WIDE_BAR-16)*3/10   //定义进度条和进度条注释之间的间距

#define LCD_BAR_START_X				LCD_XSIZE_TFT/10   //定义第一个进度框架X坐标
#define LCD_BAR_START_Y				LCD_YSIZE_TFT/2     //定义第一个进度框架Y坐标

#define LCD_BAR_LONG				LCD_XSIZE_TFT*8/10  //定义进度条长度

#define LCD_BAR_COLOUR				RGB(0xff, 0xff, 0x0)   //定义进度条颜色

#define LCD_BAR_ANNOTATE_COLOUR		RGB(0xcd, 0xcd, 0xb4)	//定义进度条注释颜色

#define LCD_BAR_SHADING				RGB(0xc0, 0xc0, 0xc0)	//定义进度条底纹颜色	


#define LCD_VIDEO_BACKGROUND_ALPHA         	(0xa)

void tq210_lcd_init(void);
void embedsky_user_logo(void);
void embedsky_tq_logo(void);
void Lcd_printf(unsigned int x,unsigned int y,unsigned int c,unsigned int bk_c,unsigned int st,char *fmt,...);
void embedsky_paint_Dot(int x,int y,u32 bmp);
void embedsky_paint_Progress_bar(u32 x,u32 y,u32 width,u32 length,u32 current,u32 colour);
void embedsky_Write_Progress_note(u32 x,u32 y,u32 length,u8 left, u32 colour,char *str);
void embedsky_paint_table(u32 x,u32 y,u32 width,u32 hight,u32 colour,char *info);



#endif /* CONFIG_EmbedSky_LCD */
#endif

