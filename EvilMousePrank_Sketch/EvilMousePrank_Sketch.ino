// Evil Mouse Prank for Arduino Yun
// Arduino Yun Sketch
// Copyright 2014 Tony DiCola (tony@tonydicola.com)
// Released under an MIT license (http://opensource.org/licenses/MIT)

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
  // NOTE: When the Yun is booting up the Serial connection will be flooded with information from
  // from the Linux boot process.  To work around this wait 90 seconds, clearing the serial buffer
  // every second.  This is an ugly hack but unfortunately necessary.  See this thread for more
  // information:
  //   http://forum.arduino.cc/index.php?topic=191820.msg1436262#msg1436262
  Serial1.begin(115200);
  for (int i = 0; i < 90; ++i) {
    delay(1000);
    while (Serial1.available() > 0) {
      Serial1.read();
    }
  }
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



