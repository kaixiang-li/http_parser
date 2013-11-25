#include "Python.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parser.h"
#include <ctype.h>

static char* global_empty = "";
static char* global_http_prefix = "HTTP_";
static char* global_request_method = "REQUEST_METHOD";
static char* global_request_uri = "REQUEST_URI";
static char* global_fragment = "FRAGMENT";
static char* global_query_string = "QUERY_STRING";
static char* global_http_version = "HTTP_VERSION";
static char* global_content_length = "CONTENT_LENGTH";
static char* global_http_content_length = "HTTP_CONTENT_LENGTH";
static char* global_request_path = "REQUEST_PATH";
static char* global_content_type = "CONTENT_TYPE";
static char* global_http_content_type = "HTTP_CONTENT_TYPE";
static char* global_gateway_interface = "GATEWAY_INTERFACE";
static char* global_gateway_interface_value = "CGI/1.2";
static char* global_server_name = "SERVER_NAME";
static char* global_server_port = "SERVER_PORT";
static char* global_server_protocol = "SERVER_PROTOCOL";
static char* global_server_protocol_value = "HTTP/1.1";
static char* global_http_host = "HTTP_HOST";
static char* global_port_80 = "80";
static char* global_http_body = "wsgi.input";
static char* global_url_scheme = "wsgi.url_scheme";
static char* global_url_scheme_value = "http";
static char* global_script_name = "SCRIPT_NAME";
static char* global_path_info = "PATH_INFO";


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
    int i = 0;
    char buff[BUFF_LEN] = {0};
    char field_buff[BUFF_LEN] = {0};
    char value_buff[BUFF_LEN] = {0};

    strncpy(buff, field, flen);
    // upcase the field name
    for(i = 0; i < flen; i++) {
        buff[i] = toupper(buff[i]);
    }
    
    // complete field name
    strcpy(field_buff, global_http_prefix);
    strncat(field_buff, buff, flen);

    strncpy(value_buff, value, vlen);
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", value_buff);
    PyDict_SetItemString(req, field_buff, val);
}

static void 
request_method(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};
    strncpy(buff, at, length);

    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_request_method, val);
}

static void 
request_uri(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};
    strncpy(buff, at, length);
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_request_uri, val);
}

static void 
fragment(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};
    strncpy(buff, at, length);
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_fragment, val);
}

static void 
request_path(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};

    strncpy(buff, at, length);

    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_request_path, val);
    PyDict_SetItemString(req, global_path_info, val);
}

static void 
query_string(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};
    strncpy(buff, at, length);
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_query_string, val);

}

static void 
http_version(void *data, const char *at, size_t length)
{
    char buff[BUFF_LEN] = {0};
    strncpy(buff, at, length);
    PyObject* req = (PyObject*)data;
    PyObject* val = Py_None;

    val = Py_BuildValue("s", buff);
    PyDict_SetItemString(req, global_http_version, val);
}

static void
header_done(void *data, const char *at, size_t length)
{
    PyObject* req = (PyObject*)data;

    // server header field
    PyObject* content_length = Py_None;
    PyObject* content_type = Py_None;
    PyObject* http_host = Py_None;
    PyObject* body = Py_None;

    content_length = PyDict_GetItemString(req, global_http_content_length);
    if(content_length != NULL) {
        PyDict_SetItemString(req, global_content_length, content_length);
        PyDict_DelItemString(req, global_http_content_length);
    }

    content_type = PyDict_GetItemString(req, global_http_content_type);
    if(content_type != NULL) {
        PyDict_SetItemString(req, global_content_type, content_type);
        PyDict_DelItemString(req, global_http_content_type);
    }
    
    PyDict_SetItemString(req, global_gateway_interface, 
            Py_BuildValue("s", global_gateway_interface_value));
    if((http_host = PyDict_GetItemString(req, global_http_host)) != NULL) {
        PyDict_SetItemString(req, global_server_name, http_host);
        PyDict_SetItemString(req, global_server_port, 
            Py_BuildValue("s", global_port_80));
    }

    //body
    if(length > 0)  {
        body = PyDict_GetItemString(req, global_http_body);
        PyDict_SetItemString(req, global_http_body, body);
        // wait to do..
    }


    if(PyDict_GetItemString(req, global_query_string) == NULL) {
        PyDict_SetItemString(req, global_query_string, 
            Py_BuildValue("s", global_empty));
    }


    if(PyDict_GetItemString(req, global_path_info) == NULL) {
        PyDict_SetItemString(req, global_path_info, 
            Py_BuildValue("s", global_empty));
    } 
  
    
    // constants
    PyDict_SetItemString(req, global_server_protocol, 
            Py_BuildValue("s", global_server_protocol_value));
    PyDict_SetItemString(req, global_url_scheme, 
            Py_BuildValue("s", global_url_scheme_value));
    PyDict_SetItemString(req, global_script_name, 
            Py_BuildValue("s", global_empty));
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

	PyObject* req_hash = NULL;
    char *data;
    int start;
	static char* kwlist[] = {"request", "data", "start", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Osi:execute",
					 kwlist, &req_hash, &data, &start))
		return NULL;

    //PyObject_Print(request, stdout, 0);
    size_t dlen = strlen(data);
    http->data = (void *)req_hash;
    
    // execute
    http_parser_execute(http, data, dlen, 0);

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

