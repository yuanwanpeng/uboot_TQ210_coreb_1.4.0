#!/bin/sh

if [ ! $1 ] ; then
	echo "SD device is not set"
	exit 1
fi

SDCARD=$1

if [ ! -e ${SDCARD} ] ; then 
	echo "SD卡不存在，请插入SD卡后重新执行命令!"
#	echo ${SDCARD}
	exit 0
fi

#对SD卡分区，留30MB的空白空间（即61440块），创建第一个分区，设置为几百MB即可，创建第二个分区，剩余全部空间。
#第一个分区格式化为fat32,第二个分区格式化为ext4：
if [ $2 ] && [ $2 = "yes" ] ; then 
	umount ${SDCARD}*
#	(echo d;echo d;echo d;echo d;echo d;echo n;echo p;echo 1;echo 61440;echo +200M;echo n;echo p;echo 2;echo 61440;echo ;echo ;echo w)|fdisk ${SDCARD}
	(echo p; echo d; echo ; echo d; echo ; echo d; echo ; echo d; echo ; echo w) | fdisk ${SDCARD} && 
	(echo n; echo p; echo 1; echo 61440; echo +200M; echo w) | fdisk ${SDCARD} && 
	(echo n; echo p; echo 2; echo 61440; echo ; echo ; echo p; echo w) | fdisk ${SDCARD} && 

	mkfs.vfat ${SDCARD}1 && 
	(echo y) | mkfs.ext4 -L ext4 ${SDCARD}2
fi
#制作并导入bl1镜像和u-boot镜像：
./sd_fusing/mkbl1 u-boot.bin SD-bl1-8k.bin 8192 && 
dd iflag=dsync oflag=dsync if=SD-bl1-8k.bin of=${SDCARD} seek=1 && 
dd iflag=dsync oflag=dsync if=u-boot.bin of=${SDCARD} seek=49

