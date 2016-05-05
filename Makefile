ARCH=ARM
OTHERSLIBPWD=/home/usrc

ifeq ($(ARCH),ARM)
#CROSS=arm-linux-
#CROSS=mipsel-openwrt-linux-
CROSS=arm-linux-gnueabihf-
endif

SRC:=$(shell ls *.cpp)

#CFLAG:=-m32
LDFLAG:=-static

ifeq ($(ARCH),ARM)
else
endif

ifeq ($(ARCH),ARM)
else
endif

ifeq ($(ARCH),ARM)
LIBS:=-lpthread
else
LIBS:=-lpthread
endif

ifeq ($(ARCH),ARM)
TARGET:=sys_monitor
else
TARGET:=sys_monitor
endif

$(TARGET) : $(SRC)
	$(CROSS)g++ $(CFLAG) -o $(TARGET) $^ $(LPATH) $(IPATH) $(LIBS) $(LDFLAG)

clean:
	rm -f  *.bin  *.dis  *.elf  *.o
