NUMPY_INCLUDE=$(shell python3 -c 'import numpy; print(numpy.get_include());')
PYTHON_INCLUDE=$(shell python3 -c 'import distutils.sysconfig; print(distutils.sysconfig.get_python_inc());')
CFLAGS=-I$(NUMPY_INCLUDE) -O3
CFLAGS+= -I$(PYTHON_INCLUDE)

all: start.so test


start.c: start.pyx
	cython start.pyx

start.so: start.c
	gcc -o start.so -shared -fPIC start.c $(CFLAGS) 

test:
	py.test



