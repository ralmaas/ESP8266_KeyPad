/*
 MQTT sample
 Read a Keypad and transmit the keys...

 Hold the # key to switch between buffered or single press
 In buffered mode the keypresses are transmitted by hitting the # key
 Default topic is KEYPAD/C0
 Change the last digit by hitting * followed by a digit (the channel)

  2019-05-06/ralm
  2019-05-11/ralm Added two LEDs
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>

// Update these with values suitable for your network.

const char* ssid = "your WiFi";
const char* password = "your password";
const char* mqtt_server = "IP of MQTT-server";

#define TOPIC "KEYPAD/C"
#define DEBUG 0

#define CONNECTED_LED D8
#define TRANSMITTING_LED D0


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int channel = 0;
bool channel_flag = 0;

/// KeyPad section /////
const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'2','1','3'},
  {'0','*','#'},
  {'8','7','9'},
  {'5','4','6'}
};
byte rowPins[ROWS] = {D1, D2, D3, D4}; 	//connect to the row pinouts of the keypad
byte colPins[COLS] = {D5, D6, D7}; 		//connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
/// KeyPad section #/////

#define TRANS_BUFFER_SIZE 50
char  trans_buffer[TRANS_BUFFER_SIZE];
bool buffer_mode = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// This is neved used - no subscrition declared
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    // digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    // digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(TOPIC, "online");
//      // ... and resubscribe
//      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Taking care of some special events.
void keypadEvent(KeypadEvent key){

  if (DEBUG)
  {
    Serial.print("Key pressed: "); Serial.print(keypad.getState()); Serial.print("  => ");Serial.println(key);
  }
    switch (keypad.getState()){
    case PRESSED:

      if (key == '#') {
//            digitalWrite(ledPin,!digitalRead(ledPin));
//            ledPin_state = digitalRead(ledPin);        // Remember LED state, lit or unlit.
      }
      break;

    case RELEASED:
        if (key == '*') {
//            digitalWrite(ledPin,ledPin_state);    // Restore LED state from before it started blinking.
//            blink = false;
        }
        break;

    case HOLD:
        if (key == '#') {
//            blink = true;    // Blink the LED when holding the * key.
            buffer_mode = !buffer_mode;
            if (!buffer_mode)
            {
              strcpy(trans_buffer, "");
            }
        }
        break;
    }
}

void setup() {
  pinMode(CONNECTED_LED, OUTPUT);
  pinMode(TRANSMITTING_LED, OUTPUT);
  digitalWrite(CONNECTED_LED, LOW);
  digitalWrite(TRANSMITTING_LED, LOW);
  Serial.begin(74880);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Set callback for KeyPad");
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  digitalWrite(CONNECTED_LED, HIGH);
}

void loop() {
  char buffer1[20];
  char buffer2[20];

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  char key = keypad.getKey();

  if (key) {
    if (key == '*')
    {
      channel_flag = 1;
    }
    else {
      if (channel_flag)
      {
        channel = int(key)-48;
        channel_flag = 0;
        if (DEBUG) {
          Serial.print("Changing channel to: "); Serial.println(channel);
        }
      }
      else {
        if (key == '#')
        {
          // Check if something to transfer
            sprintf(buffer2, "%s", trans_buffer);
            sprintf(buffer1, "%s%d", TOPIC, channel);
            digitalWrite(TRANSMITTING_LED, HIGH);
            client.publish(buffer1, buffer2);
            strcpy(trans_buffer, "");
            delay(100);
            digitalWrite(TRANSMITTING_LED, LOW);
        }
        else 
        {
          if (buffer_mode)
          {
            // Remember to check for overflow !!!!
            size_t len = strlen(trans_buffer);
            if (len < TRANS_BUFFER_SIZE-1)
            {
              trans_buffer[len] = key;
              trans_buffer[len + 1] = '\0';
            }
            else
            {
              Serial.println("Buffer overflow !!\n New characters ignored");
            }
          }
          else
          {
            sprintf(buffer1, "%s%d", TOPIC, channel);
            sprintf(buffer2, "%c", key);
            digitalWrite(TRANSMITTING_LED, HIGH);
            client.publish(buffer1, buffer2);
            delay(100);
            digitalWrite(TRANSMITTING_LED, LOW);
          }
        }
      }
    }
    if(DEBUG)
    {
      Serial.print("["); Serial.print(key); Serial.println("]");
    }
  }
}