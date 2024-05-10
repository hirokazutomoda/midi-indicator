# Makefile for compiling rawinout.c and managing the service file

CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = rawinout
SERVICE = rawinput.service

all: $(TARGET)

$(TARGET): rawinout.c
	$(CC) $(CFLAGS) rawinout.c -o $(TARGET) -lasound

install-service:
	sudo cp $(SERVICE) /etc/systemd/system/$(SERVICE)
	sudo systemctl daemon-reload
	sudo systemctl enable $(SERVICE)

link-service:
	sudo ln -sf $(PWD)/$(SERVICE) /etc/systemd/system/$(SERVICE)
	sudo systemctl daemon-reload
	sudo systemctl enable $(SERVICE)

clean:
	rm -f $(TARGET)

.PHONY: all install-service link-service clean

