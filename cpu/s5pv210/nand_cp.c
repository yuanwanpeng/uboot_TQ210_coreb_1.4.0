/*
 * $Id: nand_cp.c,v 1.1 2008/11/20 01:08:36 boyko Exp $
 *
 * (C) Copyright 2006 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * You must make sure that all functions in this file are designed
 * to load only U-Boot image.
 *
 * So, DO NOT USE in common read.
 *
 * By scsuh.
 */

/*
 * Large block only: Small block is not supported.
 * No ECC operation: Bit error is not corrected.
 */

#include <common.h>
#ifdef CONFIG_S5PC11X
#include <asm/io.h>
#include <linux/mtd/nand.h>
#include <regs.h>

#define NAND_CONTROL_ENABLE()	(NFCONT_REG |= (1 << 0))

#if defined(CONFIG_SECURE_BOOT)
#include <secure_boot.h>
#endif /* CONFIG_SECURE_BOOT */

#if USE_IROM
//用法：a表示要读取的第一个块地址（一直不变），b表示要读取的页地址（每次都变化），c表示内存地址
//即要读取从第10个块开始的20个块的内容，即a填写10,b填写20个块中的页地址，每个块64个页，第2个块的页地址从64开始。返回值：正确为0，失败为0x8。
#define NF8_ReadPage_Adv(a,b,c)	(((int(*)(u32, u32, u8 *))(*((u32 *)0xD0037F90)))(a,b,c))
#endif /* USE_IROM */

#if !USE_IROM
static void nand_readpage (ulong col, ulong row, uchar* buf, int len)
{
	int i;

	NAND_ENABLE_CE();

	NFCMD_REG = NAND_CMD_READ0;
	/* Write Column Address */
	NFADDR_REG = (col) & 0xff;
	NFADDR_REG = (col >> 8) & 0xff;

	/* Write Row Address */
	NFADDR_REG = (row) & 0xff;
	NFADDR_REG = (row >> 8) & 0xff;
	NFADDR_REG = (row >> 16) & 0xff;

	NFCMD_REG = NAND_CMD_READSTART;

	NF_TRANSRnB();

	for (i = 0; i < len; i++) {
		buf[i] = NFDATA8_REG;
	}

	NAND_DISABLE_CE();
}

static int nand_isbad (ulong addr, int page_size)
{
//	int page_size = 4096;//2048;
	uchar oob[2];

	if (addr == 0)
		return 0;

	nand_readpage(page_size, addr, oob, 2);

	if ((oob[0] == 0xFF) && (oob[1] == 0xFF))
		return 0;
	else
		return 1;
}

/*
 * address format
 *              17 16         9 8            0
 * --------------------------------------------
 * | block(12bit) | page(5bit) | offset(9bit) |
 * --------------------------------------------
 */

static int nandll_read_page (uchar *buf, ulong addr, int page_size)
{
//	int page_size = 4096;//2048;

	nand_readpage(0, addr, buf, page_size);

	return 0;
}
#endif /* !USE_IROM */

/*
 * Read data from NAND.
 */
static int nandll_read_blocks (ulong dst_addr, ulong size, int page_size)
{
	uchar *buf = (uchar *)dst_addr;
	int i;
#if USE_IROM
	u32 page_addr, block_addr;
#endif /* USE_IROM */

#if defined(CONFIG_SECURE_BOOT)
	int skipped_page = 6;
#else
	int skipped_page = 0;
#endif /* CONFIG_SECURE_BOOT */

	/* 11是2K page，12是4K page */
	uint page_shift = 11;
	if(page_size == 8192)
		page_shift = 13;
	else if(page_size == 4096)
		page_shift = 12;

#if USE_IROM
	block_addr = (0+skipped_page)/64;
#endif /* USE_IROM */
	/* Read pages */
	for (i = 0; i < (size>>page_shift); i++, buf+=(1<<page_shift)) {
#if USE_IROM
		page_addr = (i+skipped_page);//%64;
		NF8_ReadPage_Adv(block_addr, page_addr, buf);
#else
		if ((i & (64 - 1)) == 0) {
			while (nand_isbad(i + skipped_page, page_size)) {
				skipped_page += 64;
			}
		}
		nandll_read_page(buf, i + skipped_page, page_size);
#endif /* USE_IROM */

	}
	return 0;
}
int copy_uboot_to_ram (void)
{
	int large_block = 0;
	int i;
	vu_char maf_id, dev_id;
	int page_size = 2048;

	int rv;

	// reset nand
#if 0
	NFCONT_REG &= ~(1<<1);//NF_nFCE_L(); 
	NFSTAT_REG = (1<<4);//NF_CLEAR_RB();
	NFCMD_REG = (unsigned char)(NAND_CMD_RESET);//NF_CMD(CMD_RESET);
	for(i=0;i<10;i++);
	while((NFSTAT_REG & 0x11) != 0x11);//NF_DETECT_RB();
	NFCONT_REG |= (1<<1);//NF_nFCE_H();  
#endif

	NAND_CONTROL_ENABLE();
	NAND_ENABLE_CE();
	NFCMD_REG = NAND_CMD_READID;
	NFADDR_REG =  0x00;

	/* wait for a while */
	for (i=0; i<200; i++);
	maf_id = NFDATA8_REG;
	dev_id = NFDATA8_REG;

	if(maf_id == 0x2C)
	{
		if(dev_id == 0x64)
			page_size = 8192;
		else
			page_size = 4096;
	}

	if (dev_id > 0x80)
		large_block = 1;
	else
	{
		return -1;	// Do not support small page (512B) any more
	}

	/* read NAND blocks */
	rv = nandll_read_blocks(CFG_PHY_UBOOT_BASE, COPY_BL2_SIZE, page_size);

#if defined(CONFIG_SECURE_BOOT)
	rv = Check_Signature((SecureBoot_CTX *)SECURE_BOOT_CONTEXT_ADDR,
			(unsigned char *)CFG_PHY_UBOOT_BASE,
			(1024*512 - 128),
			(unsigned char *)(CFG_PHY_UBOOT_BASE + (1024*512-128)),
			128);
	if (rv != SB_OK)
		while(1);
#endif /* CONFIG_SECURE_BOOT */

	return rv;
}
 
#endif

