#ifndef H264_V4L2_DEVICE_SOURCE
#define H264_V4L2_DEVICE_SOURCE

// project
#include "DeviceSource.h"

// ---------------------------------
// H264 V4L2 FramedSource
// ---------------------------------
const char H264marker[] = { 0, 0, 0, 1 };
const char H264shortmarker[] = { 0, 0, 1 };

class H26X_V4L2DeviceSource: public V4L2DeviceSource {
protected:
	H26X_V4L2DeviceSource(UsageEnvironment& env, DeviceInterface * device,
			unsigned int queueSize, bool repeatConfig) :
			V4L2DeviceSource(env, device, queueSize), m_repeatConfig(
					repeatConfig), m_frameType(0) {
	}

	virtual ~H26X_V4L2DeviceSource() {
	}

	virtual unsigned char* extractFrame(unsigned char* frame, size_t& size,
			size_t& outsize);

protected:
	std::string m_sps;
	std::string m_pps;
	bool m_repeatConfig;
	int m_frameType;
};

class H264_V4L2DeviceSource: public H26X_V4L2DeviceSource {
public:
	static H264_V4L2DeviceSource* createNew(UsageEnvironment& env,
			DeviceInterface * device, unsigned int queueSize,
			bool repeatConfig) {
		return new H264_V4L2DeviceSource(env, device, queueSize, repeatConfig);
	}

protected:
	H264_V4L2DeviceSource(UsageEnvironment& env, DeviceInterface * device,
			unsigned int queueSize, bool repeatConfig) :
			H26X_V4L2DeviceSource(env, device, queueSize, repeatConfig) {
	}

	// overide V4L2DeviceSource
	virtual std::list<std::pair<unsigned char*, size_t> > splitFrames(
			unsigned char* frame, unsigned frameSize);
};

#endif
