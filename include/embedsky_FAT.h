#ifndef __EMBEDSKY_FAT__H
#define __EMBEDSKY_FAT__H

#define FAT_SECTOR		512
#define Fuchar			unsigned char
#define Fuint			unsigned int
#define Fusint			unsigned short int

/*typedef struct _UbootInitial
{
	char condition[8];
	char OsType[9];
	char LEDindicator[4];
	//char 
}UInit;*/
#define SD_DW_UBOOT     (1<<0)
#define SD_DW_LOGO      (1<<1)
#define SD_DW_KERNEL    (1<<2)
#define SD_DW_YAFFS	(1<<3)
#define SD_DW_UBIFS	(1<<4)
#define SD_DW_ALL	(0xff)
typedef struct DirFileCountReturn
{
	WORD GroupVal;				//�ļ���ռ��֡��
	WORD FileEnable;			//�ļ�ʹ�� 0ΪĿ¼ 1Ϊ�ļ� 0xff;
	DWORD StartCluster;			//�ļ�����ʼ���
	DWORD FileSize;				//�ļ��ĳ��� ��λKB
	
}DirFunRet;

struct bootsector710 {
	U8	bsJump[3];			// jump inst E9xxxx or EBxx90
	char	bsOemName[8];			// OEM name and version
	char	bsBPB[53];			// BIOS parameter block
	char	bsExt[26];			// Bootsector Extension
	char	bsBootCode[418];		// pad so structure is 512b
	U8	bsBootSectSig2;			// boot sector signature byte 0x00 
	U8	bsBootSectSig3;			// boot sector signature byte 0x00
	U8	bsBootSectSig0;			// boot sector signature byte 0x55 
	U8	bsBootSectSig1;			// boot sector signature byte 0xAA
#define BOOTSIG0        0x55
#define BOOTSIG1        0xaa
#define BOOTSIG2        0x00
#define BOOTSIG3        0x00
};

	
struct bpb710{
	unsigned int	bpbBytesPerSec;		// bytes per sector
	U8	bpbSecPerClust;			// sectors per cluster
	unsigned int	bpbResSectors;		// number of reserved sectors
	U8	bpbFATs;			// number of FATs
	unsigned int	bpbRootDirEnts;		// number of root directory entries
	unsigned int	bpbSectors;		// total number of sectors
	U8	bpbMedia;			// media descriptor
	unsigned int	bpbFATsecs;		// number of sectors per FAT
	unsigned int	bpbSecPerTrack;		// sectors per track
	unsigned int	bpbHeads;		// number of heads
	U32	bpbHiddenSecs;			// # of hidden sectors
// 3.3 compat ends here
	U32	bpbHugeSectors;			// # of sectors if bpbSectors == 0
// 5.0 compat ends here
	U32     bpbBigFATsecs;			// like bpbFATsecs for FAT32
	unsigned int      bpbExtFlags;		// extended flags:
#define FATNUM    0xf				// mask for numbering active FAT
#define FATMIRROR 0x80				// FAT is mirrored (like it always was)
	unsigned int      bpbFSVers;		// filesystem version
#define FSVERS    0				// currently only 0 is understood
	U32     bpbRootClust;			// start cluster for root directory
	unsigned int      bpbFSInfo;		// filesystem info structure sector
	unsigned int      bpbBackup;		// backup boot sector
	// There is a 12 byte filler here, but we ignore it
} ;//__attribute__((__packed__))   ;

struct   partrecord// length 16 bytes
{			
	U8	prIsActive;			// 0x80 indicates active partition
	U8	prStartHead;			// starting head for partition
	unsigned int	prStartCylSect;		// starting cylinder and sector
	U8	prPartType;			// partition type (see above)
	U8	prEndHead;			// ending head for this partition
	unsigned int 	prEndCylSect;		// ending cylinder and sector
	U32	prStartLBA;			// first LBA sector for this partition
	U32	prSize;				// size of this partition (bytes or sectors ?)
};// __attribute__((__packed__)) ;

struct partsector
{
	char	psPartCode[446];		// pad so struct is 512b
	U8	psPart[64];			// four partition records (64 bytes)
	U8	psBootSectSig0;			// two signature bytes (2 bytes)
	U8	psBootSectSig1;
#define BOOTSIG0        0x55
#define BOOTSIG1        0xaa
};

struct FileInfoStruct
{
	U32 StartCluster;			//< file starting cluster for last file accessed
	U32 Size;				//< file size for last file accessed
	U32 Attr;				//< file attr for last file accessed
	//unsigned short CreateTime;		//< file creation time for last file accessed
	//unsigned short CreateDate;		//< file creation date for last file accessed
	U32 Sector;				//<file record place
	U32 Offset;				//<file record offset
};

// Structure of a dos directory entry.
typedef struct direntry {
	U8		deName[8];      	// filename, blank filled
#define SLOT_EMPTY      0x00            // slot has never been used
#define SLOT_E5         0x05            // the real value is 0xE5
#define SLOT_DELETED    0xE5            // file in this slot deleted
#define SLOT_DIR		0x2E			// a directorymmm
//	U8		deAttributes;   	// file attributes
	U8		deExtension[3]; 	// extension, blank filled
	U8		deAttributes;   	// file attributes
#define ATTR_NORMAL     0x00            // normal file
#define ATTR_READONLY   0x01            // file is readonly
#define ATTR_HIDDEN     0x02            // file is hidden
#define ATTR_SYSTEM     0x04            // file is a system file
#define ATTR_VOLUME     0x08            // entry is a volume label
#define ATTR_LONG_FILENAME	0x0F		// this is a long filename entry			    
#define ATTR_DIRECTORY  0x10            // entry is a directory name
#define ATTR_ARCHIVE    0x20            // file is new or modified
	U8        deLowerCase;    	// NT VFAT lower case flags  (set to zero)
#define LCASE_BASE      0x08            // filename base in lower case
#define LCASE_EXT       0x10            // filename extension in lower case
	U8        deCHundredth;   	// hundredth of seconds in CTime
	U8        deCTime[2];     	// create time
	U8        deCDate[2];     	// create date
	U8        deADate[2];     	// access date
	unsigned short int        deHighClust; 	// high bytes of cluster number
	U8        deMTime[2];     	// last update time
	U8        deMDate[2];     	// last update date
	unsigned short int        deStartCluster; 	// starting cluster of file
	U32       deFileSize;  		// size of file in bytes
}ShortName;

// Structure of a dos directory entry.
typedef struct long_direntry {
	U8 	FlagValue;			//��־��
	U8	deName0[10];      		// ���ļ���unicode��1 
	U8    deAttributes; 		    	//���ļ���Ŀ¼���־��һ��ȡֵ0x0FH
	U8    sysPreserv;			//ϵͳ����
	U8    checkoutVal;			//У��ֵ�����Ը��ݶ��ļ����ó���
	U8	deName1[12];			//���ļ���unicode��2 
	unsigned short int 	deStartCluster;	// starting cluster of file
	U8    deName2[4];			//���ļ���unicode��3
}LongName;
//extern DWORD MMC_SD_ReadCapacity();
//extern BYTE Rd_Block(DWORD addr,BYTE* buffer);

extern BYTE FAT_LoadPartCluster(DWORD cluster,DWORD part,BYTE * buffer);//�����ļ�
//extern DWORD FAT_NextCluster_NEW(DWORD cluster);//fit system which has large ram
//FATԭ�еĺ���
extern unsigned char FAT_Init(void);//��ʼ��
extern DWORD FAT_NextCluster(DWORD cluster);//������һ�غ�
extern unsigned long FAT_FindFreeCluster(void);//���ҿմ�
extern unsigned char FAT_DisDir(BYTE *dir);//��ʾĿ¼����
extern WORD FAT_FindItem(DWORD cluster, BYTE *name, struct FileInfoStruct *FileInfo);//�����ļ�
extern DWORD FAT_Open(BYTE * dir);//���ļ�
extern DWORD FAT_OpenDir(BYTE * dir);//��Ŀ¼
extern unsigned char FAT_Read(DWORD pointer, DWORD size);//���ļ���size=0���������ļ�
extern unsigned char FAT_Rename(BYTE *dir,BYTE *newname);//������
extern unsigned char FAT_ModifyFAT(DWORD cluster,DWORD val);//�޸�FAT��
extern unsigned char FAT_Delete(BYTE *dir);//ɾ���ļ�
extern unsigned char FAT_DelItem(struct FileInfoStruct *FileInfo);//ɾ����¼��
extern WORD FAT_FindFreeItem(DWORD cluster, struct FileInfoStruct *FileInfo);//���ҿ�����
extern unsigned char FAT_MkDir(BYTE * dir);//��Ŀ¼
extern unsigned char FAT_Write(DWORD cluster,unsigned char *data,DWORD size);//д�ļ�
extern DWORD FAT_Create(BYTE * dir,DWORD size);//���ļ�������ָ���ļ���С
extern unsigned char FAT_RmDir(BYTE * dir);//ɾ��Ŀ¼
extern WORD FAT_Close(DWORD * p);//�ر��ļ�
extern unsigned char FAT_GetFile(BYTE *dir,struct FileInfoStruct *FileInfo);//��ȡָ��Ŀ¼�µ��ļ���Ϣ
extern DWORD FAT_OpenFile(char *dir,struct FileInfoStruct *File_Info);//1=ʧ��.//��ָ��·�����ļ�,����ļ���Ϣ�������ļ����״�.
extern BYTE FAT_DirLoadFile(char *dir,BYTE *Buf); //0=ʧ��;1=�ɹ�.//ͨ��Ŀ¼�����ļ�������
extern BYTE FAT_ClusterLoadFile(DWORD cluster,BYTE *Buf);//0=ʧ��;1=�ɹ�.//ͨ���ؼ��������ļ���Buf.
extern int FAT_RunFile(char *ini_dir); 


#ifdef CONFIG_OEM_SDREAD
extern DWORD FAT_OEMReadFile(DWORD Dir, char *destbuffer, DWORD filesize, unsigned char tran);
#endif
extern DWORD FAT_ReadFile(char *file,char *destbuffer,DWORD filesize,unsigned char tran);
extern DirFunRet FindDirFile(char *filename);
extern int DirFile(void);
extern char UbootStructInit(void);
extern char SD_down_main(int is_nandtype);

#endif		/* __EMBEDSKY_FAT__H */
