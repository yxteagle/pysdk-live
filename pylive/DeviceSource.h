#ifndef DEVICE_SOURCE
#define DEVICE_SOURCE

#include <list>
#include <iomanip>
#include <liveMedia.hh>

#include "DeviceInterface.h"

class V4L2DeviceSource: public FramedSource {
public:
	// ---------------------------------
	// Captured frame
	// ---------------------------------
	struct Frame {
		Frame(char* buffer, int size, timeval timestamp) :
				m_buffer(buffer), m_size(size), m_timestamp(timestamp) {
		}
		;
		Frame(const Frame&);
		Frame& operator=(const Frame&);
		~Frame() {
			delete[] m_buffer;
		}
		;

		char* m_buffer;
		int m_size;
		timeval m_timestamp;
	};

	// ---------------------------------
	// Compute simple stats
	// ---------------------------------
	class Stats {
	public:
		Stats(const std::string & msg) :
				m_fps(0), m_fps_sec(0), m_size(0), m_msg(msg) {
		}
		;

	public:
		int notify(int tv_sec, int framesize);

	protected:
		int m_fps;
		int m_fps_sec;
		int m_size;
		const std::string m_msg;
	};

public:
	static V4L2DeviceSource* createNew(UsageEnvironment& env,
			DeviceInterface * device, unsigned int queueSize);
	std::string getAuxLine() {
		return m_auxLine;
	}
	;
	int getWidth() {
		return m_device->getWidth();
	}
	;
	int getHeight() {
		return m_device->getHeight();
	}
	;

protected:
	V4L2DeviceSource(UsageEnvironment& env, DeviceInterface * device,
			unsigned int queueSize);
	virtual ~V4L2DeviceSource();

protected:
	static void* threadStub(void* clientData) {
		return ((V4L2DeviceSource*) clientData)->thread();
	}
	;
	void* thread();
	static void deliverFrameStub(void* clientData) {
		((V4L2DeviceSource*) clientData)->deliverFrame();
	}
	;
	void deliverFrame();
	static void incomingPacketHandlerStub(void* clientData, int mask) {
		((V4L2DeviceSource*) clientData)->incomingPacketHandler();
	}
	;
	void incomingPacketHandler();
	int getNextFrame();
	void processFrame(char * frame, int frameSize, const timeval &ref);
	void queueFrame(char * frame, int frameSize, const timeval &tv);

	// split packet in frames
	virtual std::list<std::pair<unsigned char*, size_t> > splitFrames(
			unsigned char* frame, unsigned frameSize);

	// overide FramedSource
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();

protected:
	std::list<Frame*> m_captureQueue;
	Stats m_in;
	Stats m_out;
	EventTriggerId m_eventTriggerId;
	DeviceInterface * m_device;
	unsigned int m_queueSize;
	pthread_t m_thid;
	pthread_mutex_t m_mutex;
	std::string m_auxLine;
	char *buffer;

};

#endif
