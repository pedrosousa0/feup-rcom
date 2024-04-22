/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
volatile int state=0;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[255];

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");


    int state=0;

    while (STOP==FALSE) {       /* loop for input */
        res = read(fd,buf,1);
        if (res>0){   /* returns after 5 chars have been input */
            buf[res]=0;               /* so we can printf... */
            printf(":%02x:%d\n", buf[0], res);
            char test[2]={0x03,0x08};
            switch(state){
                case 0:
                {   
                    printf("%d\n",state);
                    if (buf[0]==0x5c)
                    state++;
                    
                    break;
                }
                case 1:
                {
                    printf("%d\n",state);
                    if (buf[0]==0x03)
                    state++;
                    else if (buf[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 2:
                {
                    printf("%d\n",state);
                    if (buf[0]==0x08)
                    state++;
                    else if (buf[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 3:
                {
                    printf("%d\n",state);
                    if (buf[0]==(test[0]^test[1]))
                    state++;
                    else if(buf[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 4:
                {
                    printf("%d\n",state);
                    if (buf[0]==0x5c)
                    STOP=TRUE;
                    else
                    state=0;
                    break;
                }
                
            }
        }
        
    }

    char buf1[5]= {0x5c,0x03,0x08,0x00,0x5c};
    buf1[3] = buf1[1]^buf1[2];

    res = write(fd,buf1,5);
    sleep(1);
    printf("%d bytes written\n", res);


    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
