#include "common.h"
#include "malloc.h"

SYS_ENV g_sys_env;
static u32 *g_flash_sector =  NULL;

void sys_env_init(void)
{
	g_flash_sector = mymalloc(SRAMIN, 4096);	//����4K�ֽ��ڴ�  
	if (NULL == g_flash_sector) {
		myfree(SRAMIN, g_flash_sector);
	}
}

void sys_env_save(void)
{
	u16 j = 0;
	u32 *buf = g_flash_sector;

	W25QXX_Read((u8*)buf, ENV_SECTOR_INDEX*4096, 4096);//������������������
	for (j=0; j<1024; j++) {//У������
		if(buf[j]!=0XFFFFFFFF)break;//��Ҫ����  	  
	}
	
	if (j != 1024) {
		W25QXX_Erase_Sector(ENV_SECTOR_INDEX);	//��Ҫ����������
	}

	memset((u8*)buf, 0, 6096);
	memcpy((u8*)buf, (u8*)&g_sys_env, sizeof(SYS_ENV));
	W25QXX_Write((u8*)buf, ENV_SECTOR_INDEX*4096, 4096);
	
	myfree(SRAMIN, buf);
}

void sys_env_dump(void)
{
	W25QXX_Read((u8*)&g_sys_env, ENV_SECTOR_INDEX*4096, sizeof(SYS_ENV));//������������������
	
	printf("charge_times = %d\n", g_sys_env.charge_times);
}
