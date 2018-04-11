#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define  CHAIN     (16)
#define  BLOCK     (32)
#define  MAX_SIZE  (1024)

int main(int argc, char* argv[])
{
	char rec_file_name[15] = {'\0'};
	FILE *rec_file_fp[CHAIN];
	int  audio_file_fd;
	char buf[MAX_SIZE ];
	fd_set rfds;
	int  i,j,ret,step;

	for (i=0; i<CHAIN; i++)
	{
		sprintf(rec_file_name, "audio_%d.pcm", i);
		rec_file_fp[i] = fopen(rec_file_name,"w");
	}

	audio_file_fd = open("/dev/kyaudio", O_RDONLY);
	assert(audio_file_fd>0);

	/* Watch audio_file_fd to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	while (1)
	{

		ret = read(audio_file_fd,buf,MAX_SIZE);
		if (0 != ret%32)
		{
			printf("automan debug!in select!\n");
			select(audio_file_fd+1, &rfds, NULL, NULL,NULL);
			continue;
		} 

		else
		{
			for(j=0,step=0;j<BLOCK;j++)
			{

				for (i=0; i<CHAIN;i++)
				{

					ret = fwrite(buf+step,2,1,rec_file_fp[i]);
					step += 2;
				}
			}

		}
	}


	return 0;
}
