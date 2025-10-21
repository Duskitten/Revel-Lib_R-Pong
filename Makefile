TARGET = main
OBJS = main.o

INCDIR =
CFLAGS = -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu -lpspdebug -lpspdisplay -lpspctrl
BUILD_PRX=1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Revel Lib R-Pong

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
