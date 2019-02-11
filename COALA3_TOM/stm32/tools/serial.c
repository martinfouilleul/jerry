/*
    COALA 2
    Programme de test de communication entre Beaglebone et MCU de la carte COALA 2
    Le premier paramètre de la ligne de commande est envoyé tel quel dans /dev/ttyO1
    Ensuite on attend une réponse qui se termine par \n et on l'affiche 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 64
void init_comm(struct termios *pts)
{
    /* some things we want to set arbitrarily */
    pts->c_lflag &= ~ICANON;
    pts->c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
    pts->c_cflag |= HUPCL;
    pts->c_cc[VMIN] = 1;
    pts->c_cc[VTIME] = 0;
    /* Standard CR/LF handling: this is a dumb terminal.
    * Do no translation:
    * no NL -> CR/NL mapping on output, and
    * no CR -> NL mapping on input.
    */
    pts->c_oflag &= ~ONLCR; /* set NO CR/NL mapping on output */
    /*pts->c_oflag |= ONLCR;
    crnl_mapping = 1;*/
    pts->c_iflag &= ~ICRNL; /* set NO CR/NL mapping on input */
    /* set no flow control by default */
    pts->c_cflag &= ~CRTSCTS;
    pts->c_iflag &= ~(IXON | IXOFF | IXANY);
    /* set hardware flow control by default */
    /*pts->c_cflag |= CRTSCTS;
    pts->c_iflag &= ~(IXON | IXOFF | IXANY);*/
    /* set 115200 bps speed by default */
    cfsetospeed(pts, B115200);
    cfsetispeed(pts, B115200);
}

int main(int argc, char **argv)
{
    struct termios newtio;   // structures de configuration du port serie
    uint8_t buf[BUFFER_SIZE];
    uint8_t rxcar;
    uint32_t idx = 0;
    /*---- Ouverture et configuration du port serie -----*/
	int serial_fd=open("/dev/ttyO1", O_RDWR); // | O_NOCTTY | O_NDELAY);
	if(serial_fd < 0)	{
		perror("opening serial device\n");
        exit(1);
	}
	else
		printf("/dev/ttyO1 open\n");
    /* modify the port configuration */
    tcgetattr(serial_fd, &newtio);
    init_comm(&newtio);
    tcsetattr(serial_fd, TCSANOW, &newtio);
    if (write(serial_fd, argv[1], strlen(argv[1])) <= 0){
        printf("Aucune demande envoyée\n");
        exit(0);
    }
    printf("%s envoyé\n", argv[1]);
    
	while(1)
	{
		int res=read(serial_fd,&rxcar,1);
        if(res <= 0){
            continue;
        }
        if(rxcar == '\n'){
            buf[idx] = rxcar;
            break;
        }
        printf("%c", rxcar);
        buf[idx] = rxcar;
        if(idx < BUFFER_SIZE -1)
            idx++;
    }
    printf("%s", buf);
    
    close(serial_fd);
    return 0;
}
 
