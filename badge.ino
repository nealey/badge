/* e-paper display lib */
#include <GxEPD.h>
//Use the GxGDEW029T5 class if you have Badgy Rev 2C. Make sure you are on GxEPD 3.05 or above
#include <GxGDEW029T5/GxGDEW029T5.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

/* WiFi  libs*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>

#define len(a) (sizeof(a) / sizeof(*a))

char *badges[][4] = {
  {
    "Neale", "Pickett",
    "Los Alamos National Laboratory",
    "Cyber Fire Lead",
  },
  {
    "Neale", "Pickett",
    "Cybersecurity Educator",
    "Los Alamos National Laboratory",
  },
  {
    "Neale", "Pickett",
    "Los Alamos National Laboratory",
    "",
  },
  {
    "Neale", "Pickett",
    "",
    "",
  },
  NULL
};

int badgeno = 0;


struct Room {
  char *title;
  char *description;
  int north, west, east, south;
};

struct Room rooms[] = {
  {
    // 0
    "--== Neale Pickett ==--",
    (
      "YOU ARE LIKELY TO BE\n"
      "EATEN BY A GRUE\n"
      "\n"
      "IF THIS PREDICAMENT SEEMS PARTICULARLY\n"
      "CRUEL..\n"
      "\n"
      "CONSIDER WHOSE FAULT IT COULD BE..\n"
      "NOT A TORCH OR A MATCH\n"
      "IN YOUR INVENTORY....\n"
      "\n"
      "]"
    ),
    1, 1, 1, 1,
  },
  {
    // 1
    "West of House",
    (
      "You are standing in an open field west of a\n"
      "white house, with a boarded front door."
    ),
    2, -1, -1, 3,
  },
  {
    // 2
    "North of House",
    (
      "You are facing the north side of a white house.\n"
      "There is no door here, and all the windows are\n"
      "boarded up. To the north a narrow path winds\n"
      "through the trees."
    ),
    7, 1, 4, -1,
  },
  {
    // 3
    "South of House",
    (
      "You are facing the south side of a white house.\n"
      "There is no door here, and all the windows are\n"
      "boarded."
    ),
    -1, 1, 4, 9,
  },
  {
    // 4
    "Behind House",
    (
      "You are behind the white house. A path leads\n"
      "into the forest to the east. In one corner of\n"
      "the house there is a small window which is open."
    ),
    2, 5, 13, 3,
  },
  {
    // 5
    "Kitchen",
    (
      "You are in the kitchen of the white house. A\n"
      "table seems to have been used recently for the\n"
      "preparation of food. A passage leads to the\n"
      "west and a dark staircase can be seen leading\n"
      "upward. A dark chimney leads down and to the\n"
      "east is a small window which is open."
    ),
    15, 6, 4, -1,
  },
  {
    // 6
    "Living Room",
    (
      "You are in the living room. There is a doorway\n"
      "to the east, a wooden door with strange gothic\n"
      "lettering to the west, which appears to be nailed\n"
      "shut, a trophy case, and a large oriental rug in\n"
      "the center of the room.\n"
      "\n"
      "Above the trophy case hangs an elvish sword of\n"
      "great antiquity.\n"
      "\n"
      "A battery-powered brass lantern is on the trophy\n"
      "case."
    ),
    -1, -1, 5, -1,
  },
  {
    // 7
    "Forest Path",
    (
      "This is a path winding through a dimly lit\n"
      "forest. The path heads north-south here. One\n"
      "particularly large tree with some low branches\n"
      "stands at the edge of the path."
    ),
    8, 9, 10, 2,
  },
  {
    // 8
    "Clearing",
    (
      "You are in a clearing, with a forest\n"
      "surrounding you on all sides. A path leads\n"
      "south."
    ),
    -1, 9, 10, 7,
  },
  {
    // 9
    "Forest", // West
    (
      "This is a forest, with trees in all directions.\n"
      "To the east, there appears to be sunlight."
    ),
    8, -1, 7, 11,
  },
  {
    // 10
    "Forest", // East
    (
      "This is a dimly lit forest, with large trees\n"
      "all around."
    ),
    8, 7, 12, 13,
  },
  {
    // 11
    "Forest", // South
    (
      "This is a dimly lit forest, with large trees\n"
      "all around."
    ),
    13, 9, 14, -1,
  },
  {
    // 12
    "Forest", // Far East
    (
      "The forest thins out, revealing impassable\n"
      "mountains."
    ),
    10, 10, -1, 10,
  },
  {
    // 13
    "Clearing", // West
    (
      "You are in a small clearing in a well marked\n"
      "forest path that extends to the east and west."
    ),
    10, 4, 14, 11,
  },
  {
    // 14
    "Canyon View",
    (
      "You are at the top of the Great Canyon on its\n"
      "west wall. From here there is a marvelous view\n"
      "of the canyon and parts of the Frigid River\n"
      "upstream. Across the canyon, the walls of the\n"
      "White Cliffs join the mighty ramparts of the\n"
      "Flathead Mountains to the east. Following the\n"
      "Canyon upstream to the north, Aragain Falls may\n"
      "be seen, complete with rainbow. The mighty Frigid\n"
      "River flows out from a great dark cavern. To the\n"
      "west and south can be seen an immense forest,\n"
      "stretching for miles around. A path leads north."
    ),
    13, 11, -1, -1,
  },
  {
    // 15
    "Dark Place",
    (
      "It is pitch black. You are likely to be eaten\n"
      "by a grue."
    ),
    -1, -1, -1, 5,
  },
};

const int start = 0;
const int darkPlace = 15;
int where = start;


/* Always include the update server, or else you won't be able to do OTA updates! */
/**/const int port = 8888;
/**/ESP8266WebServer httpServer(port);
/**/ESP8266HTTPUpdateServer httpUpdater;
/*                                                                                */

/* Configure pins for display */
GxIO_Class io(SPI, SS, 0, 2);
GxEPD_Class display(io); // default selection of D4, D2

/* A single byte is used to store the button states for debouncing */
byte buttonState = 0;
byte lastButtonState = 0;   //the previous reading from the input pin
unsigned long lastDebounceTime = 0;  //the last time the output pin was toggled
unsigned long debounceDelay = 50;    //the debounce time

void setup()
{
  display.init();
  
  pinMode(1,INPUT_PULLUP); //down
  pinMode(3,INPUT_PULLUP); //left
  pinMode(5,INPUT_PULLUP); //center
  pinMode(12,INPUT_PULLUP); //right
  pinMode(10,INPUT_PULLUP); //up
  
  /* Enter OTA mode if the center button is pressed */
  if(digitalRead(5)  == 0){
    /* WiFi Manager automatically connects using the saved credentials, if that fails it will go into AP mode */
    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect("Badgy AP");
    /* Once connected to WiFi, startup the OTA update server if the center button is held on boot */
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    showIP();
    while(1){
      httpServer.handleClient();
    }
  }
  showBadge(); //show "Hello my name is" immediately on boot
}

void loop()
{
  byte reading =  (digitalRead(1)  == 0 ? 0 : (1<<0)) | //down
                  (digitalRead(3)  == 0 ? 0 : (1<<1)) | //left
                  (digitalRead(5)  == 0 ? 0 : (1<<2)) | //center
                  (digitalRead(12) == 0 ? 0 : (1<<3)) | //right
                  (digitalRead(10) == 0 ? 0 : (1<<4));  //up
                  
  if(reading != lastButtonState){
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      int next = -1;
      
      for (int i = 0; i < 5; i++) {
        if (bitRead(buttonState, i) == 0) {
          switch(i){
            case 0: // down
              next = rooms[where].south;
              break;
            case 1: // left
              next = rooms[where].west;
              break;
            case 2: // center
              if (where == start) {
                // Cycle through badges
                badgeno += 1;
                if (badges[badgeno][0] == NULL) {
                  badgeno = 0;
                }
                showBadge();
              } else if (where == darkPlace) {
                // Easter egg!
                next = start;
              } else {
                // Show whatever badge was last shown
                where = start;
                next = -1;
                showBadge();
              }
              break;
            case 3: // right
              next = rooms[where].east;
              break;
            case 4: // up
              next = rooms[where].north;
              break;
            default:
              break;
          }
        }
      }
      
      if (next > -1) {
        where = next;
        showRoom();
      }
    }
  }
  lastButtonState = reading;
}

void configModeCallback (WiFiManager *myWiFiManager){
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(0, 50);
  display.println("Connect to Badgy AP");
  display.println("to setup your WiFi!");
  display.update();
}

void showRoom() {
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  
  display.fillRect(0, 0, display.width(), 20, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(20, 15);
  display.println(rooms[where].title);
  
  display.setTextColor(GxEPD_BLACK);
  display.setFont();
  display.setCursor(0, 30);
  display.println(rooms[where].description);
  display.update();
}

void showIP() {
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(0, 10);

  String url = WiFi.localIP().toString() + ":"+String(port)+"/update";
  byte charArraySize = url.length() + 1;
  char urlCharArray[charArraySize];
  url.toCharArray(urlCharArray, charArraySize);

  display.println("You are now connected!");
  display.println("");
  display.println("Go to:");
  display.println(urlCharArray);
  display.println("to upload a new sketch.");
  display.update();
}

void showBadge()
{
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE);

  display.setTextColor(GxEPD_BLACK);
  
  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(30, 35);
  display.println(badges[badgeno][0]);

  display.setFont(&FreeSansBold12pt7b);
  display.setCursor(30, 65);
  display.println(badges[badgeno][1]);

  display.setFont(&FreeSans9pt7b);
  display.setCursor(0, 100);
  display.println(badges[badgeno][2]);
  display.println(badges[badgeno][3]);

  display.update();
}
