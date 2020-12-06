#include <common.h>
#include <linux/ctype.h>
#include <linux/string.h>

#ifdef CONFIG_EMBEDSKY_FAT

#include <part.h>
#include <fat.h>
#include <def.h>
#include <embedsky_FAT.h>
#include <malloc.h>
#include <mmc.h>
#include <s5pv210.h>

#ifdef CONFIG_EmbedSky_LCD
#include <lcd/s5pv210_fb.h>
#endif /* CONFIG_EmbedSky_LCD */

#define INI_FILE_SIZE				2048
#define EMBEDSKY_FAT_ADDR			0xc6008000					//FAT临时数据存放的基础地址
#define INI_FILE_BUFFER				(EMBEDSKY_FAT_ADDR)			//INI文件内容存放起始地址
#define INI_DIR_BUFFER				(INI_FILE_BUFFER + 0x800)	//需要烧写的文件的路径存放的起始地址
#define FAT_BUFFER1					(INI_DIR_BUFFER + 0x200)	//临时数据1存放的起始地址
#define FAT_BUFFER2					(FAT_BUFFER1 + 0x200)		//临时数据2存放的起始地址
#define FAT_BUFFER3					(FAT_BUFFER2 + 0x400)		//临时数据3存放的起始地址

#define SD_DOWN_ADDR				0x28000000
#define FAT_DEBUG					0

#if 0
#define Printf_FunctionName	printf("%s\n",__FUNCTION__)
#else
#define Printf_FunctionName
#endif

U16 install_flag;	//安装标志位，每一个位对应要安装的镜像的类型，目前只之用了11位，为1时表示要安装，为0时不安装。
			//从0位开始计算：	bootloader,logodir,LinuxZimage,LinuxRootfs,
			//		Stepldr,eboot,NK,AndroidzImage,AndroidRootfs,uCOSImage,ApplicationImage
U8 led_debug;		//用于LED debug灯的亮灭，当为1时，灯要闪烁；为0时，灯不闪烁。

int sd_nand_type; 				// is_nandtype 			if = 1 IS_MICRON_NAND
										//									if = 0 IS_PAGE2048_NAND
int mmc_device_num;

#ifdef CONFIG_EMBEDSKY_INI
#define MAX_LINE		256		//行最大200字节
#define MAX_SECTION		20		//块名称最长50字节
#define MAX_NAME		50		//文件名称最长50字节

struct System_ini
{
	U8 operating;
	char ostype[9];
	U8 led;
	U8 beep;
	char imagetype[20];
}system_ini;

//烧录bootloader
static int download_bootloader(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download bootloader image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
#endif  /* CONFIG_EmbedSky_LCD */
	filesize = file_fat_read (filedir, (void *)SD_DOWN_ADDR, 0);
	if (filesize != 0)
	{
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(1);
		}
#endif /* CONFIG_EmbedSky_BEEP */
		if(filesize > 0x80000)
		{
			sprintf(cmd_buf, "Seboot 0x%x", SD_DOWN_ADDR);
		}
		else
		{
			sprintf(cmd_buf, "nand erase bios; nand write  0x%x bios 0x%lx", SD_DOWN_ADDR, filesize);
		}
		run_command(cmd_buf, 3);
#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*0+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
		embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            Succeeded!");
#endif /* CONFIG_EmbedSky_LCD */
		return 1;
	}
	else
	{
#ifdef CONFIG_EmbedSky_LCD
		embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            Failed!");
		embedsky_paint_table(LCD_XSIZE_TFT-26*8-4,4,0,16*6,0xff,"loader bootloader failed.");
#ifdef CONFIG_TQ_ERROR
		run_command("error", 0);
#endif  /*CONFIG_TQ_ERROR*/
#endif /* CONFIG_EmbedSky_LCD */

		printf("file length is 0 or file does not exit!\n");
		return 1;
	}
}

//烧录logo
static int download_logo(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download logo image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
#endif /* CONFIG_EmbedSky_LCD */

	filesize = file_fat_read (filedir, (void *)SD_DOWN_ADDR, 0);
	if (filesize != 0)
	{
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(1);
		}
#endif /* CONFIG_EmbedSky_BEEP */

		sprintf(cmd_buf, "nand erase logo; nand write.jffs2 0x%x logo 0x%lx", SD_DOWN_ADDR, filesize);
		run_command(cmd_buf, 1); //
#if 0
#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*1+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
		embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            Succeeded!");
#endif /* CONFIG_EmbedSky_LCD */
#endif
		return 1;
	}
	else
	{
#ifdef CONFIG_EmbedSky_LCD
		embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            Failed!");
		embedsky_paint_table(LCD_XSIZE_TFT-26*8-4,4,0,16*6,0xff,"loader logo failed.");
#ifdef CONFIG_TQ_ERROR
		run_command("error", 0);
#endif  /*CONFIG_TQ_ERROR*/
#endif /* CONFIG_EmbedSky_LCD */

#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(4);
		}
#endif /* CONFIG_EmbedSky_BEEP */
		printf("file length is 0 or file does not exit!\n");
		return 1;
	}
}

//烧录wince的NK.bin
static int download_nk(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*2,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download WinCE NK image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*2,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
#endif /* CONFIG_EmbedSky_LCD */
	filesize = file_fat_read (filedir, (void *)SD_DOWN_ADDR, 0);
	if (filesize != 0)
	{
		sprintf(cmd_buf, "relocateNK 0x%08x 0x%x; savewince", SD_DOWN_ADDR, (unsigned int)filesize);
		run_command(cmd_buf, 0);
#if 0	//在WinCE烧写的地方已经有了，所以屏蔽掉。
#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*2+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
		embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*2,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                                                  Succeeded!");
#endif /* CONFIG_EmbedSky_LCD */
#endif
		return 3;
	}
	else
	{
		printf("file length is 0 or file does not exit!\n");
		return 1;
	}
}

//烧录Linux/Android/ubuntu的kernel镜像
static int download_kernel(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
	return 1;
}

//烧录Linux/Android/ubuntu的dtb镜像
static int download_dtb(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
	return 1;
}

//烧录Linux/Android/ubuntu的rootfs镜像
static int download_rootfs(const char * filedir)
{
	char cmd_buf[200];//命令行
	long filesize;
	unsigned int retLine;
#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*3,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download filesystem.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*3,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            initializing...");
	//embedsky_paint_Progress_bar(0,0,0,0,0,0xff);
#endif /* CONFIG_EmbedSky_LCD */
	//UBIFS images name is: *.ubi
	retLine = strlen(filedir);
	if((filedir[retLine-1] == 'i') && \
		(filedir[retLine-1-1] == 'b') && \
		(filedir[retLine-1-2] == 'u'))
	{
		sprintf(cmd_buf, "nand erase root;ubi part root;ubi create rootfs;ubi OEMwrite 0x%x rootfs %s", SD_DOWN_ADDR, filedir);
		//printf("%s\n", cmd_buf);
		run_command(cmd_buf, 0);

		sprintf(cmd_buf, "setenv bootargs noinitrd ubi.mtd=3 root=ubi0:rootfs rootfstype=ubifs init=/init console=ttySAC%d;saveenv;nand read.jffs2 0xc0008000 kernel;bootm 0xc0008000", SERIAL_PORT_NUM);
		run_command(cmd_buf, 0);
		return 1;
	}
	//Yaffs2 images name is: *.yaf or *.bin
	else if(((filedir[retLine-1] == 'n') && \
			(filedir[retLine-1-1] == 'i') && \
			(filedir[retLine-1-2] == 'b')) ||
			((filedir[retLine-1] == 'f') && \
			(filedir[retLine-1-1] == 'a') && \
			(filedir[retLine-1-2] == 'y')))//
	{
		run_command("nand erase root", 0);//Barack liu modify for 20120720
		//sprintf(cmd_buf, "nand write.OEMyaffs 0x%x root %s", SD_DOWN_ADDR, filedir);
		sprintf(cmd_buf, "nand write.i 0x%x root %s", SD_DOWN_ADDR, filedir);
		//printf("%s\n", cmd_buf);
		run_command(cmd_buf, 0);

		sprintf(cmd_buf, "setenv bootargs noinitrd root=/dev/mtdblock3 rootfstype=yaffs2 init=/init console=ttySAC%d;saveenv", SERIAL_PORT_NUM);
		run_command(cmd_buf, 0);
		return 1;
	}
		return 1;
}

//处理ini文件中的系统配置参数
static int ini_system_handler(char *name, char *value, const char *user, char *section)
{
	if(!strcmp(name, "operating"))
	{
		if(!strcmp(value, "install"))
			system_ini.operating = 1;
	}
	else if(!strcmp(name, "os type"))
	{
		strcpy(system_ini.ostype, value);
	}
	else if(!strcmp(name, "led"))
	{
		if(!strcmp(value, "on"))
			system_ini.led = 1;
	}
	else if(!strcmp(name, "beep"))
	{
		if(!strcmp(value, "on"))
			system_ini.beep = 1;
		//把user设置为新的块，程序将会直接跳转到新的块上执行。
		strcpy((char *)user, system_ini.ostype);
//		strcpy(section, system_ini.ostype);
	}
		return 1;
}

//处理ini文件中的Linux镜像烧录
static int ini_linux_handler(char *name, char *value)
{
	if(!strcmp(name, "bootloader"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_bootloader(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			/*end for beep*/
			printf("Don't need to download bootloader\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "logo"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_logo(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("Don't need to download logo image.\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "kernel"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_kernel(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("Don't need to download Linux zImage\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "dtb"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_dtb(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("Don't need to download Linux dtb\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "rootfs"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_rootfs(value);
			printf("  root size is %s !!!! \n",value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("Don't need to download Linux rootfs\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
		return 1;
}

//处理ini文件中的WinCE镜像烧录
static int ini_wince_handler(char *name, char *value)
{
	if(!strcmp(name, "bootloader"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_bootloader(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			/*end for beep*/
			printf("Don't need to download bootloader\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "logo"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_logo(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("Don't need to download logo image.\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
	else if(!strcmp(name, "nk"))
	{
		if(value[0] != '\0')
		{
			strcpy(system_ini.imagetype, name);
			download_nk(value);
		}
		else
		{
#ifdef CONFIG_EmbedSky_BEEP
			if(system_ini.beep)
			{
				beep_long(10);
			}
#endif /* CONFIG_EmbedSky_BEEP */
			printf("don't download WinCE NK.bin\n");
		}
#ifdef CONFIG_EmbedSky_BEEP
		if(system_ini.beep)
		{
			beep_long(0);
		}
#endif /* CONFIG_EmbedSky_BEEP */
	}
		return 1;
}

//处理ini文件中的Android镜像烧录
static int ini_android_handler(char *name, char *value)
{
		return 1;
}

//处理ini文件中的ubuntu镜像烧录
static int ini_ubuntu_handler(char *name, char *value)
{
		return 1;
}

//处理ini文件中的uCOS-II镜像烧录
static int ini_ucos_ii_handler(char *name, char *value)
{
		return 1;
}

//处理ini文件中的NoOS镜像烧录
static int ini_noos_handler(char *name, char *value)
{
		return 1;
}

//处理ini文件中的MAC镜像烧录
static int ini_mac_handler(char *name, char *value)
{
		return 1;
}

//处理ini文件中的License镜像烧录
static int ini_license_handler(char *name, char *value)
{
		return 1;
}

/* Strip whitespace chars off end of given string, in place. Return s. */
static char *rstrip(char *s)
{
	char *p = s + strlen(s);

	while (p > s && isspace(*--p))
		*p = '\0';
	return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char *lskip(const char *s)
{
	while (*s && isspace(*s))
		s++;
	return (char *)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char *find_char_or_comment(const char *s, char c)
{
	int was_whitespace = 0;

	while (*s && *s != c && !(was_whitespace && *s == ';'))
	{
		was_whitespace = isspace(*s);
		s++;
	}
	return (char *)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char *strncpy0(char *dest, const char *src, size_t size)
{
	strncpy(dest, src, size);
	dest[size - 1] = '\0';
	return dest;
}

/* Emulate the behavior of fgets but on memory */
static char *memgets(char *str, int num, char **mem, size_t *memsize)
{
	char *end;
	int len;
	int newline = 1;

	end = memchr(*mem, '\n', *memsize);
	if (end == NULL)
	{
		if (*memsize == 0)
			return NULL;
		end = *mem + *memsize;
		newline = 0;
	}
	len = min((end - *mem) + newline, num);
	memcpy(str, *mem, len);
	if (len < num)
		str[len] = '\0';

	/* prepare the mem vars for the next call */
	*memsize -= (end - *mem) + newline;
	*mem += (end - *mem) + newline;

	return str;
}

/* 
	ini处理函数，根据实际情况添加对应的功能。
 */
static int ini_handler(const char *user, char *section, char *name, char *value)
{
	if(!strcmp(user, "system"))
	{
		ini_system_handler(name, value, user, section);
	}
	else if(!strcmp(user, "linux"))
	{
		ini_linux_handler(name, value);
	}
	else if(!strcmp(user, "wince"))
	{
		ini_wince_handler(name, value);
	}
	else if(!strcmp(user, "android"))
	{
		ini_android_handler(name, value);
	}
	else if(!strcmp(user, "ubuntu"))
	{
		ini_ubuntu_handler(name, value);
	}
	else if(!strcmp(user, "ucos-ii"))
	{
		ini_ucos_ii_handler(name, value);
	}
	else if(!strcmp(user, "noos"))
	{
		ini_noos_handler(name, value);
	}
	else if(!strcmp(user, "mac"))
	{
		ini_mac_handler(name, value);
	}
	else if(!strcmp(user, "License"))
	{
		ini_license_handler(name, value);
	}

	/* success */
	return 1;
}

/* 
 处理第一行和后面可能出现的注释，建议不要加注释
 第一行固定内容：#### This is EmbedSky SD config file ####
 注释采用;符号
 块开头为[块名]，块与块之间加个空行
 
*/
#define TMP_ADDR 	(0xc0008000)
#define TMP_ROOTADDR 	(0x40008000)
int my_fatload(char *pwd,int is_rootfs)
{
	int filesize = 0;
	unsigned long offset = 0;
	if(is_rootfs)
		offset = TMP_ROOTADDR;
	else
		offset = TMP_ADDR;
	char buf [12];
	filesize = file_fat_read (pwd, (unsigned char *) offset, 0);
	if(filesize <=0)
		return -1;
	printf("%lX", filesize);
	sprintf(buf, "%lX", filesize);
	setenv("filesize", buf);
	return 0;
}


int down_bootloader(char * pwd)
{
	char cmd_buf[200];
	char ret;
	#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download bootloader image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
	#endif  /* CONFIG_EmbedSky_LCD */
	ret = my_fatload(pwd,0);
	if(ret){
		printf("fatlod fail!!!!!!\n");
		return -1;
	}
	strcpy(cmd_buf, "nand erase bios; nand write 0xc0008000 bios $(filesize)");
	run_command(cmd_buf, 0);
	#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*0+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
	#endif /* CONFIG_EmbedSky_LCD */
	return 0;
}
int down_logo(char * pwd)
{
	char cmd_buf[200];
	char ret;
	#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download logo image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
	#endif /* CONFIG_EmbedSky_LCD */
	ret = my_fatload(pwd,0);
	if(ret){
		printf("fatlod fail!!!!!!\n");
		return -1;
	}
	strcpy(cmd_buf, "nand erase logo; nand write.i 0xc0008000 logo $(filesize)");
	run_command(cmd_buf, 0);
	#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*1+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
	#endif /* CONFIG_EmbedSky_LCD */
	return 0;
}
int down_kernel(char * pwd)
{
	char cmd_buf[200];
	char ret;
	#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*2,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download kernle image.");
	#endif /* CONFIG_EmbedSky_LCD */
	
	ret = my_fatload(pwd,0);
	if(ret){
		printf("fatlod fail!!!!!!\n");
		return -1;
	}
	strcpy(cmd_buf, "nand erase kernel; nand write.i 0xc0008000 kernel $(filesize)");
	run_command(cmd_buf, 0);
	#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*2+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
	#endif /* CONFIG_EmbedSky_LCD */
	return 0;
}
int is_ubi(char *pwd)
{
	int num = strlen(pwd);
	//printf("Is num =[ %c ]!!!\n",pwd[num-3]);
	if(pwd[num-3]=='u')
		return 1;
	else
		return 0;
}
int down_rootfs(char * pwd)
{
	char cmd_buf[200];
	char ret;
	#ifdef CONFIG_EmbedSky_LCD
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*3,LCD_BAR_LONG,1,LCD_BAR_ANNOTATE_COLOUR,"Download rootfs image.");
	embedsky_Write_Progress_note(LCD_BAR_START_X,LCD_BAR_START_Y+LCD_WIDE_BAR*3,LCD_BAR_LONG,0,LCD_BAR_ANNOTATE_COLOUR,"                            loading...");
	#endif /* CONFIG_EmbedSky_LCD */
	ret = my_fatload(pwd,1);
	if(ret){
		printf("fatlod fail!!!!!!\n");
		return -1;
	}
	if(is_ubi(pwd))
		strcpy(cmd_buf, "nand erase root;ubi part root;ubi create rootfs;ubi write 0x40008000 rootfs $(filesize)");
	else{
		if(sd_nand_type)
			strcpy(cmd_buf, "nand erase root; nand write.i 0x40008000 root $(filesize)");
		else
			strcpy(cmd_buf, "nand erase root; nand write.yaffs 0x40008000 root $(filesize)");
	}
	#ifdef CONFIG_EmbedSky_LCD
		LCD_DrawCircle_Full(LCD_BAR_START_X+LCD_BAR_LONG, LCD_BAR_START_Y+LCD_WIDE_BAR*3+16+LCD_BAR_NOTE+LCD_BAR_WIDTH/2, LCD_BAR_WIDTH/2, LCD_BAR_COLOUR, 1);
	#endif /* CONFIG_EmbedSky_LCD */
	run_command(cmd_buf, 0);
	return 0;
}

static int ini_parse(char *filestart, size_t filelen,
	int (*handler)(const char *, char *, char *, char *),	const char *user)
{
	/* Uses a fair bit of stack (use heap instead if you need to) */
	char line[MAX_LINE];
	char section[MAX_SECTION] = "";
	char prev_name[MAX_NAME] = "";

	char *curmem = filestart;
	char *start;
	char *end;
	char *name;
	char *value;
	size_t memleft = filelen;
	int lineno = 0;
	int error = 0;
	int i;
	char tmp;
	char down_flag = 0;		
	/* Scan through file line by line */
	while (memgets(line, sizeof(line), &curmem, &memleft) != NULL) 
	{
		lineno++;
		start = lskip(rstrip(line));
		/*decide system type*/
		if(!strncmp(start, "OS Type=", sizeof("OS Type=")-1)){
			 printf("down system is  %s!!!\n",start+sizeof("OS Type=")-1);
			 tmp = *(start+sizeof("OS Type=")-1);
		}
		/*down bootloader*/
		if(!strncmp(start, "bootloader=", sizeof("bootloader=")-1)){
			down_bootloader(start+sizeof("bootloader=")-1);
		}
		/*down logo*/
		else if(!strncmp(start, "LOGO=", sizeof("LOGO=")-1)){
			down_logo(start+sizeof("LOGO=")-1);
		}
		/*down kernel and rootfs*/
		else if(down_flag){
			if(!strncmp(start, "kernel=", sizeof("kernel=")-1)){
				down_kernel(start+sizeof("kernel=")-1);
			}
			if(!strncmp(start, "rootfs=", sizeof("rootfs=")-1)){
				down_rootfs(start+sizeof("rootfs=")-1);
			}
		}
		/*open down_flag and close down_flag*/
		if(tmp == 'L'|| tmp == 'l'){
			if(!strcmp(start,"#### Linux ####")){
				down_flag = 1;
			}
			else if(!strcmp(start,"#### Linux end ####")){
				down_flag = 0;
			}
		}
		else if(tmp == 'A'|| tmp == 'a'){
			if(!strcmp(start,"#### Android ####")){
				down_flag = 1;
			}
			else if(!strcmp(start,"#### Android end ####")){
				down_flag = 0;
			}
		}
		
		
/*	if (*start == '[') 
		{
			// A "[section]" line 
			end = find_char_or_comment(start + 1, ']');
			if (*end == ']') 
			{
				*end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				for (i = 0; i < strlen(section); i++)
					section[i] = tolower(section[i]);
				*prev_name = '\0';
			}
			else if (!error) 
			{
				// No ']' found on section line 
				error = lineno;
			}
			printf("-user=%s,section=%s\n", user, section);
		}
		else if ((strcmp(user, section) == 0) && (*start && *start != ';'))
		{
			// Not a comment, must be a name[=:]value pair 
			end = find_char_or_comment(start, '=');
			if (*end != '=')
				end = find_char_or_comment(start, ':');
			if (*end == '=' || *end == ':')
			{
				*end = '\0';
				name = rstrip(start);
				value = lskip(end + 1);
				end = find_char_or_comment(value, '\0');
				if (*end == ';')
					*end = '\0';
				rstrip(value);
				// Strip double-quotes 
				if (value[0] == '"' &&
				    value[strlen(value)-1] == '"')
				{
					value[strlen(value)-1] = '\0';
					value += 1;
				}
				 //Valid name[=:]value pair found, call handler	 
				strncpy0(prev_name, name, sizeof(prev_name));
				for (i = 0; i < strlen(name); i++)
					name[i] = tolower(name[i]);
				for (i = 0; i < strlen(value); i++)
					value[i] = tolower(value[i]);
				if (!handler(user, section, name, value) && !error)
					error = lineno;
			}
			else if (!error)
				// No '=' or ':' found on name[=:]value line 
				error = lineno;
		}*/
	}

	return error;
}


#endif /* CONFIG_EMBEDSKY_INI */
/*
 * 函数功能:从src偏移n个长度开始复len长度的字符串到dest
 * 返回值  :如果返回1则成功，返回-1;则失败
 */
int memcpyn(char *dest,char *src,int n,int len)
{
	DWORD i;
	if(src == NULL || dest == NULL)
	{
		printf("\n memcpyn error!!\n");
		return -1;
	}
	else
	{
		for(i=0;i<len;i++)
		{
			dest[i] = src[n+i];
		}
		return 1;
	}
}

void Lstrupr(char *buf)
{
	int i;
	for(i=0;i<strlen(buf);i++)
	{
		if(buf[i]<0x7b && buf[i]>0x60)
		{
			buf[i] -= 0x20;
		}
	}
}

#define JIAMI

#ifdef JIAMI
//加密用
char src_buf[] = { '#', ' ', '.', 'T', 'S', ' ', 'Y', 'n', 'f', '\\',//9
		   '/', 'i', 'K', ' ', 'Q', '1', '*', '0', 'm', 's', //19
		   'l', '2', '0', '4', 'D', '6', ',', 'E', 'D', '\0',//29
		   'k', 'e', 'o', '#', 'g', ' ', '8', 'h', 'X', 'b', //39
		   'r', 'c', 'a', 'n', 'd', 'j', 'y', 'Z', 'u', 'w',};//49
unsigned char src_cnt[] = {
			/* #### Thi */		0,  0,  33, 0,  1,  3,  37, 11,\
		 	/* s is Emb */		19, 1,  11, 19, 5,  27, 18, 39,\
			/* edSky SD */		31, 44,  4, 30, 46,  1,  4, 24,\
			/*  config  */		13, 41, 32, 43,  8, 11, 34,  1,\
			/* file ####\0 */	 8, 11, 20, 31,  1,  0,  0,  0, 33, 29};

#endif /* JIAMI */

#ifdef CONFIG_EmbedSky_BEEP
static void beep_long(unsigned char times)
{
	char buf[20];
	memset(buf,0,sizeof(buf));
	sprintf(buf, "beep on %x",times);
	run_command(buf, 0);
}
#endif /* CONFIG_EmbedSky_BEEP */

#ifdef CONFIG_OEM_SDREAD
//分段读取fat的文件
DWORD FAT_OEMReadFile(DWORD Dir, char *destbuffer, DWORD filesize, unsigned char tran)
{
#if 1
	return Dir;
#else
	DWORD sector;
	u32 i = 0;
#if FAT_DEBUG
	int j;
#endif /* FAT_DEBUG */
	DWORD ReadSize=0;
	Printf_FunctionName;

	if( filesize == 0)
	{
		printf("Without this file\n");
		return 0;
	}
	else
	{
//		printf("sizeof(DWORD)=%d\n",sizeof(DWORD));
//		printf("file length is 0x%lx or %ld\n", filesize, filesize);
	}

	do
	{
		sector = (DWORD)FirstDirSector + ((DWORD)(Dir-2)*(DWORD)SectorsPerClust);//calculate the actual sector number
#if FAT_DEBUG
		printf("dir=0x%02x,%02x\n",Dir, sector);
#endif /* FAT_DEBUG */
		if(FAT_ReadCluster(sector, (destbuffer + i), 8) == 0)
		{
#if FAT_DEBUG
			printf("FAT_ReadCluster erro1\n");
#endif /* FAT_DEBUG */
			return 0;
		}
		else 
		{
			i += 4096;
			if((ReadSize+4096) > filesize)
			{
				ReadSize += (filesize - ReadSize);
				destbuffer[ReadSize] = '\0';
#if FAT_DEBUG
				printf("FileSize=%ld, Dir=0x%02x\n", ReadSize, Dir);
#endif /* FAT_DEBUG */
				return Dir;
			}
			ReadSize += 4096;
#if FAT_DEBUG
			for(j=0; j<4096; j++)
				printf("%c", destbuffer[j+i]);
#endif /* FAT_DEBUG */
		}
#if FAT_DEBUG
		printf("Dir=FAT_NextCluster\n");
#endif /* FAT_DEBUG */
		Dir=FAT_NextCluster(Dir);
	}
	while((Dir!=0x0ffffff8) && (Dir != 0x0fffffff) && (Dir != 0xffffffff));
	return Dir;
#endif
}
#endif /* CONFIG_OEM_SDREAD */

//初始化UBoot结构函数
//返回1为初始化成功
char UbootStructInit(void)
{
	U32 ret;
	char section[MAX_SECTION];
	char FileBuf[INI_FILE_SIZE];
	char dest[64];
	char *srcbuf = "#### This is TQ210 SD config files ####";
#if FAT_DEBUG
	int j;
#endif /* FAT_DEBUG */

#if CONFIG_EMBEDSKY_INI
	memset(FileBuf, 0, sizeof(FileBuf));
	ret = file_fat_read ("images\\EmbedSky.ini", FileBuf, 0);
	if (ret != 0)
	{
		printf("EmbedSky.ini is %d byte!!\n",ret); //carlos
		//判断ini文件的标题是不是天嵌的标志
		
		memcpyn(dest, FileBuf, 0, sizeof(srcbuf-2));
		dest[sizeof(srcbuf-1)] = '\0';
		
		if(strncmp(dest, srcbuf, sizeof(srcbuf)-1)!=0)
		{
			printf("Embedsky.ini Heading :%s----\n",dest);
			printf("Embedsky.ini Heading :%s----\n",srcbuf);
			return 0;//配置文件内容不对
		}

		//默认设置是不烧写镜像，不启动led和蜂鸣器提醒
		system_ini.operating = 0;
		system_ini.led = 0;
		system_ini.beep = 0;

		//调用ini处理函数从system块开始处理。后续的块名就是从system块的OS处获取。
		strcpy(section, "system");
		ini_parse(FileBuf+sizeof(srcbuf), ret, ini_handler, section);

	}
	else
	{
		printf("return -1\n");
		return -1;
	}
#endif /* CONFIG_EMBEDSKY_INI */
	return 1;
}

// is_nandtype 			if = 1 IS_MICRON_NAND
//									if = 0 IS_PAGE2048_NAND
//carlos add
char SD_down_main(int is_nandtype)
{
	U32 ret;
	char section[MAX_SECTION];
	char FileBuf[INI_FILE_SIZE];
	char dest[64];
	sd_nand_type = is_nandtype;
	char *srcbuf = "#### This is TQ210 SD config files ####";
#if FAT_DEBUG
	int j;
#endif /* FAT_DEBUG */

#if CONFIG_EMBEDSKY_INI
	memset(FileBuf, 0, sizeof(FileBuf));
	ret = file_fat_read ("images\\EmbedSky.ini", FileBuf, 0);
	if (ret != 0){
		printf("EmbedSky.ini is %d byte!!\n",ret); //carlos
		
		memcpyn(dest, FileBuf, 0, sizeof(srcbuf-2));
		dest[sizeof(srcbuf-1)] = '\0';
		//Check the contents of the file
		if(strncmp(dest, srcbuf, sizeof(srcbuf)-1)!=0){
			printf("Embedsky.ini Heading :%s----\n",dest);
			printf("Embedsky.ini Heading :%s----\n",srcbuf);
			return 0;
		}
		ini_parse(FileBuf+sizeof(srcbuf), ret, ini_handler, section);
	}
	else{
		printf("return -1\n");
		return -1;
	}
#endif /* CONFIG_EMBEDSKY_INI */
	return 1;
}

unsigned char FAT_Init(void)//Initialize of FAT  need initialize SD first
{
	block_dev_desc_t *dev_desc=NULL;
	int dev=0;
	int part=1;
	struct mmc *mmc;

	mmc = find_mmc_device(dev);
	//printf("mmc num is %s !!!\n",mmc->name);//carlos
	if (mmc_init(mmc)) {
		printf("MMC init is failed.\n");
		return 1;
	}

	dev_desc=get_dev("mmc", dev);
	if (dev_desc==NULL) {
		puts ("\n** Invalid boot device **\n");
		return 1;
	}
	if (fat_register_device(dev_desc, part) != 0) {
		printf ("\n** Unable to use %s %d:%d for fatload **\n", "mmc", dev, part);
		return 1;
	}
	return 0;
}


#if 1
int TQ210_boot_src()
{
	ulong reg;
//	ulong src;

	reg = (*(volatile u32 *)(INF_REG_BASE + INF_REG3_OFFSET));

//	printf("reg=0x%x, BOOT_MMCSD=0x%x, BOOT_NAND=0x%x\n", reg, BOOT_MMCSD, BOOT_NAND);
	return reg;
}

int do_Wboot2SD (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char cmd_buf[200];
	
	sprintf(cmd_buf, "movi write u-boot %s", argv[1]);
	if(run_command(cmd_buf, 0)<0)
	{
		printf("No MMC card found\n");
		return -1;
	}
	printf("u-boot write OK !\n");
	return 0;
}

U_BOOT_CMD(
	Wboot2SD,	2,	0,	do_Wboot2SD,
	"Write u-boot to SD Card",
	"[ImageAddress]\n"
	"\n"
);

#endif

#ifdef CONFIG_TQ_READ_LINE
/*
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out

修改：具有记忆功能，针对环境变量的修改，可以保存上次设置的值，并可在上次的基础上进行环境变量的修改
 */
#include <watchdog.h>
extern char console_buffer[];
static const char erase_seq[] = "\b \b";		/* erase sequence	*/
static const char   tab_seq[] = "        ";		/* used to expand TABs	*/
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			puts (erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				puts (tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				putc (*s);
			}
		}
	} else {
		puts (erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

int TQ_readline (const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	return TQ_readline_into_buffer(prompt, console_buffer);
}


int TQ_readline_into_buffer (const char *const prompt, char * buffer)
{
	char *p = buffer;
#ifndef CONFIG_CMDLINE_EDITING
	unsigned int len=MAX_CMDBUF_SIZE;
	int rc;
	static int initted = 0;

	/*
	 * History uses a global array which is not
	 * writable until after relocation to RAM.
	 * Revert to non-history version if still
	 * running from flash.
	 */
	//if (gd->flags & GD_FLG_RELOC) {
		if (!initted) {
			hist_init();
			initted = 1;
		}

		puts (prompt);

		rc = cread_line(prompt, p, &len);
		return rc < 0 ? rc : len;

	//} else {
//#endif	/* CONFIG_CMDLINE_EDITING */
#else
	char * p_buf = p;
	int	n = 0;				/* buffer index		*/
	int	plen = 0;			/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;

	/* print prompt */
	if (prompt) {
		plen = strlen (prompt);
		strcpy(p, prompt);
		p = p + plen;
		p_buf = p;
		puts (prompt);
	}
	col = plen;
	n = plen;
	for (;;) {
#ifdef CONFIG_BOOT_RETRY_TIME
		while (!tstc()) {	/* while no incoming data */
			if (retry_time >= 0 && get_ticks() > endtime)
				return (-2);	/* timed out */
		}
#endif
		WATCHDOG_RESET();		/* Trigger watchdog, if needed */

#ifdef CONFIG_SHOW_ACTIVITY
		while (!tstc()) {
			extern void show_activity(int arg);
			show_activity(0);
		}
#endif
		c = getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			puts ("\r\n");
			return (p - p_buf);

		case '\0':				/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				puts (erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(p_buf, p, &col, &n, plen);
			}
			continue;

		case 27:	continue;			/* 	无效*/
#if 0
		case 91:	continue;			/*  无效*/
#endif
		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Must be a normal character then
			 */
			if (n < CFG_CBSIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
#ifdef CONFIG_AUTO_COMPLETE
					/* if auto completion triggered just continue */
					*p = '\0';
					if (cmd_auto_complete(prompt, console_buffer, &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
#endif
					puts (tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					putc (c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				putc ('\a');
			}
		}
	}
//#ifdef CONFIG_CMDLINE_EDITING
//	}
#endif
}
#endif  /* CONFIG_TQ_READ_LINE */




#endif /* CONFIG_EMBEDSKY_FAT */

