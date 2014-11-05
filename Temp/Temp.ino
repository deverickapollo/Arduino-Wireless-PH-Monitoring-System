#include <OneWire.h> 
#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Xively.h>

char ssid[] = "Hub1"; //  your network SSID (name) 
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Your Xively key to let you upload data
char xivelyKey[] = "YOUR XIVELY KEY";
//your xively feed ID
#define xivelyFeed //your xively ID

//datastreams
char temperatures[] = "TEMPERATURE_SENSOR_CHANNEL";
char phLevel[] = "PH_SENSOR_LEVEL";
char ordLevel[] = "ORD_SENSOR_LEVEL";

// Define the strings for our datastream IDs
XivelyDatastream datastreams[] = {
  XivelyDatastream(temperatures, strlen(temperatures), DATASTREAM_FLOAT),
  XivelyDatastream(phLevel, strlen(phLevel), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(xivelyFeed, datastreams, 2 /* number of datastreams */);
WiFiClient client;
XivelyClient xivelyclient(client);

int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

void setup(void) {
  Serial.begin(9600);
  Serial.println("Starting single datastream upload to Xively...");
  Serial.println();
  // check for the presence of the shield:
  Serial.print("Firmware version: ");
  Serial.println( WiFi.firmwareVersion() );

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" ){
    Serial.println("Please upgrade the firmware");
    Serial.println("Current Firmware is ");
    Serial.println(fv);
  }

  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();
  connectNetwork();

  printWifiStatus();

}



void loop(void) {
  
  float temperature = getTemp();
  float ph = getPH();
  ///////////////////////////////////////////////////////
  //read Temp sensor values

  datastreams[0].setFloat(temperature);
  datastreams[1].setFloat(ph);

  //print the Temp sensor valye
  Serial.print("Read Temp sensor value ");
  Serial.println(datastreams[0].getFloat());

  //send value to xively
  Serial.println("Uploading it to Xively");
  int ret = xivelyclient.put(feed, xivelyKey);
  //return message
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  Serial.println("");
  
  
  
  /////////////////////////////////////////////////////
  //PH upload
  
  //print the PH sensor valye
  Serial.print("Read PH sensor value ");
  Serial.println(datastreams[1].getFloat());

  //send value to xively
  Serial.println("Uploading it to Xively");
   ret = xivelyclient.put(feed, xivelyKey);
  //return message---200 is success!
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  Serial.println("");
  
    /////////////////////////////////////////////////////
  //PH upload
  
  //print the PH sensor valye
  Serial.print("Read PH sensor value ");
  Serial.println(datastreams[1].getFloat());

  //send value to xively
  Serial.println("Uploading it to Xively");
   ret = xivelyclient.put(feed, xivelyKey);
  //return message---200 is success!
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  Serial.println("");

  //delay between calls
  delay(1500);



}



float getPH(){
  //analogRead(A0);     // pH Level equation
    //delay(2000);
    float PHprobe = analogRead(A0);     // pH Level equation
      Serial.println("phProbe value:");
      Serial.println(PHprobe);
      
      
    float PHLevel = (0.0178 * (PHprobe) - 1.889);  //PHprobe -26  //-14
     Serial.println("Asserting 25 degrees Celsius, the ph is as follows:");
      Serial.println(PHLevel);
      
    float pH = 7 - ((2.5 - PHprobe / 200) / (0.257179 + 0.000941468 * ((getTemp()-32)/1.8000)));
      Serial.println("ph value");
      Serial.println(pH);
     
    
   return pH;


}

float getORD(){
    int mV;
    analogRead(A1);
    //delay(2000);
    float ORPprobe = analogRead(A1);
    mV = ((2.5 - ORPprobe / 200) / 1.037)* 1000;
    return mV;
}

void connectNetwork(){
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(1000);
  } 
  Serial.println("Connected to wifi");
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm \n");
}


void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  byte numSsid = WiFi.scanNetworks();
  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);
  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    Serial.println(WiFi.encryptionType(thisNet));
  }
}



float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float temperatureSum = tempRead / 16.00;
  float sum = (9.00/5.00) *temperatureSum +32;

  return sum;

}

