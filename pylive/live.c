#include <Python.h>
#include "LibLive.h"

#define PYLIVE_INFO_OK 0
#define PYLIVE_ERROR_DEV -1
#define PYLIVE_ERROR_PARAMS -2

#define PYSDK_VERSION "PYLIVE version: 0.3"

PyObject* callback;

size_t get_data(char *buffer, size_t bufferSize) {
	PyObject *ret;
	//PyObject *bytes;
	size_t buffer_size;
	//PyGILState_STATE state = PyGILState_Ensure();	// 获取GIL控制权限
	ret = PyEval_CallObject(callback, NULL);
	//PyGILState_Release(state);						// 释放GIL控制权
	if (!PyBytes_Check(ret)) {
		return 0;
	}
	//PyArg_Parse(ret, "S", &bytes);
	buffer_size = PyBytes_Size(ret);
	char *p = PyBytes_AsString(ret);
	memcpy(buffer, p, (unsigned) buffer_size);
	Py_DECREF(ret);
	return buffer_size;
}

static PyObject* py_start(PyObject* self, PyObject* args, PyObject* kwargs) {

	printf(PYSDK_VERSION);
	printf("\n");

	MainParams mps;
	mps.fd = 0;
	mps.queueSize = 10;
	mps.rtspPort = 554;
	mps.user = NULL;
	mps.passwd = NULL;
	mps.audioDev = NULL;
	mps.audioFreq = 44100;
	mps.audioNbChannels = 2;
	mps.audioFmt = NULL;
	mps.murl = NULL;
	mps.rtspOverHTTPPort = 0;

	static char *kwlist[] = { "url", "width", "height", "bufferSize",
			"callback", "fd", "queueSize", "rtspPort", "user", "passwd",
			"audioDev", "audioFreq", "audioNbChannels", "audioFmt", "murl",
			"rtspOverHTTPPort", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "siiiO|iiisssiissi", kwlist,
			&mps.url, &mps.width, &mps.height, &mps.bufferSize, &callback,
			&mps.fd, &mps.queueSize, &mps.rtspPort, &mps.user, &mps.passwd,
			&mps.audioDev, &mps.audioFreq, &mps.audioNbChannels, &mps.audioFmt,
			&mps.murl, &mps.rtspOverHTTPPort)) {
		return Py_BuildValue("i", PYLIVE_ERROR_PARAMS);
	}

	Py_XINCREF(callback);

	start_service(mps, get_data);

	return Py_BuildValue("i", PYLIVE_INFO_OK);
}

static PyMethodDef module_methods[] = { { "start", (void*) py_start,
		METH_VARARGS | METH_KEYWORDS }, { NULL, NULL, 0, NULL } };

static struct PyModuleDef module_def = { PyModuleDef_HEAD_INIT, "live", NULL,
		-1, module_methods };

PyMODINIT_FUNC PyInit_live(void) {

	PyObject* module = NULL;
	module = PyModule_Create(&module_def);
	if (module == NULL)
		return NULL;

	PyModule_AddObject(module, "INFO_OK", Py_BuildValue("i", PYLIVE_INFO_OK));
	PyModule_AddObject(module, "ERR_DEV", Py_BuildValue("i", PYLIVE_ERROR_DEV));
	PyModule_AddObject(module, "ERR_PARAMS",
			Py_BuildValue("i", PYLIVE_ERROR_PARAMS));

	return module;
}
