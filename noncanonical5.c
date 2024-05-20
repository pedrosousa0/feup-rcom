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
        exit(-1);res = read(fd,buf,1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

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
    /*
    while (STOP==FALSE) {       
        res = read(fd,buf,1);
        if (res>0){   
            buf[res]=0;              
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
    
    res = write(fd,buf,5);
    sleep(1);
    printf("%d bytes written\n", res);
    */
    unsigned char buf2[255];
    unsigned char buf3[255];
    unsigned char buf4[255];
    unsigned char buf5[255];
    STOP=FALSE;
    state=0;
    int bcc3true=FALSE;
    int contador=0;
    //while (bcctest == FALSE){}
    while (STOP==FALSE) {       /* loop for input */
        if(bcc3true==FALSE)
        res = read(fd,buf2,1);
        else
        res = read(fd,buf3,1);
        if (res>0 && bcc3true==TRUE){
            buf3[res]=0;
            printf(":%02x:%d\n", buf3[0], res);
            contador++;
        }
        if (res>0){   /* returns after 5 chars have been input */
            buf2[res]=0;
            printf(":%02x:%d\n", buf2[0], res);
            char test[2];
            switch(state){
                case 0:
                {   
                    printf("%d\n",state);
                    if (buf2[0]==0x5c)
                    state++;
                }

                case 1:
                {
                    printf("%d\n",state);
                    if (buf2[0]==0x03){
                    state++;
                    test[0]=0x03;}
                    else if (buf2[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 2:
                {
                    printf("%d\n",state);
                    if (buf2[0]==0x80){
                    state++;
                    test[1]=0x80;}
                    else if (buf2[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 3:
                {
                    printf("%d\n",state);
                    if (buf2[0]==(test[0]^test[1])){
                    bcc3true=TRUE;
                    state++;}
                    else if(buf2[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 4:
                {
                    printf("%d\n",state);
                    if (buf3[0]==0x5c)
                    STOP=TRUE;              
                }
            }
        
        }
    } 
    
    /*
    STOP=FALSE;
    while (STOP==FALSE) {       
        res = read(fd,buf3,1);
        if (res>0){   
            buf3[res]=0;
            contador++;           
            printf(":%02x:%d\n", buf3[0], res);
            if (buf3[0]==0x5c)
            STOP=TRUE;
        }        
    }
    */
    int contador2=contador;
    for (int i=1;i<contador2+1;i++){
        for (int j=0;j<contador;j++){
            if ((buf3[i-1]=0x5d)&&(buf3[i]=0x7c)){
                buf4[j]=0x5c;
                i++;
                contador--;
            }
            else if ((buf3[i-1]=0x5d)&&(buf3[i]=0x7d)){
                buf4[j]=0x5d;
                i++;
                contador--;
            }
            else{
                buf4[j]=buf3[i-1];
            }
        }
    }
    int bcc=buf4[0];
    
    for (int j=1;j<contador;j++){
        bcc=bcc^buf4[j];
    }

    if (buf4[contador]==bcc){
        char buf5[5] = {0x5c,0x03,0x11,0x00,0x5c};
        buf5[3] = buf5[1]^buf5[2];
    }
    else{
        char buf5[5] = {0x5c,0x03,0x15,0x00,0x5c};
        buf5[3] = buf5[1]^buf5[2];
    }
    res = write(fd,buf5,5);
    sleep(1);
    printf("%d bytes written\n", res);



    /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
    */

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
