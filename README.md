# Evil Mouse Prank

Torture someone by intercepting their USB mouse communication and manipulating
it in evil ways like inverting the axes or emulating an old sticky mechanical
ball mouse!

See a video demonstration and explanation of the prank here: https://www.youtube.com/watch?v=XACMdu2bcdE

Copyright 2014 Tony DiCola (tony@tonydicola.com)

## Usage

-   This ONLY works on an Arduin Yun.  The victim's mouse needs to be plugged
    in to the Yun's USB host port (tall vertical one), and the Yun's USB mini
    port plugged in to their computer.  The Yun will intercept the mouse
    movements, manipulate them in nefarious ways, and send the mouse commands
    to the attached computer.

-   This sketch requires some advanced modifications to the Yun.  You will need
    to be comfortable logging in to the Yun's command shell and manipulating the
    Linux environment.

-   You will need to make some modifications to your Yun to deal with a few 
    software issues:

    -   Out of the box the Yun doesn't work with USB HID devices because of a bug
        in OpenWRT.  Follow the steps by sonnyyu in this thread to enable the 
        /dev/input/event1 device: http://forum.arduino.cc/index.php?topic=207069.msg1641669#msg1641669

    -   You MUST disable the Yun Bridge library completely so the Serial1 connection
        can be used exclusively by the sketch for communication.  Unfortunately the
        Bridge library has serious performance problems and is not fast enough to
        send mouse events between the Yun's two processors.  Follow this thread to
        comment the ttyATH0 line in /etc/inittab file: http://forum.arduino.cc/index.php?topic=191820.45

        When you are done with the prank, uncomment the modified line or else the Bridge
        library will be unusable in your own sketches!

-   Install the PySerial library on the Yun's Linux processor.  Follow this blog
    post to install PIP: http://codybonney.com/installing-pip-on-the-arduino-yun/

    Then run the command: ````pip install pyserial````

-   Copy both the EvilMouseYun.py and evdev.py files to an SD card that is
    mounted on the Yun (/mnt/sda1).
      
-   Create the folder /mnt/sda1/arduino/www and copy in both index.html and send.py from the
    EvilMousePrank_WWW folder.
        
-   Update the uhttpd web server configuration to support python CGI scripts.  Edit /etc/config/uhttpd and
    add the line ````list interpreter    ".py=/usr/bin/python"```` with the other interpretor defines.  See
    this question for more details (you don't need to worry about setting up flask though): http://stackoverflow.com/questions/21791209/arduino-yun-uhttpd-flask-setup

-   Update the /etc/rc.local file to add before the last line ````python /mnt/sda1/EvilMousePrank.py```` so the
    python script is run automatically after boot.

    If you don't want to always run this script, you can instead manually log in to the Yun and run that python
    script when you want to enable the prank (press Ctrl-C to quit the running script).

-   Finally load the EvilMousePrank_Sketch Arduino sketch on the Yun.

-   Reboot the Yun and wait 90 seconds for the sketch to start listening to mouse events.  If you have the default name assigned to the Yun, access http://arduino.local/sd/ to see the web page for manipulating the mouse behavior.  Replace arduino.local with the appropriate name for the Yun if you've changed it.  On some systems like Windows or Android there might not be multicast DNS support, so try accessing http://\<IP ADDRESS OF YUN\>/sd/ to see the page.
