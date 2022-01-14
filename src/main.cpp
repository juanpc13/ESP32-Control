#include <Arduino.h>
#include <BleGamepad.h>

#define enableSlider1       false
#define enableSlider2       false

BleGamepad bleGamepad("Juan GameCube-Control");

// Sticks Analogos
#define numOfSticks 6
int16_t lastSticks[numOfSticks];
byte stickPins[numOfSticks] = { 33, 32, 34, 35, 36, 39 };
void setAxe(uint8_t axe, int16_t value){
  switch(axe){
    case 0:
      bleGamepad.setX(value);
      break;
    case 1:
      bleGamepad.setY(value);
      break;
    case 2:
      bleGamepad.setRX(value);
      break;
    case 3:
      bleGamepad.setRY(value);
      break;
    case 4:
      bleGamepad.setZ(value);
      break;
    case 5:
      bleGamepad.setRZ(value);
      break;
  }
}

// Botones Digitales
#define numOfButtons 8
byte lastStates[numOfButtons];
byte buttonPins[numOfButtons] = { 27, 14, 25, 26, 12, 15, 2, 13 };
byte physicalButtons[numOfButtons] = { 1, 2, 3, 4, 5, 6, 7, 8 };

// Botones Hat Digitales
#define numOfHats 1
byte previousHatStates[numOfHats*4];
byte currentHatStates[numOfHats*4];
byte hatPins[numOfHats*4] = { 4, 17, 16, 5 };

void setup() {
  //Serial.begin(115200);

  // Iniciar Bluetooth
  bleGamepad.begin(numOfButtons, numOfHats);
  bleGamepad.setAutoReport(false);  // Evitar el envio por individual con bleGamepad.sendReport();

  // Estado de los botones para controlarlos en SRAM mas rapido que leerlo
  for (byte currentPinIndex = 0 ; currentPinIndex < numOfButtons ; currentPinIndex++) {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    lastStates[currentPinIndex] = LOW;
  }

  // Setup Hat Switches
  for (byte currentPinIndex = 0 ; currentPinIndex < numOfHats*4 ; currentPinIndex++){
    pinMode(hatPins[currentPinIndex], INPUT_PULLUP);
    previousHatStates[currentPinIndex] = HIGH;
    currentHatStates[currentPinIndex] =  HIGH;
  }

}

void loop() {
  if(bleGamepad.isConnected()) {

    // Si la comparacion anterior con la actual difiere, cambiar y guardar
    for (byte currentIndex = 0 ; currentIndex < numOfButtons ; currentIndex++){
      int last = lastStates[currentIndex];
      int current = digitalRead(buttonPins[currentIndex]);      
      // difiere del actual con el ultimo
      if(current != last){
        // cambiar el estado
        if(current == HIGH){
          bleGamepad.release(physicalButtons[currentIndex]);
        }else{
          bleGamepad.press(physicalButtons[currentIndex]);
        }
        // guardarlo
        lastStates[currentIndex] = current;
      }
    }

    // Update hat switch pin states
    for (byte currentHatPinsIndex = 0 ; currentHatPinsIndex < numOfHats*4 ; currentHatPinsIndex++){
      currentHatStates[currentHatPinsIndex]  = digitalRead(hatPins[currentHatPinsIndex]);
    }
    // Update hats
    signed char hatValues[4] = { 0, 0, 0, 0 };    
    for (byte currentHatIndex = 0 ; currentHatIndex < numOfHats ; currentHatIndex++){
      signed char hatValueToSend = 0;
      for (byte currentHatPin = 0 ; currentHatPin < 4 ; currentHatPin++){
        // Check for direction
        if(currentHatStates[currentHatPin + currentHatIndex*4] == LOW){
          hatValueToSend = currentHatPin * 2 + 1;
          // Account for last diagonal
          if(currentHatPin == 0){
            if(currentHatStates[currentHatIndex*4 + 3] == LOW){
              hatValueToSend = 8 ;
              break;
            }
          }          
          // Account for first 3 diagonals
          if(currentHatPin < 3){
            if(currentHatStates[currentHatPin + currentHatIndex*4 + 1] == LOW){
              hatValueToSend += 1;
              break;
            }
          }
        }
      }
      hatValues[currentHatIndex] = hatValueToSend;
    }
    // Set hat values
    bleGamepad.setHats(hatValues[0], hatValues[1], hatValues[2], hatValues[3]);

    // Agregando Sticks Analogos
    for (byte currentIndex = 0 ; currentIndex < numOfSticks ; currentIndex++){
      int16_t last = lastSticks[currentIndex];
      int16_t read = analogRead(stickPins[currentIndex]);
      int16_t current = map(read, 0, 4095, 32737, -32737);

      // difiere del actual con el ultimo
      if(current != last){
        // Asignar
        setAxe(currentIndex, current);
        // Guardar
        lastSticks[currentIndex] = current;
      }

    }

    // Enviar toda el estado del control
    bleGamepad.sendReport();
  }
}