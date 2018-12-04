#include "vs10XX.h"
#include "ff.h"
#include "key.h"
#include "MP3PLAY.H"
#include "delay.h" 	
#include "sdio_sdcard.H"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

unsigned char data[2048];
unsigned char sd_buffer[32];
unsigned char buffer_count; 
unsigned char stop = 0;
unsigned short yl=0x2020;
unsigned char mode,keyResult,keyCount;

extern FATFS fs;
extern FRESULT res;
extern UINT br;

unsigned char mp3Play(const char* mp3name)
{
	u8 res,i;
	FIL file;
	
	res = f_open(&file, mp3name, FA_READ);

	//Mp3Reset();       //硬复位
	VS_Soft_Reset();//软复位VS1003
	//Vs1003_CMD_Write(0x0b,yl);
	for(buffer_count=0; buffer_count<32; buffer_count++)
		sd_buffer[buffer_count] = 0;
	if(!res)
	{
		for(;;)
		{
			res = f_read(&file, sd_buffer, 32, &br);
			while(VS_DQ==0); 				//等待空闲
			for(i=0;i<32;i++)Vs1003_DATA_Write(sd_buffer[i]);//发送音乐数据   
			if(br<32||stop == 1)
			{
				//Vs1003_CMD_Write(0x0b,0Xffff);	  //设置音量;
				stop = 0;
				break;
			}
			if(keyCount < 200)keyCount ++;
#if 0			
			if(getKey()==KEY0)
			{
				if (keyCount >= 200)
				{
					//printp("key0%d",keyCount);
					keyCount = 0;
					if(yl <= 0x8000)
					{
						yl += 0x1010;
					}
					else
					{
						yl = 0xffff;
					}
					Vs1003_CMD_Write(0x0b,yl);	  //设置音量;
				}
			}
			if(getKey()==KEY1)
			{
				if (keyCount >= 200)
				{
					//printp("key1%d",keyCount);
					keyCount = 0;
					if(yl >= 0x1010)
					{
						yl -= 0x1010;
					}
					Vs1003_CMD_Write(0x0b,yl);	  //设置音量;
				}
			}
			if(getKey()==KEYMODE)
			{
				if (keyCount >= 200)
				{
					//printp("keym%d",keyCount);
					keyCount = 0;
					stop = 1;
				}
			}
#endif
		}
	}
	f_close(&file);
 	return 0;
}


