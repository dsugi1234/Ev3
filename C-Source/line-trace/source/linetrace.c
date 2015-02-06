#include  <stdio.h>
#include  <fcntl.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <signal.h>
#include  <sys/mman.h>
#include "lms2012.h"

#define CH_A    0x01
#define CH_B    0x02
#define CH_C    0x04
#define CH_D    0x08

#define CH_1    0x00
#define CH_2    0x01
#define CH_3    0x02
#define CH_4    0x03

/* Motor port */
#define RMOT_CH CH_B  // 右側モーター  CH?B
#define LMOT_CH CH_C  // 左側モーター  CH-C

/* Sensor port */
#define SENS_CH CH_3  // カラーセンサー  CH-3

/* Motor speed */
#define SPEED_D 30
#define SPEED_3 40
#define SPEED_2 10
#define SPEED_1 5

// 進行方向の定義
typedef enum{
    D_STRAIGHT      = 1,
    D_RIGHT         = 2,
    D_LEFT          = 3
} DRIVE_MODE;

// 走行エリアの定義 
typedef enum{
    A_INIT_WHITE    = 0,
    A_R_WHITE       = 1,
    A_R_EDGE        = 2,
    A_BLACK         = 3,
    A_L_EDGE        = 4,
    A_L_WHITE       = 5
} CURRENT_AREA;

// 走行エリア変更の定義
typedef enum{
    IDOLMOD         = 0,
    RW2RE           = 1,  // Right White -> Right Edge
     RE2RW           = 2, // Right Edge -> Right White
     RE2B            = 3, // Right Edge -> Black
     B2RE            = 4, // Black -> Right Edge
     LW2LE           = 5, // Left White -> Left Edge
     LE2LW           = 6, // Left Edge -> Left White
     B2LE            = 7, // Black -> Left Edge
     LE2B            = 8, // Left Edge -> Black
     INIT2RW         = 9
} CHANGE_AREA;

int pwmfp;
int uartfp;
UART *pUart;

unsigned char HighTH;
unsigned char LowTH;


/* Motor control */
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

int LineTrace(unsigned char sensor)  // ライントレース制御関数
{
    static unsigned char pre_sensor=0;

    static unsigned char pre_Rmotor=0, pre_Lmotor=0;
    unsigned char Rmotor, Lmotor;

    static int pre_change_mod=0;
    int change_mod;

    int f_change;
    int D_mode;

    static int pre_area = A_R_WHITE;
    int area;

    static int W_count = 0;

    // 走行エリアが変化しているかを判定
    f_change = 0;
    change_mod = pre_change_mod;
    area = pre_area;
    if (sensor > HighTH) { /* Right White(RW) or Left White(LF) */
        if (pre_sensor <= HighTH) { /* RE2RW or LE2LW */
            f_change = 1;
            if (pre_area == A_R_EDGE) {
                change_mod = RE2RW;
                area = A_R_WHITE;
                W_count = 0;
            } else {
                change_mod = LE2LW;
                area = A_L_WHITE;
                W_count = 0;
            }
        }
    } else if (sensor < LowTH) { /* B */
        if (pre_sensor >= LowTH) { /* RE2B or LE2B */
            f_change = 1;
            if (pre_area == A_R_EDGE) {
                change_mod = RE2B;
                area = A_BLACK;
            } else {
                change_mod = LE2B;
                area = A_BLACK;
            }
        }
    } else {  /* RE or LE */
        if (pre_sensor > HighTH) { /* RW2RE or LW2LE */
            f_change = 1;
            if (pre_area == A_R_WHITE) {
                change_mod = RW2RE;
                area = A_R_EDGE;
            } else {
                change_mod = LW2LE;
                area = A_L_EDGE;
            }
         } else { /* B2RE or B2LE */
             if (pre_change_mod != B2RE) {
                 f_change = 1;
                 change_mod = B2RE;
                 area = A_R_EDGE;
             }
        }
    }

    // 連続してWhiteエリアを走行している場合、ラインを見失っている
    if ((area == A_R_WHITE) || (area == A_L_WHITE) || (area == A_INIT_WHITE))         
    {
        if (f_change == 0) {
            W_count++;
            if (W_count > 200) {
            /* Lost line, Change INIT mode */
                W_count = 0;
                area = A_INIT_WHITE;
                change_mod = IDOLMOD;
                f_change = 1;
            }
        }
    }

    if (f_change == 0) {
        pre_area = area;
        pre_sensor = sensor;
        pre_change_mod = change_mod;
        return 0;
    }

    // 走行エリアに変化が合った場合、進行方向を決める
    switch(change_mod) {
    case RW2RE:
        D_mode = D_STRAIGHT;
        break;
    case RE2B:
        D_mode = D_RIGHT;
        break;
    case B2RE:
        D_mode = D_STRAIGHT;
        break;
    case RE2RW:
        D_mode = D_LEFT;
        break;
    case LW2LE:
        D_mode = D_RIGHT;
        break;
    case LE2B:
        D_mode = D_RIGHT;
        break;
    case B2LE:
        D_mode = D_STRAIGHT;
        break;
    case LE2LW:
        D_mode = D_RIGHT;
        break;
    case INIT2RW:
        D_mode = D_LEFT;
        break;
    case IDOLMOD:
    default:
        D_mode = D_STRAIGHT;
        break;
    }

    // 進行方向によりモーターのスピードを設定
    switch(D_mode) {
    case D_RIGHT:
        Rmotor = SPEED_1;
        Lmotor = SPEED_3;
        break;
    case D_LEFT:
        Rmotor = SPEED_3;
        Lmotor = SPEED_1;
        break;
    case D_STRAIGHT:
    default:
        Rmotor = SPEED_D;
        Lmotor = SPEED_D;
        break;
    }

    // モーター制御
    if (Rmotor != pre_Rmotor) {
        pre_Rmotor = Rmotor;
        MotorPower(RMOT_CH, Rmotor);
    }
    if (Lmotor != pre_Lmotor) {
        pre_Lmotor = Lmotor;
        MotorPower(LMOT_CH, Lmotor);
    }

    pre_Rmotor = Rmotor;
    pre_Lmotor = Lmotor;
    pre_area = area;
    pre_sensor = sensor;
    pre_change_mod = change_mod;

    return 1;
}

unsigned char GetSensor(unsigned char ch)  //  カラーセンサー値の取得
{
    return((unsigned char)pUart->Raw[ch][pUart->Actual[ch]][0]);
}

void InitSensorPram(unsigned char threshold)
{
    if (threshold > 15) {
        HighTH = threshold + 10;
        LowTH = threshold - 10;
    } else {
        HighTH = 20;
        LowTH = 10;
    }
}

int main(int argc, char *argv[])
{
    int i;
    unsigned char sensor;

    if (argc != 2) {
        printf("Require the threshold value for sensor.\n");
        printf("Usage: line [Threshold]\n");
        exit(1);
    }
 
    InitSensorPram(atoi(argv[1]));

    // Open PWM device file
    pwmfp = open("/dev/lms_pwm",O_RDWR | O_SYNC );
    if (pwmfp < 0) {
            printf("Cannot open /dev/lms_pwm\n");
            exit(-1);
    }

    // Open UART device file
    uartfp = open("/dev/lms_uart", O_RDWR | O_SYNC);
    if (uartfp < 0) {
        printf("Cannot open UART_DEVICE\n");
        exit(-1);
    }

    // mmap UART device
    pUart  =  (UART*)mmap(0, sizeof(UART),
                  PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, uartfp, 0);
    if (pUart == MAP_FAILED) {
        printf("Failed to map device\n");
        exit(-1);
    }

    MotorReset(CH_A | CH_B | CH_C | CH_D);

    MotorStart();
    sleep(1);

    // Line trace control (around 60 sec)
    for (i = 0; i<6000; i++) { /* around 60 sec */
        sensor = GetSensor(SENS_CH);
        LineTrace(sensor);
        usleep(10000); /* 10mSec */
    }

    MotorPower(CH_A|CH_B|CH_C|CH_D, 0);
    MotorStop(CH_A|CH_B|CH_C|CH_D);

    munmap(pUart, sizeof(UART));
    close(pwmfp);
    close(uartfp);
    exit(0);
}
