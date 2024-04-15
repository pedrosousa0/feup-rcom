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
    char buf[255];

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

    int flagreceive(){
        if(buf[0]=={0x5c})
        return 1;
        else
        return 0;
    }
    int areceive(){
        if(buf[0]=={0x03})
        return 1;
        else
        return 0;
    }
    int creceive(){
        if(buf[0]=={0x08})
        return 1;
        else
        return 0;
    }
    int bccok(){
        char test[2]={0x03,0x08};
        if(buf[0]==(test[0]^test[1]))
        return 1;
        else
        return 0;
    }

    while (STOP==FALSE) {       /* loop for input */
        res = read(fd,buf,1);
        if (res>0){   /* returns after 5 chars have been input */
            buf[res]=0;               /* so we can printf... */
            printf(":%s:%d\n", buf, res);
            switch(state){
                case 0:
                {
                    if (flagreceive)
                    state++;
                    break;
                }
                case 1:
                {
                    if (areceive)
                    state++;
                    else
                    state=0;
                    break;
                }
                case 2:
                {
                    if (flagreceive)
                    state=1;
                    else if (creceive)
                    state++;
                    else
                    state=0;
                    break;
                }
                case 3:
                {
                    if (flagreceive)
                    state=1;
                    else if(bccok)
                    state++;
                    else
                    state=0;
                    break;
                }
                case 4:
                {
                    if (flagreceive)
                    state++;
                    else
                    state=0;
                    break;
                }
                case 5:
                {
                STOP=TRUE;
                }

            }
        }
        
    }



    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
