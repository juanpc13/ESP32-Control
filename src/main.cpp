#include <Arduino.h>
#include <BleGamepad.h> 

BleGamepad bleGamepad("Juan GameCube-Control");

// Sticks Analogos
#define numOfSticks 2
int16_t lastSticks[numOfSticks];
byte stickPins[numOfSticks] = { 34, 35 };
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
  }
}

// Botones Digitales
#define numOfButtons 10
byte lastStates[numOfButtons];
byte buttonPins[numOfButtons] = { 5, 17, 16, 4, 2, 13, 14, 27, 26, 25 };
byte physicalButtons[numOfButtons] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

void setup() {
  Serial.begin(115200);

  // Iniciar Bluetooth
  bleGamepad.begin(numOfButtons);
  bleGamepad.setAutoReport(false);  // Evitar el envio por individual con bleGamepad.sendReport();

  // Estado de los botones para controlarlos en SRAM mas rapido que leerlo
  for (byte currentPinIndex = 0 ; currentPinIndex < numOfButtons ; currentPinIndex++) {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    lastStates[currentPinIndex] = LOW;
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