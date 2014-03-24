# Evil Mouse Prank for Arduino Yun
# CGI Send Script
# Copyright 2014 Tony DiCola (tony@tonydicola.com)
# Released under an MIT license (http://opensource.org/licenses/MIT)

# Simple pure CGI script to send commands to the EvilMousePrank process
# which forwards them to the ATmega processor.

import cgi
import os
import os.path
import struct


FIFO_PATH = '/tmp/EvilMousePrank.fifo'


def http_response(status, response=None, content_type='text/plain', other_headers=None):
	"""Send a simple HTTP response with the provided status, response, Content-Type
	and optional other headers."""
	print('Status: {0}'.format(status))
	if content_type is not None:
		print('Content-Type: {0}'.format(content_type))
	if other_headers is not None:
		print(other_headers)
	print('')
	if response is not None:
		print(response)


try:
	# Verify this is a POST request.
	method = os.environ['REQUEST_METHOD']
	if method != 'POST':
		http_response('405 Method Not Allowed', 'Method must be POST!', 'Allow: POST')
		quit()
	# Parse form fields.
	form = cgi.FieldStorage()
	command = form.getfirst('command')
	value = form.getfirst('value', 0)
	# Error if no command was specified.
	if command is None or len(command) != 1:
		http_response('400 Bad Request', 'Must specify command field with one character!')
		quit()
	# Check if the FIFO exists and open it for writing.
	if not os.path.exists(FIFO_PATH):
		raise RuntimeError('FIFO does not exist!')
	fifo = os.open(FIFO_PATH, os.O_WRONLY | os.O_NONBLOCK)
	if fifo < 0:
		raise RuntimeError('Could not open FIFO!')
	# Send the command and value to the FIFO
	if os.write(fifo, struct.pack('cb', command, int(value))) != 2:
		raise RuntimeError('Error sendind data to FIFO!')
	# Finish with OK response.
	http_response('200 OK', 'OK!')
except Exception as e:
	http_response('500 Internal Server Error', 'Error: {0}'.format(e))
finally:
	# Close the fifo connection if it exists.
	if fifo is not None and fifo >= 0:
		os.close(fifo)
