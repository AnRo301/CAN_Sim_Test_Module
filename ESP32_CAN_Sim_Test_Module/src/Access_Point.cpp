#include <Access_Point.h>
#include <CAN.h>


/* --------------------- Definitions and static variables ------------------ */

WiFiUDP udp;
const char *ssid = "CAN2WLAN_CAN_SIM";
AsyncWebServer server(80);
AsyncWebSocket socket("/ws");

int Baudrate_initial = 9600;
int Baudrate_set = Baudrate_initial;

vector<uint8_t> b_read = {0x00, 0x00, 0x00, 0x00};
vector<uint8_t> receive_identifier = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
vector<uint8_t> receive_data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};

/* --------------------------- Tasks and Functions -------------------------- */

void   Init_AP(){
  if(!SPIFFS.begin()){
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  WiFi.softAP(ssid);

  Serial.print("Access Point IP-Adresse: ");
  Serial.println(WiFi.softAPIP());
}

void sendReceivedCANData(CANFrame canFrame) {

  CANFrame canFrame4UDP;
  vector<uint8_t> id2 ={
         static_cast<uint8_t>(canFrame.identifier[3]),
         static_cast<uint8_t>(canFrame.identifier[2]),
         static_cast<uint8_t>(canFrame.identifier[1]),
         static_cast<uint8_t>(canFrame.identifier[0])};

  canFrame4UDP.identifier = canFrame.identifier; 
  canFrame4UDP.frameLength = canFrame.frameLength; 
  canFrame4UDP.data = canFrame.data; 
  canFrame4UDP.extFlag = canFrame.extFlag; 
  canFrame4UDP.rtrFlag = canFrame.rtrFlag;
  canFrame4UDP.identifier = {static_cast<uint8_t>(canFrame.identifier[3]),
                    static_cast<uint8_t>(canFrame.identifier[2]),
                    static_cast<uint8_t>(canFrame.identifier[1]),
                    static_cast<uint8_t>(canFrame.identifier[0])};

  vector<uint8_t> canFrameVec = CANFrame_to_Vec(canFrame4UDP);
  String canFrameString = CANFrame_to_JSON(canFrame);
      
  size_t canFrameLen = canFrameVec.size();

  BroadcastUDP(canFrameVec, canFrameLen);
  socket.textAll(canFrameString);

  delay(400);
}

void Init_WS(){

  socket.onEvent(onWsEvent);
  server.addHandler(&socket);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    serveFile(request, "/index_CanSim_TestModule.html", "text/html");
  });

  server.on("/style_CanSim_TestModule.css", HTTP_GET, [](AsyncWebServerRequest *request){
    serveFile(request, "/style_CanSim_TestModule.css", "text/css");
  });

  server.on("/script_CanSim_TestModule.js", HTTP_GET, [](AsyncWebServerRequest *request){
    serveFile(request, "/script_CanSim_TestModule.js", "application/javascript");
  });

  server.begin();
}

void serveFile(AsyncWebServerRequest *request, const char *filename, const char *contentType) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    request->send(404, "text/plain", "File not found");
    return;
  }

  String fileContent = file.readString();
  file.close();

  request->send(200, contentType, fileContent);
}


void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
    
    Serial.print("Client (IP: ");
    Serial.print(client->remoteIP());
    Serial.println(") connected.");
 
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
    Serial.println("-----------------------");
 
  } else if(type == WS_EVT_DATA){
 
    Serial.print("Data received: ");
 
    for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
    }
 
    Serial.println();
  }
}

vector<uint8_t> CANFrame_to_Vec(CANFrame canFrame ) {
    std::vector<uint8_t> canFrameVec;

    // Assemble CAN Frame Vector
    canFrameVec.insert(canFrameVec.end(), canFrame.magic.begin(), canFrame.magic.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.version.begin(), canFrame.version.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.framesCnt.begin(), canFrame.framesCnt.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.identifier.begin(), canFrame.identifier.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.frameLength.begin(), canFrame.frameLength.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.data.begin(), canFrame.data.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.extFlag.begin(), canFrame.extFlag.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.rtrFlag.begin(), canFrame.rtrFlag.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.options.begin(), canFrame.options.end());

    return canFrameVec;
}

String vectorToHexString(const std::vector<uint8_t>& vec) {
    String result;
    for (size_t i = 0; i < vec.size(); ++i) {
        // Convert each value to a two-digit hexadecimal string
        result += String(vec[i], HEX);

        // Add a space between every two characters, except for the last character
        if (i < vec.size() - 1) {
            result += " ";
        }
    }
    return result;
}

String CANFrame_to_JSON (CANFrame canFrame){

  DynamicJsonDocument doc(256);

  doc["identifier"] = vectorToHexString(canFrame.identifier);
  doc["frameLength"] = vectorToHexString(canFrame.frameLength);
  doc["data"] = vectorToHexString(canFrame.data);
  doc["extFlag"] = vectorToHexString(canFrame.extFlag);
  doc["rtrFlag"] = vectorToHexString(canFrame.rtrFlag);

  String CANString;
  serializeJson(doc, CANString);

  return CANString;
}

void BroadcastUDP(std::vector<uint8_t> data, size_t len) {
  udp.beginPacket("255.255.255.255", 1060);
  udp.write(data.data(), len);
  udp.endPacket();
}

uint8_t combineDigits(uint8_t num1, uint8_t num2) { // Die letzten 4 Bits jeder Zahl extrahieren 
    uint8_t last4BitsNum1 = num1 & 0x0F; 
    uint8_t last4BitsNum2 = num2 & 0x0F; // Die letzten 4 Bits von num1 nach links verschieben und mit den letzten 4 Bits von num2 kombinieren 
    uint8_t combinedNum = (last4BitsNum1 << 4) | last4BitsNum2; 
    return combinedNum; 
} 

void WS_Handling(){
  socket.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  
    if(type == WS_EVT_DATA){

      if(data[0]==0x00 && data[1]== 0x00 && data[2]== 0x00 && data[3]== 0x00 && data[4]== 0x00 && data[5]== 0x00 && data[6]== 0x00 && data[7]==0x00){
        Serial.begin(Baudrate_initial);
        printf("\nResetSpeed\n");
      }else if(data[0] == 'X'){
        printf("\nSend CAN Frame:");
        printf("\nFirstSign : X\n");

        Serial.print("Data received: ");
        printf("Length: ");
        printf("%08d\n", len);

        
        for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
        }
        
        printf("\n");
        for(int i=0; i < len; i++) {
          printf("0x%02x ", data[i]);
        }
        printf("\n");
        for(int i=0; i < len; i++) {
          cout << data[i];
        }
        printf("\n");

        int start_id = 1;
        int stop_id;
        int start_frame_len;
        int stop_frame_len;
        int start_data;
        int stop_data;
        int pos_extd;
        int pos_rtr;
        int counter = 1;

        for (int j = 1; j < len; j++){
          if(data[j] == 'X' && counter ==3){
            stop_data = j - 1;
            pos_extd = j + 1;
            pos_rtr = j + 3;
            cout << "Stop Data: " << stop_data << endl;
            cout << "Pos extd: "  << pos_extd << endl;
            cout << "Pos rtr: "   << pos_rtr << endl;
            counter++;
          }
          if(data[j] == 'X' && counter == 2){
            stop_frame_len = j - 1;
            start_data = j + 1;
            cout << "Stop Frame Len: " << stop_frame_len << endl;
            cout << "Start Data: " << start_data << endl;
            counter++;
          }
          if(data[j]== 'X' && counter == 1){
            stop_id = j - 1;
            start_frame_len = j + 1;
            cout << "Start Id: " << start_id << endl;
            cout << "Stop Id " << stop_id << endl;
            cout << "Start Frame Len: " << start_frame_len << endl;
            counter++;
          }
   
        }

        int len_id = (stop_id - start_id)+1;

        cout << "len id:" << len_id << endl;

        for(int k=1; k <= stop_id; k++) {
          Serial.print((char) data[k]);
          receive_identifier[8 - len_id + k - 1] = data[k];
        }

        for (int j = 0; j < (8 - len_id); j++)
        {
          receive_identifier[j] = 0x00;
        } 

        cout << receive_identifier[0] << receive_identifier[1] << receive_identifier[2] << receive_identifier[3] << receive_identifier[4] << receive_identifier[5] << receive_identifier[6] << receive_identifier[7] << endl;

        uint8_t digit0 = receive_identifier[0]; 
        uint8_t digit1 = receive_identifier[1]; 
        uint8_t digit2 = receive_identifier[2]; 
        uint8_t digit3 = receive_identifier[3]; 
        uint8_t digit4 = receive_identifier[4]; 
        uint8_t digit5 = receive_identifier[5]; 
        uint8_t digit6 = receive_identifier[6]; 
        uint8_t digit7 = receive_identifier[7];

        uint8_t combined1 = combineDigits(digit0, digit1);
        uint8_t combined2 = combineDigits(digit2, digit3);
        uint8_t combined3 = combineDigits(digit4, digit5);
        uint8_t combined4 = combineDigits(digit6, digit7);
 

        cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combined1) << endl; 
        cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combined2) << endl; 
        cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combined3) << endl; 
        cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combined4) << endl; 

        vector<uint8_t> read_identifier_vector = {combined1, combined2, combined3, combined4};
        uint32_t Identifier_read_uint32 = convertVectorToUInt32(read_identifier_vector); 
        cout << "Converted int value dez: " << Identifier_read_uint32 << endl;

        cout << Identifier_read_uint32 << endl;
        cout << hex << Identifier_read_uint32 << endl;
        
        twai_message_t message;
        message.identifier = Identifier_read_uint32;
        message.rtr = data[pos_rtr];
        message.extd = data[pos_extd];

        if (data[start_frame_len] == '1'){
          message.data_length_code = 0x01;
          cout << "0x01" << endl;
        }else if(data[start_frame_len] == '2'){
          message.data_length_code = 0x02;
          cout << "0x02" << endl;
        }else if(data[start_frame_len] == '3'){
          message.data_length_code = 0x03;
          cout << "0x03" << endl;
        }else if(data[start_frame_len] == '4'){
          message.data_length_code = 0x04;
          cout << "0x04" << endl;
        }else if(data[start_frame_len] == '5'){
          message.data_length_code = 0x05;
         cout << "0x05" << endl; 
        }else if(data[start_frame_len] == '6'){
          message.data_length_code = 0x06;
          cout << "0x06" << endl;
        }else if(data[start_frame_len] == '7'){
          message.data_length_code = 0x07;
          cout << "0x07" << endl;
  	    }else if(data[start_frame_len] == '8'){
          message.data_length_code = 0x08;
          cout << "0x08" << endl;
        }

        printf("Data Length Code: %02x\n", message.data_length_code);

        //Data
        int len_data = (stop_data - start_data)+1;

        cout << "len data (digits):" << len_data << endl;

        int pos;

        if(len_data%2 == 0){
          pos = 0;
          printf("gerade\n");
        }else{
          receive_data[0] = 0x00;
          cout << receive_data[0];
          pos = 1;
          printf("ungerade\n");
        }
        

        for(int k=start_data; k <= stop_data; k++) {
           Serial.print((char) data[k]);
           receive_data[pos] = data[k];
           cout << receive_data[pos];
           pos++;
        }
        cout << "\n";
 
        uint8_t digitdata0  = receive_data[0]; 
        uint8_t digitdata1  = receive_data[1];
        uint8_t combineddata1 = combineDigits(digitdata0, digitdata1);
        cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata1) << endl; 
        message.data[0] = combineddata1;

        if(message.data_length_code > 0x01){
          uint8_t digitdata2  = receive_data[2]; 
          uint8_t digitdata3  = receive_data[3]; 
          uint8_t combineddata2 = combineDigits(digitdata2, digitdata3);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata2) << endl;
          message.data[1] = combineddata2;
        }

        if(message.data_length_code > 0x02){
          uint8_t digitdata4  = receive_data[4]; 
          uint8_t digitdata5  = receive_data[5];
          uint8_t combineddata3 = combineDigits(digitdata4, digitdata5);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata3) << endl;
          message.data[2] = combineddata3;
        }

        if(message.data_length_code > 0x03){
          uint8_t digitdata6  = receive_data[6]; 
          uint8_t digitdata7  = receive_data[7]; 
          uint8_t combineddata4 = combineDigits(digitdata6, digitdata7);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata4) << endl; 
          message.data[3] = combineddata4;
        }
        
        if(message.data_length_code > 0x04){
          uint8_t digitdata8  = receive_data[8]; 
          uint8_t digitdata9  = receive_data[9]; 
          uint8_t combineddata5 = combineDigits(digitdata8, digitdata9);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata5) << endl;
          message.data[4] = combineddata5;
        }

        if(message.data_length_code > 0x05){
          uint8_t digitdata10 = receive_data[10]; 
          uint8_t digitdata11 = receive_data[11];
          uint8_t combineddata6 = combineDigits(digitdata10, digitdata11);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata6) << endl;
          message.data[5] = combineddata6;
        }

        if(message.data_length_code > 0x06){
          uint8_t digitdata12 = receive_data[12]; 
          uint8_t digitdata13 = receive_data[13];
          uint8_t combineddata7 = combineDigits(digitdata12, digitdata13);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata7) << endl; 
          message.data[6] = combineddata7;
        }
        
        if(message.data_length_code == 0x07){
          uint8_t digitdata14 = receive_data[14]; 
          uint8_t digitdata15 = receive_data[15]; 
          uint8_t combineddata8 = combineDigits(digitdata14, digitdata15);
          cout << "Die kombinierten Zahlen sind: 0x" << hex << static_cast<uint16_t>(combineddata8) << endl;
          message.data[7] = combineddata8;
        }

        twai_transmit_task(message);

      } else{

      Serial.print("Data received: ");
      printf("Length: ");
      printf("%08d\n", len);

      for(int i=0; i < len; i++) {
          Serial.print((char) data[i]);
          b_read[8 - len + i] = data[i];
      }

        Change_CAN_Speed(data);
      }
    }

 });
}