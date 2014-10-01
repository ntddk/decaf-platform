
include config-plugin.mak
include $(SRC_PATH)/$(TARGET_DIR)/config-target.mak
include $(SRC_PATH)/config-host.mak


DEFINES=-I. -I$(SRC_PATH) -I$(SRC_PATH)/plugins -I$(SRC_PATH)/fpu -I$(SRC_PATH)/shared -I$(SRC_PATH)/target-$(TARGET_ARCH) -I$(SRC_PATH)/$(TARGET_DIR) -I$(SRC_PATH)/slirp -I$(SRC_PATH)/shared/hooks 
DEFINES+=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -DNEED_CPU_H
DEFINES+=-I$(GLIB_CFLAGS)

CC=gcc
CPP=g++
CFLAGS=-Wall -O2 -g -fPIC -MMD -Wno-unused-variable 
 
LDFLAGS=-g -shared 

OBJS=keystroketaint.o

all: keystroketaint.so

%.o: %.c 
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

%.o: %.cpp
	$(CPP) $(CFLAGS) $(DEFINES) -c -o $@ $<

keystroketaint.so: $(OBJS)
	$(CPP) $(LDFLAGS) $^ -o $@ $(LIBS)
	ar cru libkeystroketaint.a $@

apitracer-static.so: $(OBJS)
	$(CPP) -static-libgcc -Wl,-static $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o *.d *.so *.a *~ $(PLUGIN) 

realclean:
	rm -f *.o  *.d *.so *.a *~ $(PLUGIN)

# Include automatically generated dependency files
-include $(wildcard *.d)
