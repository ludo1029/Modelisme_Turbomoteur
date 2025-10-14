#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "ui.h"
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <math.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// --- Variables podomètre ---
const float threshold = 1.2;     
const int bufferLength = 15;     
float buffer[bufferLength];
int bufferIndex = 0;
int stepCount = 0;
bool stepDetected = false;
const unsigned long debounceDelay = 300; 
unsigned long lastStepTime = 0;

char buf[16];

void setup()
{       
    LCD_Init();
    Lvgl_Init();
    ui_init();

    Wire.begin(5, 4); // SDA, SCL

    if (!accel.begin()) {
        Serial.println("Erreur : ADXL345 non detecte !");
        while (1);
    }
    accel.setRange(ADXL345_RANGE_2_G);

    Serial.begin(115200);
}

void loop()
{
    // Lire les valeurs de l'accéléromètre
    sensors_event_t event;
    accel.getEvent(&event);

    float accMag = sqrt(event.acceleration.x*event.acceleration.x +
                        event.acceleration.y*event.acceleration.y +
                        event.acceleration.z*event.acceleration.z);

    // Stocker dans le buffer
    buffer[bufferIndex] = accMag;
    bufferIndex = (bufferIndex + 1) % bufferLength;

    // Calculer la moyenne
    float avgMag = 0;
    for (int i = 0; i < bufferLength; i++) avgMag += buffer[i];
    avgMag /= bufferLength;

    unsigned long now = millis();
    if (accMag > (avgMag + threshold) && !stepDetected && (now - lastStepTime) > debounceDelay) {
        stepCount++;
        stepDetected = true;
        lastStepTime = now;

        Serial.print("Step count: ");
        Serial.println(stepCount);
    } else if (accMag <= (avgMag + threshold)) {
        stepDetected = false;
    }

    // Mettre à jour le label 9 avec le nombre de pas
    snprintf(buf, sizeof(buf), "%d", stepCount);
    lv_label_set_text(ui_m__parcourue, buf);

    // Rafraîchir l'affichage LVGL
    Timer_Loop();
    delay(50);
}
