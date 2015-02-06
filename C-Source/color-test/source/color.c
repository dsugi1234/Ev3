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

#define MOD_COL_REFLECT 0
#define MOD_COL_AMBIENT 1
#define MOD_COL_COLOR   2

int uartfp;
UART *pUart; // UARTは“lms2012.h”で定義されています

unsigned char GetSensor(unsigned char ch)
{
        return((unsigned char)pUart->Raw[ch][pUart->Actual[ch]][0]);
}

int ChgSensorMode(unsigned char ch, int mode)
{
        int i;
        int ret;
        DEVCON DevCon; // DEVCONは“lms2012.h”で定義されています

        for (i=0; i < 4; i++) {
                DevCon.Connection[i] = CONN_NONE;
        }

        DevCon.Connection[ch] = CONN_INPUT_UART;
        DevCon.Mode[ch] = (unsigned char)mode;

        // ioctl()によりカラーセンサーのモードを設定します
        ret = ioctl(uartfp, UART_SET_CONN, &DevCon);

        return ret;
}

int main(int argc, char *argv[])
{
        int i;
        unsigned char sensor;
        int mode;

        if (argc !=2) {
                printf("Required mode value 0:COL_REFLECT 1:COL_AMBIENT 2:CON_COLOR\n");
                printf("    Usage: color [mode]\n");
                exit(1);
        }

        mode = atoi(argv[1]);
        if ((mode < 0) || (mode > MOD_COL_COLOR)) {
                printf("mode is out of range.\n");
                exit(1);
        }

        // UART デバイスドライバをオープンします
        uartfp = open("/dev/lms_uart", O_RDWR | O_SYNC);
        if (uartfp < 0) {
                printf("Cannot open UART_DEVICE\n");
                exit(-1);
        }

        // mmap()によりUARTデバイスドライバの共有メモリをマップします
        pUart  =  (UART*)mmap(0, sizeof(UART), 
                    PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, uartfp, 0);
        if (pUart == MAP_FAILED) {
                printf("Failed to map device\n");
                exit(-1);
        }

        ChgSensorMode(CH_4, mode); // カラーセンサーの動作モードを設定

        for (i = 0; i<10; i++) {
                sensor = GetSensor(CH_4); // CH4のセンサー値を取得
                printf("Color sensor: %d \n", sensor);
                sleep(1); /* 1Sec */
        }

        munmap(pUart, sizeof(UART));
        close(uartfp);
        exit(0);
}
