# Evil Mouse Prank for Arduino Yun
# Copyright 2014 Tony DiCola (tony@tonydicola.com)
# Released under an MIT license (http://opensource.org/licenses/MIT)

# Main python program that reads mouse move events and sends them to the ATmega processor.

import os
import os.path
import select
import struct
import time

import evdev
import serial


# This should point to the mouse device (which should always be event1 if there
# is only a mouse hooked up to the Yun, i.e. no USB hubs or other devices).
DEVICE_PATH = '/dev/input/event1'
# NOTE: On the stock Arduino Yun 1.0 Linino OS there is a bug which
# prevents USB peripherals from showing up as devices under /dev/input.
# Follow the steps in this thread to install fixed kmod packages whcih
# resolve the issue:
#   http://forum.arduino.cc/index.php?PHPSESSID=2eicdm22djrh9tfmveeqkuvjc2&topic=207069.15

# Use a FIFO to receive events from other processes (like uhttpd CGI scripts).
FIFO_PATH = '/tmp/EvilMousePrank.fifo'


if __name__ == '__main__':
	# Delete the FIFO if it already exists.
	if os.path.exists(FIFO_PATH):
		os.remove(FIFO_PATH)
	# Create the FIFO and open read only communication.
	os.mkfifo(FIFO_PATH)
	# Make sure to specify the O_NONBLOCK flag or else the open of the
	# FIFO will hang until something connects (not desired here!).
	fifo = os.open(FIFO_PATH, os.O_RDONLY | os.O_NONBLOCK)
	# Open serial communication to the ATmega processor.
	atmega = serial.Serial('/dev/ttyATH0', 115200)
	# Open communication with the mouse.
	device = evdev.Device(DEVICE_PATH)
	# Set some static state before entering the loop.
	read_fds = [fifo, device.fd]
	eventsize = evdev.Event.get_format_size()
	try:
		# Loop endlessly waiting for input from the mouse or FIFO.
		while True:
			read, write, error = select.select(read_fds,[],[])
			if fifo in read:
				# FIFO has data, read two bytes (command and value) 
				# and send them to the ATmega.
				packet = os.read(fifo, 2)
				if len(packet) == 2:
					# First reset the current state, then set the new state.
					atmega.write('r0' + packet)
			if device.fd in read:
				# Mouse has a new event, read and handle it.
				data = os.read(device.fd, eventsize)
				event = evdev.Event(unpack=data)
				if event is None:
					continue
				command = None
				if event.type == 'EV_REL' and event.code == 'REL_X':
					# Mouse X axis move.
					command = 'X'
				elif event.type == 'EV_REL' and event.code == 'REL_Y':
					# Mouse Y axis move.
					command = 'Y'
				elif event.type == 'EV_REL' and event.code == 'REL_WHEEL':
					# Mouse wheel axis move.
					command = 'W'
				elif event.type == 'EV_KEY' and event.code == 'BTN_LEFT':
					# Mouse left button.
					command = 'L'
				elif event.type == 'EV_KEY' and event.code == 'BTN_RIGHT':
					# Mouse right button.
					command = 'R'
				elif event.type == 'EV_KEY' and event.code == 'BTN_MIDDLE':
					# Mouse middle button.
					command = 'M'
				# Send the event to the ATmega.
				if command is not None:
					atmega.write(struct.pack('cb', command, event.value))
	finally:
		# Close everything and clean up the FIFO.
		atmega.close()
		os.close(fifo)
		os.remove(FIFO_PATH)
