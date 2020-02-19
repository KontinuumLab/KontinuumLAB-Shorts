
int breathPin = 25;

int lowLipPin = 16;
int lowLipRead;
int lowLipVal;
int lastLowLipVal;
int lowLipMin = 130;
int lowLipMax = 150;

int upLipPin = 17;
int upLipVal;
int lastUpLipVal;
int upLipThreshold = 200;

int LEDpin = 1;
int tonguePin = 24;
int tongueRead;
int tongueVal;
int lastTongueVal;
int tongueMin = 95;
int tongueMax = 120;

int breathMin = 300;
int breathMax = 100;
int breathRaw;
int lastBreathRaw;
int breathOut;
int lastBreathOut;


// Which MIDI channel to send on:
int MIDIchannel = 1;


// Error value for the exponential filter:
float error;



void setup() {
  Serial.begin(9600);
  pinMode(LEDpin, OUTPUT);

}


// Helper function for the exponential filter function:
float snapCurve(float x){
  float y = 1.0 / (x + 1.0);
  y = (1.0 - y) * 2.0;
  if(y > 1.0) {
    return 1.0;
  }
  return y;
}

// Main exponential filter function. Input "snapMult" = speed setting. 0.001 = slow / 0.1 = fast:
int expFilter(int newValue, int lastValue, int resolution, float snapMult){
  unsigned int diff = abs(newValue - lastValue);
  error += ((newValue - lastValue) - error) * 0.4;
  float snap = snapCurve(diff * snapMult);
  float outputValue = lastValue;
  outputValue  += (newValue - lastValue) * snap;
  if(outputValue < 0.0){
    outputValue = 0.0;
  }
  else if(outputValue > resolution - 1){
    outputValue = resolution - 1;
  }
  return (int)outputValue;
}

void loop() {
  int i;

  
  // Save previous breath sensor raw value, then read and filter new value:
  lastBreathRaw = breathRaw;
  breathRaw = analogRead(breathPin);
  breathRaw = expFilter(breathRaw, lastBreathRaw, 1024, 0.005);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreathOut = breathOut;
  breathOut = map(breathRaw, breathMin, breathMax, 0, 127);
//  Serial.println(breathOut);

  // Limit output to MIDI range:
  if(breathOut < 0){
    breathOut = 0;
  }
  else if(breathOut > 127){
    breathOut = 127;
  }
//  Serial.println(breathOut);
  // If breath sensor output value has changed: 
  if(breathOut != lastBreathOut){
//    Serial.println(breathOut);
    // Send CC2 volume control:
    usbMIDI.sendControlChange(2, breathOut, MIDIchannel);
    // If breath sensor recently DEactivated, send note off message:
    if(breathOut == 0){
      usbMIDI.sendControlChange(123, 0, MIDIchannel);
    }
    // Else if breath sensor recently activated, send note on message:
    else if(breathOut != 0 && lastBreathOut == 0){
      usbMIDI.sendNoteOn(65, 127, MIDIchannel);
    }
  }
// Now for the lip:
  lastLowLipVal = lowLipVal;
  lowLipRead = touchRead(lowLipPin);
  lowLipVal = map(lowLipRead, lowLipMin, lowLipMax, 60, 127);
  if(lowLipVal < 60){
    lowLipVal = 60;
  }
  else if(lowLipVal > 127){
    lowLipVal = 127;
  }
  lowLipVal = expFilter(lowLipVal, lastLowLipVal, 128, 0.005);
  Serial.println(lowLipVal);
  if(lowLipVal != lastLowLipVal){
    usbMIDI.sendControlChange(112, lowLipVal, MIDIchannel);
  }

}
