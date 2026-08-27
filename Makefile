CC ?= cc
PKG_CONFIG ?= pkg-config
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags libusb-1.0)
LDLIBS += $(shell $(PKG_CONFIG) --libs libusb-1.0)

TARGET := build/elan-led
SOURCE := src/elan-led.c

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -rf build

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)/usr/local/libexec/elan-led
	install -Dm644 systemd/fprintd.service.d/led.conf \
		$(DESTDIR)/etc/systemd/system/fprintd.service.d/led.conf

uninstall:
	rm -f $(DESTDIR)/usr/local/libexec/elan-led
	rm -f $(DESTDIR)/etc/systemd/system/fprintd.service.d/led.conf
