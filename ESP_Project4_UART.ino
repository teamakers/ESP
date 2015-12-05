// Библиотечки:
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <EEPROM.h>

const String ver = "0.3 UART is hard!"; 

// Fiags:
bool modAP = true; // true - AP  || false - STA  

bool debugP = true; // if(debugP) Serial.print
//var
  //UART
  String inputString = "";         // a string to hold incoming data
  boolean stringComplete = false; 
  


//WiFi
const char *ssid = "ESP_WiFi"; // for AP
//const char* sid = "Inter-zet48";

const char* passw = "987654321";
String str2 = "";//"c9h8e7a6t5";
String str1 = "";//"Inter-zet48";

// multicast DNS responder
MDNSResponder mdns;

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);

void setup() {
  // var
  inputString.reserve(200); 
  
  //begin
  Serial.begin(9600);
  EEPROM.begin(512);
 
  // WiFi mod
  if ( EEPROM.read(500) == 0) modAP = true;
   else                        modAP = false;
   
   if(debugP) if(debugP) Serial.print("EEPROM.read(500) = ");
   if(debugP) if(debugP) Serial.println(modAP);
   
   if ( modAP ) {
    // AP mode:
   if(debugP) if(debugP) Serial.print("START AP");
    WiFi.mode( WIFI_AP);
    WiFi.softAP(ssid, passw);
  }
  else {
    
    String EEPROMs = EEPROMread(0, "</s>", 64);
    // STA Mode:
    str2 = processingString(0, "<p>", "</p>", EEPROMs);
    str1 = processingString(0, "</p>", "</s>", EEPROMs);
    //
    if(debugP) if(debugP) Serial.print("EEPROMread = ");
    if(debugP) if(debugP) Serial.println(EEPROMs);
    
    unsigned char* buf = new unsigned char[32];
    str1.getBytes(buf, 32, 0);
  
    const char *sid = (const char*)buf;
    //
    unsigned char* buf1 = new unsigned char[32];
    str2.getBytes(buf1, 32, 0);
  
    const char *password = (const char*)buf1;
    //
    if(debugP) Serial.print("START STA");
    WiFi.mode( WIFI_STA);
    WiFi.begin(sid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if(debugP) Serial.print(".");
    }
  }
  
  if(debugP) Serial.println("");
  if (modAP) {
     if(debugP) Serial.print("Ssid: ");
     if(debugP) Serial.println(ssid);
     if(debugP) Serial.print("IP address: ");
     if(debugP) Serial.println(WiFi.softAPIP());
  }  
  else {
     if(debugP) Serial.print("connected to ");
  if(debugP) Serial.println(str1);
  if(debugP) Serial.print("IP address: ");
    if(debugP) Serial.println(WiFi.localIP());
  
  
    if (!mdns.begin("esp8266", WiFi.localIP())) {
      if(debugP) Serial.println("Error setting up MDNS responder!");
      while(1) { 
        delay(1000);
      }
    }
  }
  if(debugP) Serial.println("mDNS responder started");
  
  // Start TCP (HTTP) server
  server.begin();
  if(debugP) Serial.println("TCP server started");
  
}

void loop() {
    // Check for any mDNS queries and send responses
  mdns.update();
   
  // Check if a client has connected
  WiFiClient client = server.available();
  
  serialEvent();                                                                //         #UART
  if (stringComplete) {                                                        //          #UART
   // if(debugP) Serial.print(inputString+"LoL");
    inputString = "";
    stringComplete = false;
  }
  
  if (!client) {
    return;
  }
  if(debugP) Serial.println("");
  if(debugP) Serial.println("New client");
  
  //while(client.connected() && !client.available()){
  //  delay(1);
  //}
  String req = client.readStringUntil('\r');
    
  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    if(debugP) Serial.print("Invalid request: ");
    if(debugP) Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  if(debugP) Serial.print("Request: ");
  if(debugP) Serial.println(req);
  client.flush();
  
  String s;
  //обработка URI 
  if (modAP) s = reqAP(req);
  else       s = reqSTA(req);
  //
  client.print(s);
  
  if(debugP) Serial.println("Done with client");
}

String reqAP (String req) {
  String s;
  
  if (req == "/") {
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
    s += "Start to STA mode:";
    s += "\r\n<p>";
    s += "/new?ssid=[your ssid]+pass=[your password]";
    s += "</p>\r\n";
    s += "</html>\r\n\r\n";
    if(debugP) Serial.println("Sending 200");
  } 
  else {
    String id = "";
    String pas = "";
    
    if ( testFor(req, "/EEPROM?clear", 0)) {
        for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0);
        EEPROM.end();
        if(debugP) Serial.print("EEPROM clear");
    }
    if (testFor(req, "/ESP?ver", 0)) {
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>"+ ver+"</html>\r\n\r\n";
      if(debugP) Serial.println("VER..."+ver);
    }
    if ( testFor(req, "/new?ssid=", 0) ) {
      id = processingString(0, "/new?ssid=", "+pass=", req);
      pas = processingString(0, "+pass=", "", req);
      if( (id != "" ) && (pas != "") ) {
      if(debugP) Serial.print("ssid= ");
      if(debugP) Serial.println(id);
      if(debugP) Serial.print("pass = ");
      if(debugP) Serial.println(pas);
      String Es = "<p>" + pas + "</p>" + id + "</s>";
      EEPROM.write(500, 8);
      EEPROM.commit();
      if(debugP) Serial.print("EEPROM 500, 8");
      
      EwS(Es, 0);
      resetE();
      }
      else {
        if(debugP) Serial.print("ERROR -1");
        
      }
    }
    if (testFor(req, "/ESP?reset", 0)) {
      if(debugP) Serial.print("RESET...");
      resetE();
    }
    s = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>404 Not Found</html>\r\n\r\n";
    if(debugP) Serial.println("Sending 404");
  }
  
  return s;
}

String reqSTA (String req) {
  String s;
  if (req == "/") {
   // s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
   // s += "This is STA!";
   s = "123";
    //s += "</html>\r\n\r\n";
    if(debugP) Serial.println("Sending 200");
  } else { 
      s = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>404 Not Found</html>\r\n\r\n";
      if(debugP) Serial.println("Sending 404");
     if ( testFor(req, "/EEPROM?clear", 0)) {
        for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0);
        EEPROM.end();
        if(debugP) Serial.print("EEPROM clear");
    }
    if (testFor(req, "/ESP?ver", 0)) {
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>"+ ver+"</html>\r\n\r\n";
      if(debugP) Serial.println("VER..."+ver);
    }
    if (testFor(req, "/ESP?reset", 0)) {
      if(debugP) Serial.print("RESET...");
      resetE();
    }
    
  }
  return s;
}

//void prinS( String prin, String nam) {
//  if(debugP) Serial.print(nam + " = ");
//  if(debugP) Serial.println(prin);
//}

String processingString ( int startto, String startm, String endm, String ins) {
  ins = ins.substring(startto);
  String outs = ins;
  int stopi = startto;
  if (endm != "") {
    for ( int i = ins.length(); i > 0; i--) {
     if (ins.substring( i - endm.length(), i) == endm) {
      stopi = i;
      i = -1; 
     }
    }
  }
  else {
    stopi = endm.length() + ins.length();
  }
 for (int i = 0; i < (ins.length() - startm.length()) ; i++) {
   if ( ins.substring ( i, i + startm.length()) == startm ) {
     outs = ins.substring( i + startm.length(), stopi - endm.length());
   } 
  }
  return outs;
}

void EwS(String out, int startW) {
  byte outb[2];
  outb[0] = 0;
  String chare = "";
  for (int i = startW; i < out.length(); i++) {
    chare = out.substring(i, i+1);
    chare.getBytes(outb, 2);
    EEPROM.write(i, outb[0]);
  }
  EEPROM.end();
}

String EEPROMread( int startto, String stopm, int maximum) {
  maximum += startto;
  String outs ="";
  int i = startto;
  int f = -1;
  char c = 0;
  while ((i != maximum) && f == -1) {
    c = char(EEPROM.read(i));
    outs += String(c);    
    if ( i > stopm.length() ) {
     if  (outs.substring(i-startto-stopm.length(), i-startto) == stopm) f = i;      
    }
    i++;
  }
  return outs;
}
bool testFor(String in, String fors, int tostart) {
 if(in.substring(tostart, fors.length() + tostart) == fors) return true;
 else                                                       return false;
}
void resetE() {
  int i = 0;
  Serial.print("   RESET   ");
  while (true) {
    i++;
  }
} 

void serialEvent() {                                                            //          #UART
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if ((inChar == '\n')||(inChar == '#')) {
      stringComplete = true;
    }
  }
}
