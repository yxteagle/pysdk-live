#ifndef CAPTURE_H
#define CAPTURE_H

#include "DeviceInterface.h"

typedef size_t (*f_get_frame)(char*, size_t);

class MyCapture: DeviceInterface {
public:
	MyCapture(int width, int height, int BufferSize, f_get_frame get_frame, int fd) {
		bufferSize = BufferSize;
		this->width = width;
		this->height = height;
		this->get_frame = get_frame;
		this->fd = fd;
	}
	~MyCapture() {
	}

	size_t read(char* buffer, size_t bufferSize) {
		return this->get_frame(buffer, bufferSize);
	}
	unsigned long getBufferSize() {
		return bufferSize;
	}
	int getWidth() {
		return width;
	}
	int getHeight() {
		return height;
	}
	int getFd(){
		return fd;
	}
private:
	unsigned long bufferSize;
	int width;
	int height;
	int fd;
	f_get_frame get_frame;

};

#endif
