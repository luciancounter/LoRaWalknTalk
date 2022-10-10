//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

 

//packet counter
int counter = 0;
String message;
String outgoing;    
byte localAddress = 0xFF;     // address of this device
byte destination = 0xcc;      // destination to send to
int Letter_Entered =0;
int lft =0;
int But =0;
int Transmit =0;
int tog =2;
int New_Position=0;
int Old_Position=0;

// String variable holding the text to transmit
  String To_Transmit="";

// Length of the text to transmit
  int To_Transmit_Length=0;

int key =1;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

char Letters[28]="ABCDEFGHIJKLMNOPQRSTUVWXYZ_";


void Add_To_String(){

  Letter_Entered=1;
  
}

void ButtonLeft(){
  lft = 1;
}

void ButtonRight(){
  But = 1;
}

void Triger_Transmit(){
  
  Transmit=1;

} 
void Toggle()
  {
  
    tog++;
  }



void Highlight_letter(int New_Pos, int Old_Pos){
  // When position changes from Old_Pos to New_Pos
  // Draw the inverse rect in the Old_pos to deactivate  the highlight in the old spot
  // Draw the inverse rect to Highlite the new spot
  int X_pos;
  int Y_pos;
  
  // Calculate X and Y coordinates of the New_Pos on the letter board
  X_pos=New_Pos - ((int)New_Pos/9)*9;
  Y_pos=(int)New_Pos/9;
  // Displaying Inverse rect
  display.fillRect(X_pos*12+2*X_pos, Y_pos*16 +16, 12, 16, SSD1306_INVERSE);
  
  // Calculate X and Y coordinates of the Old_Pos on the letter board
  X_pos=Old_Pos - ((int)Old_Pos/9)*9;
  Y_pos=(int)Old_Pos/9;
  // Displaying Inverse rect
  display.fillRect(X_pos*12+2*X_pos, Y_pos*16 +16, 12, 16, SSD1306_INVERSE);
  
  display.display();
}

void sendMessage(String To_Transmit) {
  Serial.print("Message: ");
    Serial.println(To_Transmit);
    String result = To_Transmit;
  int transformed_index, char_index, char_ascii;
  for(int i = 0; i < To_Transmit.length(); i++) {
    char_ascii = (int) To_Transmit[i];
    char_index = (int) char_ascii % 32;
    
    if(char_ascii != 32) {
      transformed_index = (char_index + key) % 26;
      transformed_index = transformed_index % 26 == 0 ? 26 : transformed_index;
      transformed_index += 96;
      if(char_ascii < 96) transformed_index -= 32;
    } else
      transformed_index = char_ascii;
      result[i] = transformed_index;
  }

    sendMessage(result);
    Serial.println("Sending " + result);

  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(counter);                 // add message ID
  LoRa.write(result.length());        // add payload length
  LoRa.print(result);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  counter++;                           // increment message ID
}


void setup() {

  //initialize Serial Monitor

  Serial.begin(115200);
  attachInterrupt(32,Add_To_String, FALLING);
  attachInterrupt(33,Triger_Transmit, FALLING);
  attachInterrupt(35,ButtonRight, FALLING);
  attachInterrupt(15,ButtonLeft, FALLING);
  attachInterrupt(25, Toggle, FALLING);

  //reset OLED display via software

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

 

  //initialize OLED

  Wire.begin(OLED_SDA, OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

 

  
 

  Serial.println("LoRa Sender Test");

 

  //SPI LoRa pins

  SPI.begin(SCK, MISO, MOSI, SS);

  //setup LoRa transceiver module

  LoRa.setPins(SS, RST, DIO0);

 

  if (!LoRa.begin(BAND)) {

    Serial.println("Starting LoRa failed!");

    while (1);

  }

 
  Serial.println("LoRa Initializing OK!");
  
  delay(2000);

}

 

void loop() 
{
  display.clearDisplay();
  while(tog %2 ==0)
  {
    
  
  // Display Letter Board 3 rows 9 character in each row 
  display.setTextSize(2);            
  display.setTextColor(SSD1306_WHITE);       
  for (int j=0; j<3;j++){
  for (int i=0; i<9;i++){
     display.setCursor(i*12+2*i+1,j*16+17);           
     display.println(Letters[i+j*9]);
     //display.fillRect(i*12+2*i, j*16 +16, 12, 16, SSD1306_INVERSE);
     display.display();
  }
    
  // Display filled in rect in the top section of the display when To_Transfer would be output
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
  // Highlight character A by displaying Inverse rect at first position
  display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);
  display.display();

  }

if(lft == 1){
    New_Position--;

    if (Old_Position!=New_Position){
       Highlight_letter (New_Position,Old_Position);
       Old_Position=New_Position;
    }
    lft =0;
    delay(100);
  }
 

  if(But == 1){
      New_Position++;
      But = 0;

      if (Old_Position!=New_Position){
       Highlight_letter (New_Position,Old_Position);
       Old_Position=New_Position;
    }
    delay(100);
  }
    
  // Wait 100 loop cycles before checking if the new letter was entered
  
  // If letter was entered add the letter to To_Transmit and output it on the display
  if (Letter_Entered==1 ){
      To_Transmit=To_Transmit + Letters[New_Position];
      To_Transmit_Length++;
      display.setCursor(3,0);
      display.setTextColor(BLACK     );
      display.fillRect(0, 0, 128, 15, SSD1306_WHITE);
      display.println(To_Transmit);
      display.display();
      Letter_Entered=0;
      delay(100);
  }
  // If Transmit button was pressed start morse code transmition
  
  if (Transmit==1){
    sendMessage(To_Transmit);
    Serial.print("\n" + To_Transmit);
    To_Transmit="";
    display.setTextColor(BLACK);
    display.fillRect(0,0,128,15,SSD1306_WHITE);\
    display.println(To_Transmit);
    display.display();
    Transmit=0;
    delay(100);
  }
  
    

  //Send LoRa packet to receive

    

  }
  
  while(tog%2 !=0){

display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("Message Number:");
    display.setCursor(100,0);
    display.print(counter);
    display.setCursor(0,10);
    display.print("Message:");
    display.setCursor(55,10);
    display.print(To_Transmit);      
    display.display();  
  onReceive(LoRa.parsePacket());
  

  }

}


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                           // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xCC) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

    String dec = incoming;
  int transformed_index, char_index, char_ascii;
  for(int i = 0; i < incoming.length(); i++) {
    char_ascii = (int) incoming[i];
    char_index = (int) char_ascii % 32;
    
    if(char_ascii != 32) {
      transformed_index = (char_index - key + 26) % 26;
      transformed_index = transformed_index % 26 == 0 ? 26 : transformed_index;
      transformed_index += 96;
      if(char_ascii < 96) transformed_index -= 32;
    } else
      transformed_index = char_ascii;
    dec[i] = transformed_index;
  }
    display.clearDisplay();
    display.setCursor(0,30);
    display.setTextSize(1);
    display.print("Message: " + dec);
    display.setCursor(0,40);
    display.print("Received from: 0x" + String(sender, HEX));
    
    display.display();
}