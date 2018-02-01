#ifndef LIBLIVE_H
#define LIBLIVE_H

typedef size_t (*f_get_data)(char*, size_t);

typedef struct {
	char *url;
	int width;
	int height;
	int bufferSize;
	int fd;
	int queueSize;
	unsigned short rtspPort;
	char *user;
	char *passwd;
	char *audioDev;
	int audioFreq;
	int audioNbChannels;
	char *audioFmt;
	char *murl;
	unsigned short rtspOverHTTPPort;
} MainParams;

void start_service(MainParams mps, f_get_data get_data);

#endif
