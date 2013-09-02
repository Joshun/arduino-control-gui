#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>

#include "arduino.h"


int init_arduino(struct termios *config, const char *path);
void *poll_arduino(void *data);

//int main()
//{
	//char *buffer = malloc(ARDUINO_BUF);
	
	//struct termios config;
	//int fd = init_arduino(&config, DEVICE_PATH);
	
	//struct pollfd fds;
	//int bytes;
	//fds.fd = fd;
	//fds.events = POLLIN | POLLPRI;
	//int i;
	
	//INIT_BUF(buffer);
	
	//while(1) {
		//usleep(INTERVAL);
		//if( poll(&fds, 1, TIMEOUT) < 1 )
		//{
			//printf("Connection timed out.\n");
			//exit(1);
		//}
		
		//if( fds.revents & POLLIN || fds.revents & POLLPRI) {
			////printf("Receiving data...\n");
			//ioctl(fd, FIONREAD, &bytes);
			//if( bytes > sizeof(buffer) )
			//{
				//char *new_p = realloc(buffer, bytes + 1);
				//if( new_p == NULL )
				//{
					//printf("Error: Out of memory\n");
					//exit(EXIT_FAILURE);
				//}
				//else
				//{
					//buffer = new_p;
					//INIT_BUF(buffer);
				//}
			//}
			//read(fd, buffer, bytes);
			//printf("%s\n", buffer);
			
			//CLR_BUF(buffer);		
			
		//}
	//}
	
		
	//return 0;
//}


int init_arduino(struct termios *config, const char *path)
{
	int fd = open(path, O_RDWR | O_NOCTTY);
	
	if( fd < 0 )
	{
		printf("Could not connect to Arduino.\n");
		perror("Error");
		exit(EXIT_FAILURE);
	}
	
	tcgetattr(fd, config);
	
	cfsetispeed(config, B9600);
	cfsetospeed(config, B9600);
	/* 8 bits, no parity, no stop bits */
	config->c_cflag &= ~PARENB;
	config->c_cflag &= ~CSTOPB;
	config->c_cflag &= ~CSIZE;
	config->c_cflag |= CS8;
	/* no hardware flow control */
	config->c_cflag &= ~CRTSCTS;
	/* enable receiver, ignore status lines */
	config->c_cflag |= CREAD | CLOCAL;
	/* disable input/output flow control, disable restart chars */
	config->c_iflag &= ~(IXON | IXOFF | IXANY);
	/* disable disable echo,
	disable visually erase chars,
	disable terminal-generated signals */
	config->c_iflag &= ~(ECHO | ECHOE | ISIG);
	/* disable output processing */
	config->c_oflag &= ~OPOST;
	
	/* enable canonical input */
	config->c_lflag |= ICANON;
	
	/* wait for 24 characters to come in before read returns */
	config->c_cc[VMIN] = 12;
	/* no minimum time to wait before read returns */
	config->c_cc[VTIME] = 0;
	 
	/* commit the options */
	tcsetattr(fd, TCSANOW, config);
	printf("Arduino setup completed.\n");
	return fd;
}

void *poll_arduino(void *data)
{
	struct threadData *config = (struct threadData*)data;
	char *buffer = malloc(ARDUINO_BUF);
	
	struct pollfd fds;
	int bytes;
	fds.fd = config->fd;
	fds.events = POLLIN | POLLPRI;
	
	int size = ARDUINO_BUF;
	INIT_BUF(buffer, size);
	
	printf("Using fd %d\n", config->fd);
	//write(config->fd, "\n", strlen("\n"));
	
	while(config->run) {
		//usleep(INTERVAL);
		if( poll(&fds, 1, TIMEOUT) < 1 )
		{
			//printf("Connection timed out.\n");
			//exit(1);
		}
		if( fds.revents & POLLIN || fds.revents & POLLPRI) {
			//printf("Receiving data...\n");
			ioctl(config->fd, FIONREAD, &bytes);
			if( bytes > size )
			{
				printf("Allocating more memory...\n");
				char *new_p = realloc(buffer, bytes + 1);
				if( new_p == NULL )
				{
					printf("Error: Out of memory\n");
					exit(EXIT_FAILURE);
				}
				else
				{
					buffer = new_p;
					size = bytes + 1;
					INIT_BUF(buffer, size);
				}
			}
			read(config->fd, buffer, bytes);
			printf("%s\n", buffer);
			
			CLR_BUF(buffer);		
			
		}
	}
	printf("Ending thread...\n");
	free(buffer);
}
		
