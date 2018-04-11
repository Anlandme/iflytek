#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <strings.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ALL_CHANNEL_COUNT 13 
#define BASESIZE (512)
#define BUFSIZE (BASESIZE * ALL_CHANNEL_COUNT)  

int main(void)
{

	char *rec_file_name = "audio.pcm";
	FILE *rec_file_fp;
	int  audio_file_fd;
	char buf[BUFSIZE];
	char tempbuf[BUFSIZE];
	int i = 0,idx = 0,step = 0,ret = -1,len = 0;
	
	
	rec_file_fp = fopen(rec_file_name,"w");
	assert(rec_file_fp != NULL);

	audio_file_fd = open("/dev/mscap0", O_RDONLY);
	assert(audio_file_fd>0);

	while(1)
	{
		bzero(buf, sizeof(buf));
		bzero(tempbuf,sizeof(tempbuf));
		len = 0;
		step = 0;

		do
		{
			ret = read(audio_file_fd, &buf[len], BUFSIZE - len);
			if(ret < 0)
			{
				continue;
			}
			len += ret;
		}while(len < BUFSIZE);

		for(idx = 0; idx < BUFSIZE; )
		{
			int ch = buf[idx];

			if (ch >= ALL_CHANNEL_COUNT || ch < 0)
			{
				idx += 4;
				continue;
			}
			idx += 2;

			if (ch == 0)
			{
				memcpy(tempbuf+step,buf+idx,2);
				step += 2;
			}

			idx += 2;
		}

		fwrite(tempbuf,step,1,rec_file_fp);
	}

}
