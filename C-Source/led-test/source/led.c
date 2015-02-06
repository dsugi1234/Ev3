#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLACK           0+'0'
#define GREEN           1+'0'
#define RED             2+'0'
#define ORANGE          3+'0'
#define GREEN_FLASH     4+'0'
#define RED_FLASH       5+'0'
#define ORANGE_FLASH    6+'0'
#define GREEN_PULSE     7+'0'
#define RED_PULSE       8+'0'
#define ORANGE_PULSE    9+'0'

int uifp;       /* File descripter for /dev/lms_ui */

int SetLed(unsigned char pat)
{
        unsigned char Buf[2];
        int ret;

        Buf[0] = pat;
        Buf[1] = 0;

        ret = write(uifp, Buf, 2);// LEDのモードを渡しています

        return ret;
}


int main(void)
{
        int cnt;

        uifp = open("/dev/lms_ui",O_RDWR); // Open LED device
        if (uifp < 0) {
                printf("Cannot open /dev/lms_ui\n");
                exit(-1);
        }

        SetLed(BLACK);
        sleep(2);
        SetLed(GREEN);
        sleep(2);
        SetLed(RED);
        sleep(2);
        SetLed(ORANGE);
        sleep(2);
        SetLed(GREEN_FLASH);
        sleep(2);
        SetLed(RED_FLASH);
        sleep(2);
        SetLed(ORANGE_FLASH);
        sleep(2);
        SetLed(GREEN_PULSE);
        sleep(2);
        SetLed(RED_PULSE);
        sleep(2);
        SetLed(ORANGE_PULSE);
        sleep(2);

        close(uifp);
        exit(0);
}
