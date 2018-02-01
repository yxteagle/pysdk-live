#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include "logger.h"

#include "H264_V4l2DeviceSource.h"
#include "ServerMediaSubsession.h"
#include "Capture.h"
#include "ALSACapture.h"

extern "C" {
#include "LibLive.h"
}
using namespace std;

char quit = 0;

snd_pcm_format_t decodeAudioFormat(const std::string& fmt) {
	snd_pcm_format_t audioFmt = SND_PCM_FORMAT_UNKNOWN;
	if (fmt == "S16_BE") {
		audioFmt = SND_PCM_FORMAT_S16_BE;
	} else if (fmt == "S16_LE") {
		audioFmt = SND_PCM_FORMAT_S16_LE;
	} else if (fmt == "S32_BE") {
		audioFmt = SND_PCM_FORMAT_S32_BE;
	} else if (fmt == "S32_LE") {
		audioFmt = SND_PCM_FORMAT_S32_LE;
	}
	return audioFmt;
}

// -----------------------------------------
//    create UserAuthenticationDatabase for RTSP server
// -----------------------------------------
UserAuthenticationDatabase* createUserAuthenticationDatabase(const char *user =
		NULL, const char *passwd = NULL) {
	UserAuthenticationDatabase* auth = NULL;
	if (user != NULL && passwd != NULL) {
		auth = new UserAuthenticationDatabase();
		auth->addUserRecord(user, passwd);
	}
	return auth;
}

// -----------------------------------------
//    create RTSP server
// -----------------------------------------
RTSPServer* createRTSPServer(UsageEnvironment& env, unsigned short rtspPort,
		unsigned short rtspOverHTTPPort, int timeout, const char *user,
		const char *passwd) {
	UserAuthenticationDatabase* auth = createUserAuthenticationDatabase(user,
			passwd);
	//RTSPServer* rtspServer = HTTPServer::createNew(env, rtspPort, auth, timeout, hlsSegment);
	RTSPServer* rtspServer = RTSPServer::createNew(env, rtspPort, auth,
			timeout);
	if (rtspServer != NULL) {
		// set http tunneling
		if (rtspOverHTTPPort) {
			rtspServer->setUpTunnelingOverHTTP(rtspOverHTTPPort);
		}
	}
	return rtspServer;
}

// -----------------------------------------
//    create FramedSource server
// -----------------------------------------
FramedSource* createFramedSource(UsageEnvironment* env,
		DeviceInterface* videoCapture, int queueSize, bool repeatConfig) {
	FramedSource* source = H264_V4L2DeviceSource::createNew(*env, videoCapture,
			queueSize, repeatConfig);
	;
	return source;
}

// -----------------------------------------
//    add an RTSP session
// -----------------------------------------
int addSession(RTSPServer* rtspServer, const string & sessionName,
		const list<ServerMediaSubsession*> & subSession) {
	int nbSubsession = 0;
	if (subSession.empty() == false) {
		UsageEnvironment& env(rtspServer->envir());
		ServerMediaSession* sms = ServerMediaSession::createNew(env,
				sessionName.c_str());
		if (sms != NULL) {
			list<ServerMediaSubsession*>::const_iterator subIt;
			for (subIt = subSession.begin(); subIt != subSession.end();
					++subIt) {
				sms->addSubsession(*subIt);
				nbSubsession++;
			}

			rtspServer->addServerMediaSession(sms);

			char* url = rtspServer->rtspURL(sms);
			if (url != NULL) {
				LOG(NOTICE)<< "Play this stream using the URL \"" << url << "\"";
				delete[] url;
			}
		}
	}
	return nbSubsession;
}

// -------------------------------------------------------
//    decode multicast url <group>:<rtp_port>:<rtcp_port>
// -------------------------------------------------------
void decodeMulticastUrl(const string & maddr, in_addr & destinationAddress,
		unsigned short & rtpPortNum, unsigned short & rtcpPortNum) {
	istringstream is(maddr);
	string ip;
	getline(is, ip, ':');
	if (!ip.empty()) {
		destinationAddress.s_addr = inet_addr(ip.c_str());
	}

	string port;
	getline(is, port, ':');
	rtpPortNum = 20000;
	if (!port.empty()) {
		rtpPortNum = atoi(port.c_str());
	}
	rtcpPortNum = rtpPortNum + 1;
}

void start_service(MainParams mps, f_get_data get_data) {
	//const char *user = NULL;
	//cout << "user: " << user << endl;

	//const char *passwd = NULL;
	//cout << "passwd: " << passwd << endl;

	//string url = "unicast";
	string url = mps.url;
	cout << "unicast url: " << url << endl;

	//int queueSize = 10;
	cout << "Number of frame queue(queueSize): " << mps.queueSize << endl;

	//unsigned short rtspPort = 8554;
	cout << "rtspPort: " << mps.rtspPort << endl;

	//unsigned short rtspOverHTTPPort = 0;
	cout << "rtspOverHTTPPort: " << mps.rtspOverHTTPPort << endl;

	//string murl = "multicast";
	string murl;
	string maddr;
	bool multicast = false;
	if (mps.murl != NULL) {
		multicast = true;
		murl = mps.murl;
	}
	//cout << "multicast enabled: " << multicast <<endl;
	//cout << "multicast url: " << murl <<endl;
	//cout << "multicast addr(default is random_address:20000): " << maddr <<endl;

	int verbose = 0;
	cout << "verbose(0,1,2): " << verbose << endl;

	int timeout = 65;
	cout << "RTCP expiration timeout in seconds(timeout): " << timeout << endl;

	bool repeatConfig = true;
	cout << "repeat config before IDR frame(repeatConfig): " << repeatConfig
			<< endl;

	//const char *audioDev = "default";

	//int audioFreq = 44100;
	cout << "audioFreq: " << mps.audioFreq << endl;

	//int audioNbChannels = 2;
	cout << "audioNbChannels: " << mps.audioNbChannels << endl;

	snd_pcm_format_t audioFmt = decodeAudioFormat("S16_BE");
	if (mps.audioFmt != NULL) {
		audioFmt = decodeAudioFormat(mps.audioFmt);
	}
	cout << "audioFmt(default S16_BE): " << mps.audioFmt << endl;

	// init logger
	initLogger(verbose);

	// create live555 environment
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	// split multicast info
	struct in_addr destinationAddress;
	destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
	unsigned short rtpPortNum = 20000;
	unsigned short rtcpPortNum = rtpPortNum + 1;
	unsigned char ttl = 5;
	decodeMulticastUrl(maddr, destinationAddress, rtpPortNum, rtcpPortNum);

	// create RTSP server
	RTSPServer* rtspServer = createRTSPServer(*env, mps.rtspPort,
			mps.rtspOverHTTPPort, timeout, mps.user, mps.passwd);
	if (rtspServer == NULL) {
		LOG(ERROR)<< "Failed to create RTSP server: " << env->getResultMsg();
	}
	else
	{
		int nbSource = 0;
		string baseUrl;

		StreamReplicator* videoReplicator = NULL;
		string rtpFormat;
		string fmt = "video/H264";
		rtpFormat.assign(fmt);

		// Init video capture
		MyCapture *videoCapture = new MyCapture(mps.width, mps.height, mps.bufferSize, get_data, mps.fd);
		FramedSource* videoSource = createFramedSource(env, new DeviceCaptureAccess<MyCapture>(videoCapture), mps.queueSize, repeatConfig);
		if (videoCapture->getBufferSize() > OutPacketBuffer::maxSize)
		{
			OutPacketBuffer::maxSize = videoCapture->getBufferSize();
		}

		videoReplicator = StreamReplicator::createNew(*env, videoSource, false);

		// Init audio capture
		StreamReplicator* audioReplicator = NULL;
		string rtpAudioFormat;
		if (mps.audioDev != NULL)
		{
			LOG(NOTICE) << "Create ALSA Source..." << mps.audioDev;

			ALSACaptureParameters param(mps.audioDev, audioFmt, mps.audioFreq, mps.audioNbChannels, verbose);
			ALSACapture* audioCapture = ALSACapture::createNew(param);
			if (audioCapture)
			{
				FramedSource* audioSource = V4L2DeviceSource::createNew(*env, new DeviceCaptureAccess<ALSACapture>(audioCapture), mps.queueSize);
				if (audioSource == NULL)
				{
					LOG(FATAL) << "Unable to create source for device " << mps.audioDev;
					delete audioCapture;
				}
				else
				{
					std::ostringstream os;
					os << "audio/L16/" << audioCapture->getSampleRate() << "/" << audioCapture->getChannels();
					rtpAudioFormat.assign(os.str());

					// extend buffer size if needed
					if (audioCapture->getBufferSize() > OutPacketBuffer::maxSize)
					{
						OutPacketBuffer::maxSize = audioCapture->getBufferSize();
					}
					audioReplicator = StreamReplicator::createNew(*env, audioSource, false);
				}
			}
		}

		// Create Multicast Session
		if (multicast)
		{
			LOG(NOTICE) << "RTP  address " << inet_ntoa(destinationAddress) << ":" << rtpPortNum;
			LOG(NOTICE) << "RTCP address " << inet_ntoa(destinationAddress) << ":" << rtcpPortNum;

			list<ServerMediaSubsession*> subSession;
			if (videoReplicator)
			{
				subSession.push_back(MulticastServerMediaSubsession::createNew(*env, destinationAddress, Port(rtpPortNum), Port(rtcpPortNum), ttl, videoReplicator, rtpFormat));
				// increment ports for next sessions
				rtpPortNum+=2;
				rtcpPortNum+=2;
			}
			if (audioReplicator)
			{
				subSession.push_back(MulticastServerMediaSubsession::createNew(*env, destinationAddress, Port(rtpPortNum), Port(rtcpPortNum), ttl, audioReplicator, rtpAudioFormat));

				// increment ports for next sessions
				rtpPortNum+=2;
				rtcpPortNum+=2;
			}
			nbSource += addSession(rtspServer, baseUrl+murl, subSession);
		}

		list<ServerMediaSubsession*> subSession;
		if (videoReplicator)
		{
			subSession.push_back(UnicastServerMediaSubsession::createNew(*env, videoReplicator, rtpFormat));
		}
		if (audioReplicator)
		{
			subSession.push_back(UnicastServerMediaSubsession::createNew(*env, audioReplicator, rtpAudioFormat));
		}
		nbSource += addSession(rtspServer, baseUrl+url, subSession);

		if (nbSource>0)
		{
			// main loop
			env->taskScheduler().doEventLoop(&quit);
			LOG(NOTICE) << "Exiting....";
		}

		Medium::close(rtspServer);
		delete videoCapture;

	}

	env->reclaim();
	delete scheduler;
}
