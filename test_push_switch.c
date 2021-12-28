#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MAX_BUTTON 9
unsigned char quit = 0;

void user_signal1(int sig) { quit = 1; }

void display(unsigned char *addr) {
				int i, j;
				system("clear");
				printf("Press <ctrl + c> to quit. \n\n");
				for(i=0 ; i<3 ; i++) {
								for(j=0 ; j<3 ; j++) {
												if(addr[i*3 + j] == 1) printf("%d\t", i*3+j+1);
												else if(addr[i*3+j] == 0) printf("\t");
												else printf("E\t");
								}
								printf("\n");
				}
}

int main(void) {
				int dev;
				unsigned char push_sw_buff[MAX_BUTTON];

				dev = open("/dev/csemad_push_switch", O_RDWR);

				if(dev == -1) {
								printf("fail to open\n");
								return -1;
				}

				(void)signal(SIGINT, user_signal1);

				while(!quit) {
								usleep(200000);
								if(read(dev, push_sw_buff, MAX_BUTTON) == -1) {
												printf("fail to read\n");
												return -1;
								}
								display(push_sw_buff);
				}
				close(dev);
				return 0;
}

