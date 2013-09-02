#define DEVICE_PATH "/dev/ttyACM0"

#define ARDUINO_BUF 1000
#define TIMEOUT 500
#define INTERVAL 5000

#define INIT_BUF(buffer, size) ( memset(buffer, '\0', size) )
#define CLR_BUF(buffer) ( memset(buffer, '\0', strlen(buffer)) )

struct threadData {
	int fd;
	int run;
};

int init_arduino(struct termios *config, const char *path);
void *poll_arduino(void *data);
