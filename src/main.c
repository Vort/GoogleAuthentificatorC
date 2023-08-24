#include <stdio.h>
#include <time.h>
#ifdef WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif
#include "totp.h"

int key_pressed()
{
#ifdef WIN32
    if (_kbhit())
    {
        _getch();
        return 1;
    }
    else
        return 0;
#else
    int i;
    char ch;
    struct termios term;
    struct termios term2;
    tcgetattr(0, &term);

    term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    if (byteswaiting > 0)
    {
        for (i = 0; i < byteswaiting; i++)
            if (read(0, &ch, 1) < 0)
                perror ("read()");
        return 1;
    }
    else
        return 0;
#endif
}

void sleep_ms(int milliseconds)
{
#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

int main(int argc, char **argv)
{
    int i;
    time_t now;
    int prevLeft = -1;
    int left;
    int eraseCount;
    char code[7];

    if (argc != 2)
    {
        printf("./ga [secret]\n");
        return 0;
    }

    for (;;)
    {
        now = time(NULL);
        left = (int)(30 - now % 30);
        if (prevLeft != -1)
        {
            eraseCount = prevLeft < left ? 11 : 3;
            for (i = 0; i < eraseCount; i++)
                putchar('\b');
        }
        if (prevLeft < left)
        {
            get_2fa_code(argv[1], code);
            printf("%s  ", code);
        }
        prevLeft = left;
        printf("%2ds", left);
        fflush(stdout);
        sleep_ms(250);
        if (key_pressed())
        {
            putchar('\n');
            break;
        }
    }
    return 0;
}
