执行命令：make，可以编译出来mkbl1和sd_fdisk两个工具。
mkbl1用于制作8K的bl1镜像，sd_fdisk用于制作mbr的分区文件。

制作SD启动卡的命令：
对SD卡分区，留30MB的空白空间（即61440块），创建第一个分区，设置为几百MB即可，创建第二个分区，剩余全部空间。
第一个分区格式化为fat32,第二个分区格式化为ext4：
fdisk /dev/sdd
mkfs.vfat /dev/sdd1
mkfs.ext4 /dev/sdd2
制作并导入bl1镜像和u-boot镜像：
mkbl1 ../u-boot.bin SD-bl1-8k.bin 8192
dd iflag=dsync oflag=dsync if=SD-bl1-8k.bin of=/dev/sdd seek=1
dd iflag=dsync oflag=dsync if=../u-boot.bin of=/dev/sdd seek=49





