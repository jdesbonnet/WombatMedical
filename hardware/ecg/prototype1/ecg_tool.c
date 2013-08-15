/**
 * ecg_tool.c- a command line utility to communicate with the
 * wearable ECG prototype
 *
 * Author: Joe Desbonnet, jdesbonnet@gmail.com
 * 
 *
 * Version 0.1 (20 April 2013)
 * void read_n_esc_bytes (int fd, uint8_t *buf, int length) {
 * To compile:
 * gcc -o ecg_tool ecg_tool.c -lrt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>


#define APP_NAME "ecg_tool"
#define VERSION "0.1, 20 Apr 2013"

#define TRUE 1
#define FALSE 0

#define STREAM_START (0x12)
#define STREAM_ESCAPE (0x7D)

#define REG_ID (0x00)
#define REG_CONFIG1 (0x01)
#define REG_CONFIG2 (0x02)
#define REG_CH1SET (0x04)
#define REG_CH2SET (0x05)


// ADS1292R registers
// RegAddr RegName: Bit7 Bit6 .. Bit0 [value on reset]
// 0x00 ID: REV_ID7 REV_ID6 REV_ID5 1 0 0 REV_ID1 REV_ID0 [factory programmed]
// 0x01 CONFIG1: SINGLE-SHOT 0 0 0 0 DR2 DR1 DR0 [0x02 on reset]
// 0x02 CONFIG2: 1 PBD_LOFF_COMP PDB_REFBUF VREF_4V CLK_EN 0 INT_TEST TEST_FREQ [0x80 on reset]
// 0x03 LOFF: COMP_TH2 COMP_TH1 COMP_TH0 1 ILEAD_OFF1 ILEAD_OFF0 0 FLEAD_OFF [0x10 on reset]
// 0x04 CH1SET: PD1 GAIN1_2 GAIN1_1 GAIN1_0 MUX1_3 MUX1_2 MUX1_1 MUX1_0 [0x00]
// 0x05 CH1SET: PD2 GAIN2_2 GAIN2_1 GAIN2_0 MUX2_3 MUX2_2 MUX2_1 MUX2_0 [0x00]

// The debug level set with the -d command line switch
int debug_level = 0;

// Use -q flag to enable quiet mode. Warning messages will be suppressed.
int quiet_mode = FALSE;

// Set to true in signal_handler to signal exit from main loop
int exit_flag = FALSE;

void debug (int level, const char *msg, ...);
void warning (const char *msg, ...);

uint8_t cmd_buf[256];

typedef struct {
	uint16_t status;
	int32_t ch1;
	int32_t ch2;
} ecg_record_t;

/**
 * Open serial IO device to ADS1292R EVM 
 * (8N1, 57600bps, raw mode, no handshaking)
 *
 * @param deviceName Pointer to string with device name (eg "/dev/ttyACM0")
 * @return Operating system file descriptor or -1 if there was an error.
 */
int ecg_open(char *deviceName, int bps) {

	int fd = open(deviceName,O_RDWR);
	if (fd==0) {
		fprintf (stderr,"Error: unable to open device %s\n",deviceName);
		return -1;
	} 

	struct termios tios;
	int status=tcgetattr(fd,&tios);
	if (status < 0) {
    	fprintf (stderr,"Error: error calling tcgetattr\n");
		return -1;
	}

	int speed = B9600;
	switch (bps) {
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;
			break;
		case 38400:
			speed = B38400;
			break;
		case 57600:
			speed = B57600;
			break;
		case 115200:
			speed = B115200;
			break;
		default:
			fprintf (stderr,"Unsupported speed %d bps\n", bps);
	}

	// Set tx/rx speed at 115200bps, and set raw mode
 	cfsetispeed(&tios,speed);
 	cfsetospeed(&tios,speed);
	cfmakeraw(&tios);

	tios.c_cflag &= ~CSTOPB; // Set 1 stop bit
	tios.c_cflag |= (CREAD | CLOCAL); // Enable receiver and disable hardware flow control
	tios.c_oflag = 0; // Disable some modem settings


	//tios.c_cc[VMIN] = 1; 
	//tios.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &tios);

	return fd;
}

/**
 * Close serial IO device.
 * @param fd File descriptor
 */
void ecg_close(int fd) {
	close(fd);
}


/**
 * Display to stderr current version of this application.
 */
void version () {
	fprintf (stderr,"%s, version %s\n", APP_NAME, VERSION);
}

/**
 * Display help and usage information. 
 */
void usage () {
	fprintf (stderr,"\n");
	fprintf (stderr,"Usage: ecg_tool [-q] [-v] [-h] [-d level] device\n");

	//fprintf (stderr,"  -c channel \t Set channel. Allowed values: 11 to 26.\n");	
	fprintf (stderr,"\n");
	fprintf (stderr,"Options:\n");
	fprintf (stderr,"  -d level \t Set debug level, 0 = min (default), 9 = max verbosity\n");
	fprintf (stderr,"  -n nsample \t Number of ECG samples\n");
	fprintf (stderr,"  -q \t Quiet mode: suppress warning messages.\n");
	fprintf (stderr,"  -v \t Print version to stderr and exit\n");
	fprintf (stderr,"  -h \t Display this message to stderr and exit\n");
	fprintf (stderr,"\n");
	fprintf (stderr,"Parameters:\n");
	fprintf (stderr,"  device:  the unix device file corresponding to the dongle device (often /dev/ttyACM0)\n");
	fprintf (stderr,"\n");
	//fprintf (stderr,"See this blog post for details: \n    http://jdesbonnet.blogspot.com/2012/04/stm32w-rfckit-as-802154-network.html\n");
	fprintf (stderr,"Version: ");
	version();
	fprintf (stderr,"Author: Joe Desbonnet, jdesbonnet@gmail.com.\n");
	fprintf (stderr,"Copyright 2012. Source released under BSD licence.\n");
	fprintf (stderr,"\n");
}



/**
 * Display debug message if suitable log level is selected. 
 * Use vararg mechanism to allow use similar to the fprintf()
 * function.
 *
 * @param level Display this message if log level is greater
 * or equal this level. Otherwise ignore the message.
 * @param msg  Format string as described in fprintf()
 */
void debug (int level, const char* msg, ...) {
	if (level >= debug_level) {
		return;
	}
	va_list args;
	va_start(args, msg);		// args after 'msg' are unknown
	vfprintf(stderr, msg, args);
	fprintf(stderr,"\n");
	fflush(stderr);
	va_end(args);
}
/**
 * Display warning message if unless quiet_mode is enabled.
 * 
 * @param msg  Format string as described in fprintf()
 */
void warning (const char* msg, ...) {
	if (quiet_mode) {
		return;
	}
	fprintf(stderr,"WARNING: ");
	va_list args;
	va_start(args, msg);		// args after 'msg' are unknown
	vfprintf(stderr, msg, args);
	fprintf(stderr,"\n");
	fflush(stderr);
	va_end(args);
}

/**
 * Signal handler for handling SIGPIPE and...
 */
void signal_handler(int signum, siginfo_t *info, void *ptr) {
	debug (1, "Received signal %d originating from PID %lu\n", signum, (unsigned long)info->si_pid);
	//exit(EXIT_SUCCESS);
	exit_flag = TRUE;
}

/**
 * Continue to read from file handle fd until 'length' bytes
 * have been read.
 */
void read_n_bytes (int fd, void *buf, int length) {
	//fprintf (stderr,"read_n_bytes length=%d\n",length);
	int n=0;
	while (n < length) {
		n += read(fd,buf+n,length-n);
	}
}

/**
 * Read n data bytes from an escaped stream.
 */
void read_n_esc_bytes (int fd, uint8_t *buf, int length) {
	int i;
	for (i = 0; i < length; i++) {
		read_n_bytes(fd,buf+i,1);
		if (buf[i]==STREAM_ESCAPE) {
			read_n_bytes(fd,buf+i,1);
			buf[i] ^= 0x20;
		}
	}
}

		
		

/**
 * Display length bytes from pointer buf in zero padded
 * hex.
 */
void display_hex(uint8_t *buf, int length) {
	int i;
	for (i = 0; i < length; i++) {
		fprintf (stderr,"%02X ",buf[i]);
	}
}


/**
 *
 * @return The entire frame length (excluding cksum) if successful, -1 on error.
 */
void ecg_read_record (int fd, ecg_record_t *record) {
	
	int i;
	uint8_t c;
	uint8_t buf[8];

	debug (9, "ecg_read_record: wait for header");

	// Read up to start-of-header
	do {
		//fprintf (stderr,".");
		read_n_bytes (fd,&c,1);
		//fprintf (stderr,"%x ",c);
	} while (c != STREAM_START);

	debug (9, "ecg_read_record: data frame found");

	// Check frame type (expecting type 0x01)
	read_n_bytes (fd,&c,1);
	if (c != 0x01) {
		debug(1,"ecg_read_record: error: was expecting frame type 0x01, got 0x%x",c);
		return;
	}

	// Read ECG frame
	read_n_esc_bytes (fd,buf,7);

	record->status = buf[0];
	record->ch1 = (((buf[1]<<16) | (buf[2]<<8) | buf[3])<<8)/256;
	record->ch2 = (((buf[4]<<16) | (buf[5]<<8) | buf[6])<<8)/256;
}

void ecg_read_32bit (int fd, uint32_t *value) {

	int i;
	uint8_t c;
	uint8_t buf[4];

	// Read up to start-of-header
	do {
		//fprintf (stderr,".");
		read_n_bytes (fd,&c,1);
		//fprintf (stderr,"%x ",c);
	} while (c != STREAM_START);

	debug (9, "ecg_read_32bit: data frame found");

	// Check frame type (expecting type 0x02)
	read_n_bytes (fd,&c,1);
	if (c != 0x02) {
		debug(1,"ecg_read_32bit: error: was expecting frame type 0x02, got 0x%x",c);
		return;
	}

	// Read 32 bits
	read_n_esc_bytes (fd,buf,4);

	*value = (uint32_t) ((buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3]);
}


int ecg_register_read (int fd, int regId) {

	ecg_cmd_send(fd, "CMD SDATAC");
	ecg_cmd_send(fd, "SET MODE BINARY");

	char buf[32];
	sprintf (buf, "REGR %d\r\n", regId);

	ecg_cmd_send(fd, buf);

	uint32_t value;
	ecg_read_32bit (fd, &value);

	return value;

}
void ecg_register_write (int fd, int regId, int regValue) {
	char buf[32];
	sprintf (buf, "REGW %d %d\r\n", regId, regValue);
	ecg_cmd_send(fd, buf);
}

void ecg_display_record (ecg_record_t record) {
	double ch1 = (double)record.ch1 / (double)(1<<23);
	double ch2 = (double)record.ch2 / (double)(1<<23);
	fprintf (stdout,"%lf %lf\n", ch1, ch2 );
	//fprintf (stdout,"%lf\n", ch1 );
}
void ecg_display_temperature (ecg_record_t record) {
	int64_t uv = ((int64_t)record.ch1 * 2420000L) >> 23;
	int64_t t = (uv - 145300)/49 + 250;
	fprintf (stdout,"ch1=%x uv=%d t=%f\n", record.ch1, (int)uv, ((float)t)/10 );
}

/**
 * Write a command. Commands are:
 * WRITE_REG_COMMAND (0x91): register, value
 * READ_REG_COMMAND (0x92): register, 0x00
 * DATA_STREAMING_PACKET (0x93): on/off, 0x00  (0x00 = off, 0x01 = on)
 *
 * @return Always 0.
 */

int ecg_cmd_send (int fd, char *cmd) {
	//char eol[2] = {'\r','\n'};
	// find end of cmd string (0 terminated)
	char *i = cmd;
	while (*(i++) != 0) ;

	debug (1, "ecg_cmd_send, cmd=%s len=%d", cmd, (i-cmd) );

	write (fd, cmd, (i-cmd) );
	write (fd, "\r\n", 2 );

	tcdrain (fd);

	return 0;
}

int main( int argc, char **argv) {
	int i,v;
	int speed = 115200;
	int amp_gain = -1;
	int n_sample = 5000;
	int mux_setting = -1;

	char *device;
	char *command;

	int test_flag = 0;
	ecg_record_t record;

	char ecgcmd[32];

	// Setup signal handler. Catching SIGPIPE allows for exit when 
	// piping to Wireshark for live packet feed.
	//signal(SIGPIPE, signal_handler);
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = signal_handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGPIPE, &act, NULL);


	// Parse command line arguments. See usage() for details.
	int c;
	while ((c = getopt(argc, argv, "b:c:d:g:hm:n:qs:tv")) != -1) {
		switch(c) {
			case 'b':
				speed = atoi (optarg);
				break;
			case 'd':
				debug_level = atoi (optarg);
				break;
			
			case 'g':
				amp_gain = atoi (optarg);
				break;

			case 'm':
				mux_setting = atoi(optarg);
				break;

			case 'n':
				n_sample = atoi (optarg);
				break;

			case 'h':
				version();
				usage();
				exit(EXIT_SUCCESS);

			case 'q':
				quiet_mode = TRUE;
				break;

			case 't':
				test_flag = TRUE;
				break;

			case 'v':
				version();
				exit(EXIT_SUCCESS);
			case '?':	// case when a command line switch argument is missing
				/*
				if (optopt == 'c') {
					fprintf (stderr,"ERROR: 802.15.4 channel 11 to 26 must be specified with -c\n");
					exit(-1);
				}
				*/
				if (optopt == 'd') {
					fprintf (stderr,"ERROR: debug level 0 .. 9 must be specified with -d\n");
					exit(-1);
				}
				break;
		}
	}

	// One parameters are mandatory
	if (argc - optind < 1) {
		fprintf (stderr,"Error: missing command arguments. Use -h for help.");
		exit(EXIT_FAILURE);
	}

	device = argv[optind];
	command = argv[optind+1];

	if (debug_level > 0) {
		debug (1,"device=%s",device);
	}

	if (debug_level > 0) {
		fprintf (stderr,"DEBUG: debug level %d\n",debug_level);
	}
	
	// Open device
	int fd = ecg_open(device,speed);
	if (fd < 1 ) {
		fprintf (stderr,"Error: unable to open device %s\n", device);
		return EXIT_FAILURE;
	}


	// Ignore anything aleady in the buffer
	tcflush (fd,TCIFLUSH);


	
	// Reset ECG AFE and MCU
	//ecg_cmd_send(fd, "Z");
	//sleep(1);

	if (amp_gain > 0) {

	}

	if (command[0]=='t') {
		ecg_cmd_send(fd, "SET MODE BINARY");
		ecg_cmd_send(fd, "TEMP");
		int32_t temperature;
		ecg_read_32bit (fd, &temperature);
		fprintf (stdout, "%d\n", temperature);

	} else if (command[0]=='e') {

		// CONFIG2 = 0xA3
		//ecg_cmd_send(fd, "WWREG 0x2 0xA3");

		// CH2SET = 0x05	
		//ecg_cmd_send(fd, "WWREG 0x5 0x05");

		for (i = 0; i < 8; i++) {
			v = ecg_register_read(fd,i);
			fprintf (stderr,"REG[%d]=%x\n", i, v);
		}

		if (mux_setting >= 0) {
			v = ecg_register_read(fd,REG_CH1SET);
			fprintf (stderr,"CH1SET=%d\n", v);
			debug (9,"CH1SET=%d", v);
			ecg_register_write (fd,REG_CH1SET, v | (mux_setting & 0x0f));
			v = ecg_register_read(fd,REG_CH1SET);
			fprintf (stderr,"CH1SET=%d\n", v);
			debug (9,"CH1SET=%d", v);

			v = ecg_register_read(fd,REG_CH2SET);
			fprintf (stderr,"CH2SET=%d\n", v);
			debug (9,"CH2SET=%d", v);
			ecg_register_write (fd,REG_CH2SET, v | (mux_setting & 0x0f));
			v = ecg_register_read(fd,REG_CH2SET);
			fprintf (stderr,"CH2SET=%d\n", v);
			debug (9,"CH2SET=%d", v);

		}


		if (test_flag) {
			fprintf (stderr,"test on\n");
			ecg_cmd_send(fd, "SET TEST ON");
		}

		sprintf (ecgcmd, "ECGRN %d B", n_sample);
fprintf (stderr,"cmd=%s\n",ecgcmd);
		ecg_cmd_send(fd,ecgcmd);

		for (i = 0; i < n_sample; i++) {
			ecg_read_record (fd,&record);
			ecg_display_record (record);
			fflush(stdout);

if (i%100==0) {
	fprintf (stderr,"*");
	fflush (stderr);
}

		}

	}

	ecg_close(fd);

	debug (1, "Normal exit");
	return EXIT_SUCCESS; 
}
