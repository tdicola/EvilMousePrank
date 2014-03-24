// Evil Mouse Prank for Arduino Yun
//
// Torture someone by intercepting their USB mouse communication and manipulating
// it in evil ways like inverting the axes or emulating an old sticky mechanical
// ball mouse!
//
// Copyright 2014 Tony DiCola (tony@tonydicola.com)
//
// Usage:
//
//   - This ONLY works on an Arduin Yun.  The victim's mouse needs to be plugged
//     in to the Yun's USB host port (tall vertical one), and the Yun's USB mini
//     port plugged in to their computer.  The Yun will intercept the mouse
//     movements, manipulate them in nefarious ways, and send the mouse commands
//     to the attached computer.
//
//   - This sketch requires some advanced modifications to the Yun.  You will need
//     to be comfortable logging in to the Yun's command shell and manipulating the
//     Linux environment.
//
//   - You will need to make some modifications to your Yun to deal with a few 
//     issues with the Yun software:
//
//     - Out of the box the Yun doesn't work with USB HID devices because of a bug
//       in OpenWRT.  Follow the steps by sonnyyu in this thread to enable the 
//       /dev/input/event1 device:
//         http://forum.arduino.cc/index.php?topic=207069.msg1641669#msg1641669
//
//     - You MUST disable the Yun Bridge library completely so the Serial1 connection
//       can be used exclusively by the sketch for communication.  Unfortunately the
//       Bridge library has serious performance problems and is not fast enough to
//       send mouse events between the Yun's two processors.  Follow this thread to
//       comment the ttyATH0 line in /etc/inittab file:
//         http://forum.arduino.cc/index.php?topic=191820.45
//
//       When you are done with this sketch, uncomment that line or else the Bridge
//       library will be unusable in your own sketches.
//
//   - Install the PySerial library on the Yun's Linux processor.  Follow this blog
//     post to install PIP:
//       http://codybonney.com/installing-pip-on-the-arduino-yun/
//
//     Then run the command:
//       pip install pyserial
//
//   - Copy both the EvilMouseYun.py and evdev.py files to an SD card that is
//     mounted on the Yun (/mnt/sda1).
//
//   - Update init script to run EvilMouseYun.py at start up.
//     When you are done using the prank, remove this init setup.  Alternatively
//     you can just log in to the Yun and run python EvilMouseYun.py
//
//   - Copy pages to /mnt/sda/www to support remote control.


// Scale factor applied to mouse X axis.  Value of 1 is normal, -1 is inverted.
char mouseXScale = 1;
// Scale factor applied to mouse Y axis.  Value of 1 is normal, -1 is inverted.
char mouseYScale = 1;
// Scale factor applied to mouse wheel axis.  Value of 1 is normal, -1 is inverted.
char mouseWheelScale = 1;
// True/false if the mouse X and Y axis should transpose their values (this is SUPER disorienting!).
bool mouseXYFlipped = false;
// True/false if mouse left and right buttons should be flipped.
bool mouseLeftRightFlipped = false;
// True/false to enable sticky ball mouse emulation.  When sticky ball emulation is on
// the X and Y axis will randomly refuse to move for short periods of time (to emulate
// an infuriatingly sticky old mechanical ball mouse).
bool emulateStickyBall = false;
// Internal state used by the sticky ball emulation:
bool stickyX = false;
bool stickyY = false;
unsigned long stickyPeriod = 0;

void setup() {
  // Initialize Serial1, the port which is connected to the Linux processor.  The EvilMousePrank.py 
  // script will send mouse events over Serial1 to the sketch.
  Serial1.begin(115200);
  Mouse.begin();
  // Seed the random number generator with noise.
  randomSeed(analogRead(0));
}

void loop() {
  // Check if a command has been sent to the sketch.
  // Each command is 2 bytes in length, the first byte is the command ID and the second
  // is a signed byte value (which might be ignored by some commands).
  if (Serial1.available() >= 2) {
    // Read the command and value.
    char command = Serial1.read();
    char value = Serial1.read();
    // Handle the command.
    switch (command) {
      case 'X': mouseX(value);
                break;
      case 'Y': mouseY(value);
                break;
      case 'W': mouseWheel(value);
                break;
      case 'L': mouseLeft(value);
                break;
      case 'R': mouseRight(value);
                break;
      case 'M': mouseMiddle(value);
                break;
      case 's': emulateStickyBall = true;
                break;
      case 'x': // Invert X axis.
                mouseXScale = -1;
                break;
      case 'y': // Invert Y axis.
                mouseYScale = -1;
                break;
      case 't': mouseXYFlipped = true;
                break;
      case 'f': mouseLeftRightFlipped = true;
                break;
      case 'r': reset();
                break;
    }
  }
  
  // Update sticky ball emulation state.
  if (emulateStickyBall) {
    // Check if sticky ball state should update.
    if (millis() >= stickyPeriod) {
      // Each axis has a 1/4 chance of going sticky.
      int n = random(4);
      if (n == 0) {
        // Set X axis sticky.
        stickyX = true;
        stickyY = false;
      }
      else if (n == 1) {
        // Set Y axis sticky.
        stickyY = true;
        stickyX = false;
      }
      else {
        // Make no axis sticky.
        stickyY = false;
        stickyX = false;
      }
      // Pick a new random period of time between 500-1500ms for the stickyness to last.
      stickyPeriod = millis() + random(500, 1500); 
    }  
  }
}

// Mouse X axis command handler.
void mouseX(char value) {
  if (stickyX) {
    // Do nothing if the axis is stuck.
    return;
  }
  // Send a mouse movement that's scaled and on the appropriate axis.
  if (!mouseXYFlipped) {
    Mouse.move(mouseXScale*value, 0, 0);
  }
  else {
    Mouse.move(0, mouseXScale*value, 0);
  }
}

// Mouse Y axis command handler.
void mouseY(char value) {
  if (stickyY) {
    // Do nothing if the axis is stuck.
    return;
  }
  // Send a mouse movement that's scaled and on the appropriate axis.
  if (!mouseXYFlipped) {
    Mouse.move(0, mouseYScale*value, 0);
  }
  else {
    Mouse.move(mouseYScale*value, 0, 0);
  }
}

// Mouse wheel axis command handler.
void mouseWheel(char value) {
  Mouse.move(0, 0, mouseWheelScale*value);
}

// Mouse left button command handler.
void mouseLeft(char value) {
  int button = mouseLeftRightFlipped ? MOUSE_RIGHT : MOUSE_LEFT;
  if (value == 1) {
    Mouse.press(button);
  }
  else if (value == 0) {
    Mouse.release(button);
  }
}

// Mouse right button command handler.
void mouseRight(char value) {
  int button = mouseLeftRightFlipped ? MOUSE_LEFT : MOUSE_RIGHT;
  if (value == 1) {
    Mouse.press(button);
  }
  else if (value == 0) {
    Mouse.release(button);
  }
}

// Mouse middle button command handler.
void mouseMiddle(char value) {
  if (value == 1) {
    Mouse.press(MOUSE_MIDDLE);
  }
  else if (value == 0) {
    Mouse.release(MOUSE_MIDDLE);
  }
}

// Reset all the state back to normal mouse behavior.
void reset() {
  mouseXScale = 1;
  mouseYScale = 1;
  mouseWheelScale = 1;
  mouseXYFlipped = false;
  mouseLeftRightFlipped = false;
  emulateStickyBall = false;
  stickyX = false;
  stickyY = false;
}



