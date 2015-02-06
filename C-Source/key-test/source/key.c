#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

typedef struct {
        unsigned char Pressed[6];
} KEYBUF;

int uifp;

int main(void)
{
        KEYBUF *KeyPt;
        int cnt;

        uifp = open("/dev/lms_ui",O_RDWR);
        if (uifp < 0) {
                printf("Cannot open /dev/lms_ui\n");
                exit(-1);
        }
        // 共有メモリをmmap()によりマッピングします
        KeyPt = (KEYBUF *)mmap(0, sizeof(KEYBUF),
                    PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, uifp, 0);
        if (KeyPt == MAP_FAILED) {
                printf("mmap failed\n");
                exit(-1);
        }

        for (cnt=0; cnt<30; cnt++) {
                printf("%d %d %d %d %d %d \r\n",
                        (*KeyPt).Pressed[0],
                        (*KeyPt).Pressed[1],
                        (*KeyPt).Pressed[2],
                        (*KeyPt).Pressed[3],
                        (*KeyPt).Pressed[4],
                        (*KeyPt).Pressed[5]);
                sleep(1);
        }

        munmap(KeyPt, sizeof(KEYBUF));
        close(uifp);
        exit(0);
}
