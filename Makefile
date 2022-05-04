VERSION := 1.0.0

SOURCE := $(wildcard *.c ./src/*.c)

OBJ := $(patsubst %.c,%.o,$(SOURCE))

OTH := $(filter-out usbDongle.o,$(OBJ))

CC:=/opt/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin/mipsel-openwrt-linux-gcc

LDFLAGS :=  -lpthread -luci

CFLAGS := -g -Wall

DEBUG := -D PROGRAM_NAME=\"usb_dongle\" -D DEBUG

INCLUDES := -I ./include/

TARGET := usb_dongle 

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(OTH) -o $@ $(DEBUG)
%.o:%.c
	$(CC) -c $(CLFAGS) -o $@ $^ $(INCLUDES) $(DEBUG)

.PHONY:clean
clean:
	rm -rf ./src/*.o ./src/*.gch usb_dongle 
