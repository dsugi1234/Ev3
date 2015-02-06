#include  <stdio.h>
#include  <fcntl.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <signal.h>
#include "lms2012.h"

#define CH_A    0x01
#define CH_B    0x02
#define CH_C    0x04
#define CH_D    0x08

int pwmfp;

int PrgStart(void)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opPROGRAM_START;
        ret = write(pwmfp,Buf,1);
        return ret;
}

int PrgStop(void)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opPROGRAM_STOP;
        ret = write(pwmfp,Buf,1);
        return ret;
}

int MotorStop(unsigned char ch)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opOUTPUT_STOP;
        Buf[1] = ch;
        ret = write(pwmfp,Buf,2);
        return ret;
}

int MotorStart(void)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opOUTPUT_START;
        Buf[1] = CH_A | CH_B | CH_C | CH_D;
        ret = write(pwmfp,Buf,2);
        return ret;
}

int MotorPower(unsigned char ch, unsigned char power)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opOUTPUT_POWER;
        Buf[1] = ch;
        Buf[2] = power;
        ret = write(pwmfp,Buf,3);
        return ret;
}

int MotorReset(unsigned char ch)
{
        unsigned char Buf[4];
        int ret;

        Buf[0] = opOUTPUT_RESET;
        Buf[1] = ch;
        ret = write(pwmfp,Buf,2);
        return ret;
}

int main(void)
{
        pwmfp = open("/dev/lms_pwm",O_RDWR);
        if (pwmfp < 0) {
                printf("Cannot open dev/lms_pwm\n");
                exit(-1);
        }

        PrgStop();
        PrgStart();
        MotorReset(CH_D);
        MotorStart();
        MotorPower(CH_D, 30);
        sleep(2);
        MotorPower(CH_D, 60);
        sleep(2);
        MotorPower(CH_D, 100);
        sleep(2);
        MotorPower(CH_D, 0);
        sleep(2);
        MotorPower(CH_D, (unsigned char)-30);
        sleep(2);
        MotorPower(CH_D, (unsigned char)-60);
        sleep(2);
        MotorPower(CH_D, (unsigned char)-100);
        sleep(2);
        MotorPower(CH_D, 0);
        sleep(2);
        MotorStop(CH_D);
        PrgStop();

        close(pwmfp);
        exit(0);
}
