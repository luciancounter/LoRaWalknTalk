#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <SPI.h>
#include <LoRa.h>
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

// Mario's Ideas
// Morse Code transmiter

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Oled display size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16

//Potentiometer PIN A1
  

// Variables capturing current and newly calculated position on the letter board (Values 0-26)
  int New_Position=0;
  int Old_Position=0;

// Variable capturing Potentimeter read (Values 0 1023)
  int Pot_read=0;

// Variable used for calculating moving average of potentiometer reading to stabilise the input
  int Count_For_Moving_Average=0;

// String variable holding the text to transmit
  String To_Transmit="";

// Length of the text to transmit
  int To_Transmit_Length=0;

// Flag indicating the letter was entered on the leter board to be added to To_Transmit string
  int Letter_Entered=0;
  int Transmit=0;
  int But = 0;
  int lft = 0;
  int counter =1;
  String message;
  String outgoing;    
  byte localAddress = 0xaa;     // address of this device
  byte destination = 0xbb;      // destination to send to
  int key =1;


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);


// Used for displaying Leter board
char Letters[28]="ABCDEFGHIJKLMNOPQRSTUVWXYZ_";


void Add_To_String(){

  Letter_Entered=1;
  delay(100);
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



void setup() {
  Serial.begin(115200);
  // Define two interrupts
  // D2- when button pressed the current letter on the letter board is entered to To_Transmit
  // D3- When button is pressed To_Transmit string is output in Morse code 
  attachInterrupt(32,Add_To_String, FALLING);
  attachInterrupt(33,Triger_Transmit, FALLING);
  attachInterrupt(35,ButtonRight, FALLING);
  attachInterrupt(15,ButtonLeft, FALLING);
  SPI.begin(SCK, MISO, MOSI, SS);

  LoRa.setPins(SS, RST, DIO0);

 

  if (!LoRa.begin(BAND)) {

    Serial.println("Starting LoRa failed!");

    while (1);

  }

Wire.begin(OLED_SDA, OLED_SCL);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  
  // Clear the buffer
  display.clearDisplay();
  display.display();
  
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
  }
  // Display filled in rect in the top section of the display when To_Transfer would be output
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
  // Highlight character A by displaying Inverse rect at first position
  display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);

  display.display();
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

void loop() 
{
  if(lft == 1){
    New_Position--;

    if (Old_Position!=New_Position){
       Highlight_letter (New_Position,Old_Position);
       Old_Position=New_Position;
    }
    lft =0;
  }
  // Increase Count_For_Moving_Average 30 times before calcluating moving average
  //Count_For_Moving_Average++;

  if(But == 1){
      New_Position++;
      But = 0;

      if (Old_Position!=New_Position){
       Highlight_letter (New_Position,Old_Position);
       Old_Position=New_Position;
    }
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
    Serial.println(To_Transmit);
    To_Transmit="";
    display.setTextColor(BLACK);
    display.fillRect(0,0,128,15,SSD1306_WHITE);\
    display.println(To_Transmit);
    display.display();
    Transmit=0;
    
  }
  
}

void sendMessage(String outgoing) {
  
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(counter);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  Serial.println(outgoing);
  counter++;                           // increment message ID
}