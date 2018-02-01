from distutils.core import setup
from distutils.core import Extension
from distutils.command.build_ext import build_ext as _build_ext


class build_ext(_build_ext):
    def run(self):
        print('WTF, Nothing but python is the best language for developers!')
        _build_ext.run(self)


modules = [

    Extension(
        'pylive.live',
        sources=[
            'pylive/live.c',
            'pylive/DeviceSource.cpp',
            'pylive/H264_V4l2DeviceSource.cpp',
            'pylive/LibLive.cpp',
            'pylive/ServerMediaSubsession.cpp',
            'pylive/ALSACapture.cpp'
            ],
        extra_compile_args=[
            '-std=c++11',
            ],
        include_dirs=[
            '/usr/include/BasicUsageEnvironment',
            '/usr/include/groupsock',
            '/usr/include/liveMedia',
            '/usr/include/UsageEnvironment'
            ],
        library_dirs=['/usr/lib'],
        libraries=[
            'liveMedia',
            'pthread',
            'log4cpp',
            'groupsock',
            'BasicUsageEnvironment',
            'UsageEnvironment',
            'asound'
            ],
        ),

]

setup(

    name='pysdk-live',
    version='0.3',
    author='yxt',
    author_email='yxteagle@gmail.com',
    url='http://mail.google.com',
    license='BSD',
    packages=['pylive'],
    description='sdk for media in python3',
    long_description=open('README.md').read() + open('CHANGES.txt').read(),
    classifiers=['Development Status :: 3 - Alpha',
                 'Environment :: Console',
                 'Intended Audience :: Developers',
                 'Intended Audience :: Education',
                 'License :: OSI Approved :: MIT License',
                 'Operating System :: POSIX :: Linux',
                 'Programming Language :: Python',
                 'Topic :: Home Automation',
                 'Topic :: Software Development :: Embedded Systems'
                 ],
    ext_modules=modules,
    cmdclass={'build_ext': build_ext}

)
