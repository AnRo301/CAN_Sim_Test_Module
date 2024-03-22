#include <CAN.h>
#include <Access_Point.h>


/* --------------------- Definitions and static variables ------------------ */

twai_message_t message;
twai_message_t message_receive;
int data_hex = 32;
twai_general_config_t g_config;
twai_timing_config_t t_config;
twai_filter_config_t f_config;


/* --------------------------- Tasks and Functions -------------------------- */

void Init_CAN(){

  //Initialize configuration structures using macro initializers
  g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_37, GPIO_NUM_36, TWAI_MODE_NORMAL);   //Platine
  t_config = TWAI_TIMING_CONFIG_25KBITS();
  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    printf("Driver installed\n");
  }
  else
  {
    printf("Failed to install driver\n");
    return;
  }
  // Start TWAI driver
  if (twai_start() == ESP_OK)
  {
    printf("Driver started\n");
  }
  else
  {
    printf("Failed to start driver\n");
    return;
  }
}


void twai_receive_task()
{
    printf("twai_receive_task: while Loop\n");

    twai_message_t message;

    if(twai_receive(&message, 1000) == ESP_OK){
    printf("0x%03x :", message.identifier);
    printf("0x%03x :", message.data_length_code);
    if (!(message.rtr))
    {
      for (int i = 0; i < message.data_length_code; i++)
      {
        printf("0x%02x ", message.data[i]);
      }
      printf("\n");
    }

    //uint32_t x = message.identifier;

    uint8_t d = (message.identifier >> 24) & 0xFF;  //a
    uint8_t c = (message.identifier >> 16)& 0xFF;   //b
    uint8_t b = (message.identifier >> 8) & 0xFF;   //c
    uint8_t a = message.identifier & 0xFF;          //d
    
    vector<uint8_t> identifier = {
         static_cast<uint8_t>(a),
         static_cast<uint8_t>(b),
         static_cast<uint8_t>(c),
         static_cast<uint8_t>(d)};

     vector<uint8_t> frameLength = {message.data_length_code};

     vector<uint8_t> data = {
         static_cast<uint8_t>(message.data[0]),
         static_cast<uint8_t>(message.data[1]),
         static_cast<uint8_t>(message.data[2]),
         static_cast<uint8_t>(message.data[3]),
         static_cast<uint8_t>(message.data[4]),
         static_cast<uint8_t>(message.data[5]),
         static_cast<uint8_t>(message.data[6]),
         static_cast<uint8_t>(message.data[7])};

    
    vector<uint8_t> extFlag = {0x00};
    vector<uint8_t> rtrFlag = {0x00};

    if ((unsigned char)message.extd == 1)
    {
      extFlag = {0x01};
    } else {
      extFlag = {0x00};
    }

    if((unsigned char)message.rtr == 1)
    {
      rtrFlag = {0x01};
    } else {
      rtrFlag = {0x00};
    }

    CANFrame CanFrameReceived;
    
    CanFrameReceived.identifier = identifier;
    CanFrameReceived.frameLength = frameLength;
    CanFrameReceived.data = data;
    CanFrameReceived.extFlag = extFlag;
    CanFrameReceived.rtrFlag = rtrFlag;

    sendReceivedCANData(CanFrameReceived);

    }else{
      printf("Failed to receive message\n");
      return;
    }
}

void twai_receive_task_module1(twai_message_t message2)
{
    printf("twai_receive_task: while Loop\n");
    printf("Message received\n");
    
    printf("0x%03x :", message2.identifier);
    printf("0x%03x :", message2.data_length_code);
    if (!(message2.rtr))
    {
      for (int i = 0; i < message2.data_length_code; i++)
      {
        printf("0x%02x ", message2.data[i]);
      }
      printf("\n");
    }

    CANFrame CanFrameReceived;

    uint8_t a = (message2.identifier >> 24) & 0xFF;
    uint8_t b = (message2.identifier >> 16)& 0xFF;
    uint8_t c = (message2.identifier >> 8) & 0xFF;
    uint8_t d = message2.identifier & 0xFF;
    
    std::vector<uint8_t> identifier = {
         static_cast<uint8_t>(a),
         static_cast<uint8_t>(b),
         static_cast<uint8_t>(c),
         static_cast<uint8_t>(d)};

    std::vector<uint8_t> frameLength = {message2.data_length_code};

    std::vector<uint8_t> data = {
         static_cast<uint8_t>(message2.data[0]),
         static_cast<uint8_t>(message2.data[1]),
         static_cast<uint8_t>(message2.data[2]),
         static_cast<uint8_t>(message2.data[3]),
         static_cast<uint8_t>(message2.data[4]),
         static_cast<uint8_t>(message2.data[5]),
         static_cast<uint8_t>(message2.data[6]),
         static_cast<uint8_t>(message2.data[7])};

    
    std::vector<uint8_t> extFlag = {0x00};
    std::vector<uint8_t> rtrFlag = {0x00};

    if ((unsigned char)message2.extd == 1){
      extFlag = {0x01};
    } else{
      extFlag = {0x00};
    }

    if((unsigned char)message2.rtr == 1){
      rtrFlag = {0x01};
    }else{
      rtrFlag = {0x00};
    }

    CanFrameReceived.identifier = identifier;
    CanFrameReceived.frameLength = frameLength;
    CanFrameReceived.data = data;
    CanFrameReceived.extFlag = extFlag;
    CanFrameReceived.rtrFlag = rtrFlag;

     sendReceivedCANData(CanFrameReceived);
}

void print_transmitted_message(twai_message_t message){

    printf("Message queued for transmission\n");
    // Print transmitted message
    printf("Transmitting Message\n");
    cout << "ID (hex): " << hex << message.identifier;
    cout << "  -  DataLengthCode: " << hex << message.data_length_code;
    cout << "  -  Data: " << hex << message.data[0] << message.data[1] << message.data[2] << message.data[3] << message.data[4] << message.data[5] << message.data[6] << message.data[7] << endl;

    printf("0x%03x :", message.identifier);
    printf("0x%03x :", message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++)
    {
      printf("0x%02x ", message.data[i]);
      }
      cout << endl;
}


void twai_transmit_task(twai_message_t message){
  
  // Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
  {
    printf("Message queued for transmission\n");
  } else {
    printf("Failed to queue message for transmission\n");
  }

  printf("Transmitting Message\n");
        
  printf("0x%04x :", message.identifier);
  printf("0x%02x :", message.data_length_code);
  for (int i = 0; i < message.data_length_code; i++)
  {
    printf("0x%02x ", message.data[i]);
  }
  cout << endl;

  vTaskDelay(2000);
}

void twai_cyclic_send_task(){
  // Build message to send via CAN Bus
  message.identifier = 0x123;
  message.rtr = 0;
  message.extd = 1;
  message.data_length_code = 8;
  
  for (int i = 0; i < message.data_length_code; i++){
      message.data[i] = data_hex;
      data_hex++;
      if (data_hex == 120 || data_hex == 0x78)
      {
        data_hex = 32;
      }
  }


  if (twai_receive(&message_receive, pdMS_TO_TICKS(10000)) == ESP_OK)
  {
      // Receive Message
      twai_receive_task_module1(message_receive);
  }
  else if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
  {
      // Transmit Message
      print_transmitted_message(message);
  }
  else
  {
      printf("Failed to queue message for transmission\n");
      printf("No message received\n");
  }

}


int convertVectorToInt(const vector<uint8_t>& vec) { 
    
    int result = 0;
    
    for (size_t i = 0; i < vec.size(); i++) { 
        result = (result << 8) | vec[i]; 
    } 
        
    return result; 
}


int convertVectorToUInt32(const vector<uint8_t>& vec) { 
    
    uint32_t result = 0;
    
    for (size_t i = 0; i < vec.size(); i++) { 
        result = (result << 8) | vec[i]; 
    } 
        
    return result; 
}


int ConvertHexToInt(int b){
  int dez= 9600;

  if(b  == 768){
    dez = 300;}
  else if (b == 4608){
    dez = 1200;}
  else if(b == 9216){
    dez=2400;}
  else if (b == 18432){
    dez = 4800;}
  else if (b == 38400){
    dez = 9600;}
  else if (b == 102912){
    dez=19200;}
  else if (b == 230400){
    dez = 38400;}
  else if (b == 357888){
    dez = 57600;}
  else if (b == 477312){
    dez=74880;}
  else if (b == 1135104){
      dez = 115200;}
  else if (b == 2294784){
    dez = 230400;}
  else if (b == 2424832){
    dez=250000;}

  printf("\nDez-Wert:%8d\n", dez);
  return dez;
}


int Change_Baudrate(int  baudrate_r){

  int bset;

  if(baudrate_r == 300){
    Serial.begin(300);
    bset = baudrate_r;}
  else if(1200){
    Serial.begin(1200);
    bset = baudrate_r;}
  else if(2400){
    Serial.begin(2400);
    bset = baudrate_r;}
  else if(4800){
    Serial.begin(4800);
    bset = baudrate_r;}
  else if(9600){
      Serial.begin(9600);
      bset = baudrate_r;}
  else if(19200){
      Serial.begin(19200);
      bset = baudrate_r;}
  else if(38400){
      Serial.begin(38400);
      bset = baudrate_r;}
  else if(57600){
      Serial.begin(57600);
      bset = baudrate_r;}
  else if(74880){
      Serial.begin(74880);
      bset = baudrate_r;}
  else if(115200){
      Serial.begin(115200);
      bset = baudrate_r;}
  else if(230400){
      Serial.begin(230400);
      bset = baudrate_r;}
  else if(250000){
      Serial.begin(250000);
      bset = baudrate_r;}
  else {
      Serial.begin(9600);
      bset = 9600;
  }
  
  printf("\nBaudrate changed to %8x\n", bset);
  return bset;
}

void Change_CAN_Speed(uint8_t *data){

  if(data[0] == '1' && data[1] == 'M'){
    t_config = TWAI_TIMING_CONFIG_1MBITS();
    printf("\nCAN-Speed changed to 1Mbit/s\n");
  } else if(data[0] == '8'){
    t_config = TWAI_TIMING_CONFIG_800KBITS();
    printf("\nCAN-Speed changed to 800Kbit/s\n");
  } else if(data[0] == '5' && data[2] == '0'){
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    printf("\nCAN-Speed changed to 500Kbit/s\n");
  } else if(data[0] == '2' && data[2] == '0'){
    t_config = TWAI_TIMING_CONFIG_250KBITS();
    printf("\nCAN-Speed changed to 250Kbit/s\n");
  } else if(data[0] == '1' && data[1] == '2'){
    t_config = TWAI_TIMING_CONFIG_125KBITS();
    printf("\nCAN-Speed changed to 125Kbit/s\n");
  } else if(data[0] == '1' && data[2] == '0'){
    t_config = TWAI_TIMING_CONFIG_100KBITS();
    printf("\nCAN-Speed changed to 100Kbit/s\n");
  } else if(data[0] == '5' && data[1] == '0'){
    t_config = TWAI_TIMING_CONFIG_50KBITS();
    printf("\nCAN-Speed changed to 50Kbit/s\n");
  } else if(data[0] == '2' && data[1] == '5'){
    t_config = TWAI_TIMING_CONFIG_25KBITS();
    printf("\nCAN-Speed changed to 25Kbit/s\n");
  }else{
    t_config = TWAI_TIMING_CONFIG_25KBITS();
    printf("\nDefault CAN-Speed\n");
  }

  //Stop the TWAI driver
  if (twai_stop() == ESP_OK) {
    printf("Driver stopped\n");
  } else {
    printf("Failed to stop driver\n");
    return;
  }

  //Uninstall the TWAI driver
  if (twai_driver_uninstall() == ESP_OK) {
    printf("Driver uninstalled\n");
  } else {
    printf("Failed to uninstall driver\n");
    return;
  }

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    printf("Driver installed\n");
  }
  else
  {
    printf("Failed to install driver\n");
    return;
  }
  // Start TWAI driver
  if (twai_start() == ESP_OK)
  {
    printf("Driver started\n");
  }
  else
  {
    printf("Failed to start driver\n");
    return;
  }
}