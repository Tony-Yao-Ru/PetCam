#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include "motor_control.h"

#define PORT 8080
#define MAXCLIENTS 10
#define HEIGHT_IMG 480
#define WIDTH_IMG 640
#define HTML_FILE "View/index.html"

struct cambuffer {
	void   *start;
	size_t length;
};


int cam_fd;
struct cambuffer cam_buf;
pthread_mutex_t cam_mutex = PTHREAD_MUTEX_INITIALIZER;

struct gpiod_chip *g_chip = NULL;


// Print a descriptive error msg
void error(const char *msg) {
	perror(msg);
	exit(1);
}

int init_USBcamera() 
{
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer vbuf;	

	// To open a V4L2 device applications with the desired device name
	cam_fd = open("/dev/video0", O_RDWR);
	if (cam_fd < 0) {
		error("Error in opening the camera");
		return -1;
	}
	
	// To negotiate the format of data exchanged btw driver and application
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = WIDTH_IMG;
	fmt.fmt.pix.height = HEIGHT_IMG;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0) {
		error("Error in setting format");
	}
	
	// To initiate memory mapped buffer and to initialize the request buffer
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (ioctl(cam_fd, VIDIOC_REQBUFS, &req) < 0) {
		error("Error in requesting buffers");
		return -1;
	}

	// To exchange the data into the buffer
	vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;
	vbuf.index = 0;
	if (ioctl(cam_fd, VIDIOC_QUERYBUF, &vbuf) < 0) {
		error("Error in querying buffer");
		return -1;
	}

	cam_buf.length = vbuf.length;
	cam_buf.start = mmap(NULL, vbuf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_fd, vbuf.m.offset);
	if (cam_buf.start == MAP_FAILED) {
		error("Error in mmap");
		return -1;
	}
	return 0;

}

int capture_frame(unsigned char **data, int *length) {
    struct v4l2_buffer vbuf;

    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vbuf.memory = V4L2_MEMORY_MMAP;
    vbuf.index = 0;

    if (ioctl(cam_fd, VIDIOC_QBUF, &vbuf) < 0) return -1;
    if (ioctl(cam_fd, VIDIOC_STREAMON, &vbuf.type) < 0) return -1;       // Tell the camera to start streaming
    
    // Wait for I/O operation
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(cam_fd, &fds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    if (select(cam_fd + 1, &fds, NULL, NULL, &tv) < 0) return -1;        // Wait for the camera becomes ready for a read operation
									 // select() system call can send the signal when it is ready
									 // to start saving the file. 

    if (ioctl(cam_fd, VIDIOC_DQBUF, &vbuf) < 0) return -1;

    *data = cam_buf.start;
    *length = vbuf.bytesused;

    return 0;
}

void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char header[1024];
    snprintf(header, sizeof(header),
             "HTTP/1.0 200 OK\r\n"
             "Server: MJPEGStreamer\r\n"
             "Connection: close\r\n"
             "Max-Age: 0\r\n"
             "Expires: 0\r\n"
             "Cache-Control: no-cache, private\r\n"
             "Pragma: no-cache\r\n"
             "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n"); // multipart/x-mixed-replace is an HTTP content type
										 // Your server can use it to push dynamically updated content 
										 // to the web browser. It works by telling the browser to 
										 // keep the connection open and replace the web page or 
										 // piece of media it is displaying with another 
										 // when it receives a special token.
    send(client_fd, header, strlen(header), 0);

    for (;;) {
        unsigned char *frame;
        int len;

        pthread_mutex_lock(&cam_mutex);
        if (capture_frame(&frame, &len) != 0) {
            pthread_mutex_unlock(&cam_mutex);
            break;
        }
        pthread_mutex_unlock(&cam_mutex);

        char part_header[256];
        snprintf(part_header, sizeof(part_header),
                 "--frame\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %d\r\n\r\n", len);

        if (send(client_fd, part_header, strlen(part_header), 0) < 0) 
	{
		perror("Send header");
		break;
	}
        if (send(client_fd, frame, len, 0) < 0) 
	{
		perror("send frame");
		break;
	}
        if (send(client_fd, "\r\n", 2, 0) < 0) 
	{
		perror("send newline");
		break;
	}

        usleep(100000); // ~10 fps
    }

    close(client_fd);
    return NULL;
}

void serve_html(int client_fd) {
    FILE *f = fopen(HTML_FILE, "r");
    if (!f) {
        const char *msg = "HTTP/1.0 500 Internal Server Error\r\n\r\nError loading HTML\n";
        send(client_fd, msg, strlen(msg), 0);
        return;
    }

    const char *header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(client_fd, header, strlen(header), 0);

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        send(client_fd, line, strlen(line), 0);
    }

    fclose(f);
}



int main() 
{
	int sockfd, *newsockfd;
	int opt = 1;
	char rcvData[1024] = {0};
	struct sockaddr_in serv_addr = {0};	 // struct sockaddr_in {
						 // 	short 	sin_family; /* must be AF_INET*/
						 // 	u_short sin_port;
						 // 	struct	in_addr sin_addr;
						 // 	char	sin_zero[8]; /*Not used, must be zero*/

	if (init_USBcamera() < 0) {
		error("Camera init failed\n");
		return -1;
	}
	
	g_chip = gpiod_chip_open_by_name(CHIPNAME);
	if (!g_chip) {
		error("Failed to open GPIO chip");
		return -1;
	}

	signal(SIGPIPE, SIG_IGN);

	/*(a) Create socket for incoming request */

	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Socket(): system call creates a new socket
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
    	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);	  // Convert the unsigned short integer to network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	/*(b) Listen to any clients trying to connect to the port */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // bind(): system call binds a socket to an address 
		error("ERROR on binding()");
	}
	listen(sockfd, MAXCLIENTS); 		  // listen(): system call allows the process to listen on the socket for connections.

	printf("Visit http://192.168.1.219:8080/");
	
	while (1) {
		newsockfd = (int *) malloc(sizeof(int));
		*newsockfd = accept(sockfd, NULL, NULL);
                     
		if (*newsockfd < 0) continue;
	
		recv(*newsockfd, rcvData, sizeof(rcvData)-1, 0);
		rcvData[sizeof(rcvData)-1] = '\0';  // null-terminate
		//printf("Received data:\n%s\n", rcvData);
        /*(c) Distinguish the request from the client */
		if (strncmp(rcvData, "GET /stream", 11) == 0) {
    			pthread_t tid;
    			pthread_create(&tid, NULL, client_thread, newsockfd);
    			pthread_detach(tid);
		} else if (strncmp(rcvData, "GET /control?dir=", 17) == 0) {
			char *dir_start = rcvData + 17;
			char direction[16] = {0};
			sscanf(dir_start, "%15[^ ]", direction);
			
			printf("Received control command: %s\n", direction);
			
			
    			if (strcmp(direction, "left") == 0) {
				onClickedLeft(g_chip);
    			} else if (strcmp(direction, "right") == 0) {
        			onClickedRight(g_chip);
    			} else if (strcmp(direction, "go") == 0) {
        			onClickedForward(g_chip);
    			} else if (strcmp(direction, "back") == 0) {
        			onClickedBackward(g_chip);
    			}
			

		        const char *response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nCommand received\n";
			send(*newsockfd, response, strlen(response), 0);
			close(*newsockfd);
		        free(newsockfd);			
		} else {
    			serve_html(*newsockfd);
    			close(*newsockfd);
    			free(newsockfd);
		}	

	}

	close(sockfd);
	return 0;
}
