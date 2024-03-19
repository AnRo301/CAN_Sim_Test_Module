#include <Arduino.h>
#include <iostream>
#include <vector>
#include <driver/gpio.h>
#include <sstream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "driver/twai.h"

#include "Access_Point.h"
#include "CAN.h"

using namespace std;

  
/* --------------------------- Tasks and Functions -------------------------- */

void setup()
{
  Serial.begin(9600);

  Init_AP();    //Init Access Points

  Init_WS();    // WebSocket-Handling

  Init_CAN();   //CAN

}

void loop()
{
  twai_cyclic_send_task();

  WS_Handling();

  vTaskDelay(1000);  
}
