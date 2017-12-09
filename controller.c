#include <stdio.h>
#include <mraa/i2c.h>
#include "lib/LSM9DS0.h"
#include <math.h>

// For socket communication
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

float calculate_magnitude(data_t data)
{
	return sqrt(data.x*data.x+data.y*data.y+data.z*data.z);
}

// send data to
void connect_send(char* data)
{
    char host_ip[] = "127.0.0.1"; // local host
    int portno = 6666;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket_fd < 0) {
		error("ERROR opening socket");
	}
	server = gethostbyname(host_ip);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	// clear our the serv_addr buffer
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	// set up the socket
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	// try to connect to the server
	if (connect(client_socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("ERROR connecting");
	}

	// send user input to the server
	int n = write(client_socket_fd, data, strlen(data));
	// n contains how many bytes were received by the server
	// if n is less than 0, then there was an error
	if (n < 0) {
		error("ERROR writing to socket");
	}

	// clear out the buffer
	memset(data, 0, 256);

    // clean up the file descriptors
	close(client_socket_fd);
}

int main() {
	mraa_i2c_context accel;
	float a_res;
	accel_scale_t a_scale;
        data_t ad;//Accel Data, Gyro Data, Mag Data
	data_t Go;//Gyro Offset
	int fb, rl;

	a_scale=A_SCALE_2G;

	accel = accel_init();
	set_accel_scale(accel, a_scale);
	set_accel_ODR(accel, A_ODR_100);
	a_res=calc_accel_res(a_scale);

	while(1){
	    ad = read_accel(accel, a_res);

	    fb = rl = 0;

	    if(ad.z > -0.6)
	    {
		    if(ad.y > 0)
			    fb = 1;
		    else
			    fb = -1;
	    }

	    if(ad.x > 0.5)
		    rl = -1;
	    if(ad.x < -0.5)
		    rl = 1;

	    printf("%d, %d \n", fb, rl);

	    usleep(100000);
	    char data[256];
	    sprintf(data, "movement: %d %d", fb, rl);
	    connect_send(data);
	}
	return 0;
}
