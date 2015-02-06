#include  <stdio.h>
#include  <fcntl.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <sys/mman.h>
#include  <sys/ioctl.h>
#include "lms2012.h"

#define CH_1    0x00
#define CH_2    0x01
#define CH_3    0x02
#define CH_4    0x03

int analogfp;
ANALOG *pAnalog;

int GetTouch(unsigned char ch)
{
        if (pAnalog->Pin6[ch][pAnalog->Actual[ch]] > 100) {
                return 1;
        } else {
                return 0;
        }
}

int main()
{
        int i;
        int touch;

        // Open Analog device file
        analogfp = open("/dev/lms_analog", O_RDWR | O_SYNC);
        if (analogfp < 0) {
                printf("Cannot open /dev/lms_analog\n");
                exit(-1);
        }

        // mmap（）によりドライバの共有メモリにアクセスできる様にします
        pAnalog  =  (ANALOG*)mmap(0, sizeof(ANALOG), 
                PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, analogfp, 0);
        if (pAnalog == MAP_FAILED) {
                printf("Failed to map device\n");
                exit(-1);
        }

        for (i = 0; i<10; i++) {
                touch = GetTouch(CH_4); // CH-4のデータを取得します
                printf("Touch sensor: %d \n", touch);
                sleep(1); /* 1Sec */
        }

        munmap(pAnalog, sizeof(ANALOG));
        close(analogfp);
        exit(0);
}
