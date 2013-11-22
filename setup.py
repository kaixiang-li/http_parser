from distutils.core import setup, Extension
setup(name='http_parser', version='1.0',  \
      ext_modules=[Extension('http_parser', ['http_parser.c', 'parser.c'])])
