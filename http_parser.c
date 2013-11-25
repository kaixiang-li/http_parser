#include "Python.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parser.h"
#include <ctype.h>

//static char* global_empty = "";
//static char* global_http_prefix = "HTTP_";
static char* global_request_method = "REQUEST_METHOD";
//static char* global_request_uri = "REQUEST_URI";
//static char* global_fragment = "FRAGMENT";
//static char* global_query_string = "QUERY_STRING";
//static char* global_http_version = "HTTP_VERSION";
//static char* global_content_length = "CONTENT_LENGTH";
//static char* global_http_content_length = "HTTP_CONTENT_LENGTH";
//static char* global_request_path = "REQUEST_PATH";
//static char* global_content_type = "CONTENT_TYPE";
//static char* global_http_content_type = "HTTP_CONTENT_TYPE";
//static char* global_gateway_interface = "GATEWAY_INTERFACE";
//static char* global_gateway_interface_value = "CGI/1.2";
//static char* global_server_name = "SERVER_NAME";
//static char* global_server_port = "SERVER_PORT";
//static char* global_server_protocol = "SERVER_PROTOCOL";
//static char* global_server_protocol_value = "HTTP/1.1";
//static char* global_http_host = "HTTP_HOST";
//static char* global_port_80 = "80";
//static char* global_http_body = "wsgi.input";
//static char* global_url_scheme = "wsgi.url_scheme";
//static char* global_url_scheme_value = "http";
//static char* global_script_name = "SCRIPT_NAME";
//static char* global_path_info = "PATH_INFO";


/* HTTPParser - http parser instance.
 * callback methods:
 *  http_field
 *  request_method
 *  request_uri
 *  fragment
 *  request_path
 *  query_string
 *  http_version
 *  header_done
*/

typedef struct {
    PyObject_HEAD
    http_parser *http;
} HTTPParser;
#define BUFF_LEN 4096

static void 
http_field(void *data, const char *field,  size_t flen, const char *value, size_t vlen)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, field, flen);
    strcat(buff, ": ");
    strncat(buff, value, vlen);
    printf("HTTP_FIELD: \"%s\"\n", buff);
}

static void 
request_method(void *data, const char *at, size_t length)
{
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", at);
    //PyObject_Print(val, stdout, 0);
    PyDict_SetItemString(req, global_request_method, val);
}

static void 
request_uri(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    printf("URI: \"%s\"\n", buff);

}

static void 
fragment(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    printf("FRAGMENT: \"%s\"\n", buff);
}

static void 
request_path(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    printf("PATH: \"%s\"\n", buff);
}

static void 
query_string(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    printf("QUERY: \"%s\"\n", buff);
}

static void 
http_version(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    printf("VERSION: \"%s\"\n", buff);
}

static void
header_done(void *data, const char *at, size_t length)
{
    printf("done.\n");
}

static PyObject *
HTTPParser_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    // Create a new HTTPParser and initialize its state.
    HTTPParser *parser = (HTTPParser *)type->tp_alloc(type, 0);;
    if(!parser) 
        return NULL;

   // initialize the parser
    http_parser *http = (http_parser *) PyMem_Malloc(sizeof(http_parser));
    if(!http)
        return NULL;

    parser->http = http;
    http->http_field = http_field;
    http->request_method = request_method;
    http->request_uri = request_uri;
    http->fragment = fragment;
    http->request_path = request_path;
    http->query_string = query_string;
    http->http_version = http_version;
    http->header_done = header_done;
    http_parser_init(http);

    return (PyObject *)parser;
}


static PyObject*
HTTPParser_execute(PyObject *self, PyObject *args, PyObject *kwargs)
{
    HTTPParser *parser = (HTTPParser*)self;
    http_parser *http = parser->http;
	PyObject* request;
    char *data;
    int start;
	static char* kwlist[] = {"request", "data", "start", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Osd:execute",
					 kwlist, &request, &data, &start))
		return NULL;

    PyObject_Print(request, stdout, 0);
    size_t dlen = strlen(data);
    //printf("%s", data);
    printf("%d", start);
    //http->data = (void *)req_hash;
    // execute
    http_parser_execute(parser->http, data, dlen, 0);

    return (PyObject *)self;
}

static PyMethodDef http_parser_methods[] = {
    {"execute",(PyCFunction)HTTPParser_execute, METH_KEYWORDS,	NULL},
    {NULL}  /* Sentinel */
};


PyTypeObject PyHTTPParser_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "http_parser",                       /* tp_name */
    sizeof(HTTPParser),            /* tp_basicsize */
    0,                              /* tp_itemsize */
    0, //(destructor)HTTPParser_dealloc,     /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    0,                              /* tp_iternext */
    http_parser_methods,            /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    PyType_GenericAlloc,            /* tp_alloc */
    HTTPParser_new                  /* tp_new */
};



#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inithttp_parser(void)
{
    PyObject* module; //PyModule_Create(&http_parsermodule);

    

    if (PyType_Ready(&PyHTTPParser_Type) < 0)
        return;

    module = Py_InitModule3("http_parser", http_parser_methods,
                            "http_parser module that uses ragel parser.");

    Py_INCREF((PyObject *)&PyHTTPParser_Type);
    PyModule_AddObject(module, "HTTPParser", (PyObject *)&PyHTTPParser_Type);
}

