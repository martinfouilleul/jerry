	# Execute 'make' to create prucode.bin and control loop cpu executable code
# Other options:
# make clean
# make all
# make pru
# make project
# make prucode
# make clean
#

PRU0 = pruTimer
PRU1 = pruConverter
MAIN = coala
FRAMEWORK_PATH = ../../../
ARCHITECTURE = cortex-a8
PWD = $(shell pwd)

INCLUDES = -I/usr/xenomai/include/ -I$(FRAMEWORK_PATH) -I$(FRAMEWORK_PATH)include/
LIBS = -lpthread -lprussdrv -lnative -lrt -lxenomai -lstdc++ -lm -ldl
LFLAGS = -L/usr/xenomai/lib
# $(FRAMEWORK_PATH)lib/arm/libffts.a

SOURCES = main.cpp CoalaWrapper.cpp BeagleBoneBlackRealTime.cpp Microcontroller.cpp PrussManager.cpp CoalaModel.cpp ModalFilter.cpp BiquadFilter.cpp ChirpFilter.cpp ModulatedGainFilter.cpp GenFilter.cpp SignalConverter.cpp ServerBase.cpp WebServer.cpp OSCServer.cpp $(FRAMEWORK_PATH)core/ControlLoopFacade.cpp $(FRAMEWORK_PATH)maths/Vector.cpp $(FRAMEWORK_PATH)maths/VectorDouble.cpp $(FRAMEWORK_PATH)maths/BlockDiagonalMatrix.cpp $(FRAMEWORK_PATH)maths/SquareMatrix.cpp $(FRAMEWORK_PATH)maths/r8lib.cpp $(FRAMEWORK_PATH)maths/matrix_exponential.cpp ActiveControlFilter.cpp TomControl.cpp
OBJS = $(SOURCES:.cpp=.o)

LEVEL = -O3
# -DNEON_ON
EXTRA_DEFINE = -mcpu=$(ARCHITECTURE) -mfpu=neon -mtune=$(ARCHITECTURE) -ftree-vectorize -ffast-math -fsingle-precision-constant -Wno-pointer-arith
# -mfloat-abi=softfp
CCFLAGS = $(EXTRA_DEFINE) -Wno-pointer-arith
CC = gcc
CFLAGS = $(EXTRA_DEFINE)
PASM = pasm

all: pru0 pru1 genmodule main
	@echo Compilation of main executable and pru assembly completed!

main: $(OBJS)
	$(CC) $(LEVEL) $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
#	cp -f $(MAIN) /usr/bin
#	@echo pwd=$(PWD)
	mv -f $(MAIN) ../bin
	ln -fs $(PWD)/../bin/$(MAIN) /usr/bin/coala

debug: $(OBJS)
#	$(CC) -g $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(SOURCES) $(LFLAGS) $(LIBS)
	$(CC) -g $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
#	cp -f $(MAIN) /usr/bin
	mv -f $(MAIN) ../bin
	ln -fs $(PWD)/../bin/$(MAIN) /usr/bin/coala

pru0: $(PRU0:%=%.p)
	$(PASM) -b $(PRU0).p
	mv -f $(PRU0).bin ../bin

pru1: $(PRU1:%=%.p)
	$(PASM) -b $(PRU1).p
	mv -f $(PRU1).bin ../bin

genmodule:
	make --directory=/usr/src/coala/gen -f /usr/src/coala/gen/Makefile

clean:
	rm -rf $(OBJS) ../*.bin ../$(MAIN) core *~

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@
