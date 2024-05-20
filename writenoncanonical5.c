/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[255];
    int i, sum = 0, speed = 0;
    int control=0;
    int UA=1;
    int disc=0;
    int resend=0;
    
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

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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


    
   //for (i = 0; i < 255; i++) {
   //    buf[i] = 'a';
   // }
    unsigned char buf1[5]= {0x5c,0x02,0x03,0x04,0x09};
    unsigned char buf2[6];
    unsigned char buf3[12];
    //bcc2//    
    
    for(i=0;i<4;i++)
        {
        buf2[i]=buf1[i];
        if(i==0)
            buf2[5]=buf1[0];
        else
            buf2[5]=buf2[5]^buf1[i];
        }

    //bytestuffing//
    int j=0;    
    for(i=0;i<5;i++)
        {
                
            {
            if (buf2[i]==0x5c)
                {                
                buf3[j]= 0x5d;
                buf3[j+1]= 0x7c;
                j++;
                }
            else if (buf2[i]==0x5d)
                {                
                buf3[j+1]= 0x7d;
                j++;
                }
            else
                buf3[j]=buf2[i];
            }
        j++;
        }
            
    //buffer byte stuffed com bcc2//
    unsigned char buf4[17];
    //flaginicial//
        buf4[0]=0x5c;
    //address//
        buf4[1]=0x03;
    //control//
        buf4[2]=0x80;
    //bcc1//
        buf4[3]=buf4[1]^buf4[2];
    //data//        
        for(i=4;i<15;i++)
            {
            for(int k=0;k<j;k++)
                {
                buf4[i]=buf3[k];
                }
            }
    //flagfinal//
        buf4[j+5]=0x5c;
    
    /*testing*/
    

    //res = write(fd,buf,255);
    res = write(fd,buf4,17);
    sleep(1);
    printf("%d bytes written\n", res);


    /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar
    o indicado no guião
    */


int state=0;


while (STOP==FALSE) {       /* loop for input */
        res = read(fd,buf,1);
        if (res>0){   /* returns after 5 chars have been input */
            buf[res]=0;               /* so we can printf... */
            printf(":%02x:%d\n", buf[0], res);
            char test[3];
            switch(state){
                case 0:
                {
                    if (buf[0]==0x5c)
                    {
                    state++;
                    test[0]=buf[0];
                    }
                    break;
                }
                case 1:
                {
                    if (buf[0]==0x03)
                    {
                    state++;
                    test[1]=buf[0];
                    }
                    else if (buf[0]==0x5c)
                    state=1;
                    else
                    state=0;
                    break;
                }
                case 2:
                {
                    if (buf[0]==0x5c)
                    state=1;
                    else if (buf[0]==0x06 && UA==1)
                    {
                    state++;
                    test[2]=buf[0];
                    }                        
                    /*else if (buf[0]==0x01 && control=0)
                    {                    
                    state++;
                    test[2]=buf[0];
                    } */
                    else if (buf[0]==0x11 /*&& control=1*/)
                    {                    
                    state++;
                    test[2]=buf[0];
                    }
                    /*else if (buf[0]==0x05 && control=0)
                    state++;
                    test[2]=buf[0];
                    resend=1;
                    */
                    else if (buf[0]==0x15 /*&& control=1*/)
                    {                    
                    state++;
                    test[2]=buf[0];
                    resend=1;
                    }
                    else if (buf[0]==0x0a)
                    {                    
                    state++;
                    test[2]=buf[0];
                    disc=1;
                    }               
                    else
                    state=0;
                    break;
                }
                case 3:
                {
                    if (buf[0]==0x5c)
                    state=1;
                    else if(buf[0]==(test[0]^test[1]^test[2]))
                    state++;
                    else
                    state=0;
                    break;
                }
                case 4:
                {
                    if (buf[0]==0x5c)
                    STOP=TRUE;
                    else
                    state=0;
                    break;
                }
                
                }

            }
        }
        
    

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
}
  
    close(fd);
    return 0;
}

