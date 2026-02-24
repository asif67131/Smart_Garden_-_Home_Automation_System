#define BLYNK_TEMPLATE_ID "abc"
#define BLYNK_TEMPLATE_NAME "LED Control System"
#define BLYNK_AUTH_TOKEN "abc"        //replace abc with ur blynk credentials
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h> 

char ssid[] = "XYZ";
char pass[] = "XYZ";                  //replace XYZ with ur wifi credentials

#define PCF_ADDRESS 0x20 

// --- GPIO PIN DEFINITIONS ---
#define LED1_GPIO   D0   
#define LED2_GPIO   D7   
#define LED3_GPIO   D8   
#define PUMP_GPIO   D6   
#define SOIL_POWER  D5   
#define SOIL_PIN    A0   
#define BUZZER_PIN  D3   // NEW: Direct GPIO for Buzzer

WidgetTerminal terminal(V10); 
BlynkTimer timer;

// State Variables
bool s_LED1 = false;   
bool s_LED2 = false;   
bool s_LED3 = false;   
bool s_PUMP = false;   
bool manualOverride = false; 

bool p_LED1, p_LED2, p_LED3; // Previous states for terminal logging

unsigned long actionStartTime = 0;
int systemState = 0; 

// Adjustable Settings
int pumpDuration = 1000;             
const int DRY_THRESHOLD = 30;        
const int MOISTURE_INCREASE_REQ = 2; 
const int MAX_FAILURES = 3;          

int currentSoil = 0;
int previousSoil = 0;
int failureCount = 0;
bool systemLocked = false; 

BLYNK_CONNECTED() {
  terminal.clear();
  terminal.println(F("==============================="));
  terminal.println(F("   SYSTEM ACTIVE & ONLINE      "));
  terminal.println(F("==============================="));
  terminal.flush();
  Blynk.virtualWrite(V1, s_LED1);
  Blynk.virtualWrite(V2, s_LED2);
  Blynk.virtualWrite(V3, s_LED3);
  Blynk.virtualWrite(V4, s_PUMP);
}

void printHeartbeat() {
  terminal.print(F("HEARTBEAT: System OK | Uptime: "));
  terminal.print(millis() / 60000); 
  terminal.println(F(" min"));
  terminal.flush();
}

void updateHardware() {
  // Terminal Logging for LEDs
  if (s_LED1 != p_LED1) { terminal.print(F("LED 1: ")); terminal.println(s_LED1 ? F("ON") : F("OFF")); p_LED1 = s_LED1; }
  if (s_LED2 != p_LED2) { terminal.print(F("LED 2: ")); terminal.println(s_LED2 ? F("ON") : F("OFF")); p_LED2 = s_LED2; }
  if (s_LED3 != p_LED3) { terminal.print(F("LED 3: ")); terminal.println(s_LED3 ? F("ON") : F("OFF")); p_LED3 = s_LED3; }
  terminal.flush();

  // PCF8574 Buttons (Setting pins HIGH to read them)
  byte data = 0;
  data |= (1 << 0) | (1 << 4) | (1 << 5) | (1 << 6); 
  Wire.beginTransmission(PCF_ADDRESS);
  Wire.write(data);
  Wire.endTransmission();

  // Relay Control (Active LOW)
  digitalWrite(LED1_GPIO, s_LED1 ? LOW : HIGH);
  digitalWrite(LED2_GPIO, s_LED2 ? LOW : HIGH);
  digitalWrite(LED3_GPIO, s_LED3 ? LOW : HIGH);
  digitalWrite(PUMP_GPIO, s_PUMP ? LOW : HIGH);
}

bool readButton(int pin) {
  Wire.requestFrom(PCF_ADDRESS, 1);
  if (Wire.available()) {
    byte state = Wire.read();
    if ((state & (1 << pin)) == 0) return true; 
  }
  return false; 
}

void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void updateTerminalStatus() {
  terminal.println(F("--- LIVE STATUS UPDATE ---"));
  terminal.print(F("PUMP: ")); terminal.println(s_PUMP ? F("ON") : F("IDLE"));
  terminal.print(F("SOIL: ")); terminal.print(currentSoil); terminal.println(F("%"));
  terminal.flush();
}

int readSoilMoisture() {
  digitalWrite(SOIL_POWER, HIGH); 
  delay(50); 
  int raw = analogRead(SOIL_PIN);
  digitalWrite(SOIL_POWER, LOW);  
  return constrain(map(raw, 1024, 400, 0, 100), 0, 100);
}

void runIrrigationLogic() {
  if (systemLocked || manualOverride) return; 
  unsigned long now = millis();

  if (systemState == 1) { // WATERING
    if (now - actionStartTime >= (unsigned long)pumpDuration) { 
      s_PUMP = false;
      updateHardware();
      systemState = 2;
      actionStartTime = now;
      Blynk.virtualWrite(V4, 0); 
    }
  }

  if (systemState == 2) { // SOAKING
    if (now - actionStartTime >= 10000) { 
      currentSoil = readSoilMoisture();
      int increase = currentSoil - previousSoil;
      if (increase < MOISTURE_INCREASE_REQ) {
        failureCount++;
        beep(200);
      } else {
        failureCount = 0; 
      }
      if (failureCount >= MAX_FAILURES) {
        systemLocked = true;
        terminal.println(F("!!! SYSTEM LOCKED !!!"));
        terminal.println(F("Type 'RESET' to continue."));
      }
      systemState = 0;
      updateTerminalStatus();
    }
  }
}

void startCheck() {
  if (systemState != 0 || systemLocked || manualOverride) return;
  currentSoil = readSoilMoisture();
  Blynk.virtualWrite(V0, currentSoil); 
  
  if (currentSoil < DRY_THRESHOLD) {
    previousSoil = currentSoil; 
    s_PUMP = true; 
    updateHardware(); 
    systemState = 1;
    actionStartTime = millis();
    updateTerminalStatus();
  }
}

void checkPhysicalButtons() {
  if (readButton(0)) {
    if (!systemLocked) {
      s_PUMP = !s_PUMP;
      manualOverride = s_PUMP;
      updateHardware();
      Blynk.virtualWrite(V4, s_PUMP); 
      beep(50);
      updateTerminalStatus();
    }
    while(readButton(0)) delay(10);
  }

  if (readButton(4)) { s_LED1 = !s_LED1; updateHardware(); Blynk.virtualWrite(V1, s_LED1); while(readButton(4)) delay(10); }
  if (readButton(5)) { s_LED2 = !s_LED2; updateHardware(); Blynk.virtualWrite(V2, s_LED2); while(readButton(5)) delay(10); }
  if (readButton(6)) { s_LED3 = !s_LED3; updateHardware(); Blynk.virtualWrite(V3, s_LED3); while(readButton(6)) delay(10); }
}

BLYNK_WRITE(V1) { s_LED1 = param.asInt(); updateHardware(); }
BLYNK_WRITE(V2) { s_LED2 = param.asInt(); updateHardware(); }
BLYNK_WRITE(V3) { s_LED3 = param.asInt(); updateHardware(); }

BLYNK_WRITE(V4) { 
  if(!systemLocked) {
    s_PUMP = param.asInt(); 
    manualOverride = s_PUMP;
    updateHardware(); 
    updateTerminalStatus();
  }
} 

BLYNK_WRITE(V5) {
  int rawValue = param.asInt();
  pumpDuration = constrain((rawValue / 500) * 500, 500, 10000);
}

BLYNK_WRITE(V10) {
  String cmd = param.asStr();
  cmd.toUpperCase(); cmd.trim();        
  if (cmd == "RESET") {
    systemLocked = false;
    failureCount = 0;
    systemState = 0;
    terminal.println(F("> Command Received: RESET"));
    terminal.println(F("> System Unlocked."));
    terminal.flush();
    beep(100);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); 
  
  pinMode(LED1_GPIO, OUTPUT); digitalWrite(LED1_GPIO, HIGH);
  pinMode(LED2_GPIO, OUTPUT); digitalWrite(LED2_GPIO, HIGH);
  pinMode(LED3_GPIO, OUTPUT); digitalWrite(LED3_GPIO, HIGH);
  pinMode(PUMP_GPIO, OUTPUT); digitalWrite(PUMP_GPIO, HIGH);
  pinMode(SOIL_POWER, OUTPUT); digitalWrite(SOIL_POWER, LOW);
  pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, LOW); // Setup D3

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  p_LED1 = s_LED1; p_LED2 = s_LED2; p_LED3 = s_LED3;
  updateHardware(); 
  
  timer.setInterval(10000L, startCheck);    
  timer.setInterval(30000L, printHeartbeat); 
}

void loop() {
  Blynk.run();
  timer.run();
  runIrrigationLogic(); 
  checkPhysicalButtons(); 
}
