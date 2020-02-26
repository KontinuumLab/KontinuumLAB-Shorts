int breathPin = 25;
int lipPin = 16;

int breathMin = 300;
int breathMax = 120;

int lipMin = 310;
int lipMax = 190;

int breathReading;
int lipReading;
int lastLipReading;
int lipMapped;
int lipPos;
int lastLipMapped;

int breathOut;
int lastBreathOut;

int transpose;
int harmonics[5] = {0, 7, 12, 16, 19};

int baseNote = 60;

int currentNote;
int lastNote;


// Error value for the exponential filter:
float error;

void setup() {
  Serial.begin(9600);

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
  lastBreathOut = breathOut;
  lastNote = currentNote;
  lastLipMapped = lipMapped;
  
  breathReading = analogRead(breathPin);
  breathOut = map(breathReading, breathMin, breathMax, 0, 127);
  if(breathOut < 0){
    breathOut = 0;
  }
  else if(breathOut > 127){
    breathOut = 127;
  }
  

  lipReading = touchRead(lipPin);
  lipMapped = map(lipReading, lipMin, lipMax, 0, 127);
  if(lipMapped < 0){
    lipMapped = 0;
  }
  else if(lipMapped > 127){
    lipMapped = 127;
  }
  lipMapped = expFilter(lipMapped, lastLipMapped, 128, 0.001);
  
  lipPos = map(lipMapped, 0, 127, 0, 4);
  
  transpose = harmonics[lipPos];
  currentNote = baseNote + transpose; // + valve calculation
  
  Serial.print(transpose);
  Serial.print(" - ");
  Serial.println(breathOut);

  if(breathOut != 0){
    if(lastBreathOut != breathOut){
      usbMIDI.sendControlChange(2, breathOut, 1);
    }
    if(lastBreathOut == 0){
      usbMIDI.sendNoteOn(currentNote, 127, 1);
    }
      if(lastNote != currentNote){
      usbMIDI.sendNoteOn(lastNote, 0, 1);
      usbMIDI.sendNoteOn(currentNote, 127, 1);
    }
  }
  else if(lastBreathOut != 0){
    usbMIDI.sendNoteOn(currentNote, 0, 1);
    usbMIDI.sendNoteOn(lastNote, 0, 1);
  }


  
//  Serial.print(analogRead(breathPin));
//  Serial.print(" - ");
//  Serial.println(touchRead(lipPin));
//  delay(20);

}
