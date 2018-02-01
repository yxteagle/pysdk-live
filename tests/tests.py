import os
from multiprocessing import Process, Queue
from pylive import live
from pyuvc import uvc


DEV = '/dev/video1'
STREAM_NUM = 4
BASE_RTSP_PORT = 8554
IMG_WIDTH = 1280
IMG_HEIGHT = 720
STREAM_DATA_LIST = []


class Capture:

    def __init__(self):
        params = {
            'width': IMG_WIDTH,
            'height': IMG_HEIGHT,
            'fps': 30,
            'fmt_type': uvc.H264
            }
        print('Start capture: %s' % DEV)
        r = uvc.open(
            fd=DEV,
            **params
            )
        if r != 0:
            print('Open capture failed, test exit(%d)' % r)
            os._exit(0)

    def init_stream_list(self):
        for _ in range(STREAM_NUM):
            STREAM_DATA_LIST.append(Queue())

    def start_stream_loop(self):
        while 1:
            data = uvc.get_frame()
            for i in STREAM_DATA_LIST:
                i.put(data)


class Live:

    def __init__(self, i):
        self._i = i

    def _live_loop(self):
        data_factory = STREAM_DATA_LIST[self._i]

        def callback():
            return data_factory.get()

        live.start(
            url='unicast',
            width=IMG_WIDTH,
            height=IMG_HEIGHT,
            bufferSize=uvc.get_buffer_size(),
            callback=callback,
            # fd=uvc.get_fd(),
            # queueSize=10,
            rtspPort=BASE_RTSP_PORT+self._i,
            # user='admin',
            # passwd='admin',
            # audioDev='default',
            # audioFreq=44100,
            # audioNbChannels=2,
            # murl='multicast',
            # rtspOverHTTPPort=8080
            )

    def start_live_loop_thread(self):
        print('start live loop process: %d' % self._i)
        p = Process(target=self._live_loop)
        p.daemon = True
        p.start()


def test_multi():
    c = Capture()
    c.init_stream_list()
    for i in range(STREAM_NUM):
        Live(i).start_live_loop_thread()
    c.start_stream_loop()


def test_base():

    def callback():
        return uvc.get_frame()

    Capture()
    live.start(
        url='unicast',
        width=IMG_WIDTH,
        height=IMG_HEIGHT,
        bufferSize=uvc.get_buffer_size(),
        callback=callback,
        # fd=uvc.get_fd(),
        # queueSize=10,
        rtspPort=8554,
        # user='admin',
        # passwd='admin',
        audioDev='default',
        # audioFreq=44100,
        # audioNbChannels=2,
        # audioFmt='S16_BE',
        # murl='multicast',
        # rtspOverHTTPPort=8080
        )


if __name__ == '__main__':
    test_base()
    # test_multi()

