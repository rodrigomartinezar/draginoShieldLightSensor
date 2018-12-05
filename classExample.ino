#include <TembooYunShield.h>
#include <TembooMonitoring.h>
#include "utility/TembooGPIO.h"
#include "TembooAccount.h" // Contains Temboo account information  
 
// Declaring sensor configs
TembooGPIOConfig tmb_lightsensoryun5Config;

// Declaring TembooSensors
TembooSensor tmb_lightsensoryun5;

// Monitoring interval in milliseconds
unsigned long monitorInterval = 30000; 
// Store the time of the last time sensor data was sent
unsigned long lastMonitorRunTime = millis() - monitorInterval; 

// Initializing TembooMessaging sensor array and object
TembooSensor *sensors[1];
TembooMessaging msg(sensors, 1);

const unsigned long TEMBOO_CHOREO_TIMEOUT_MS = 900000;

void setup() {
  Serial.begin(9600);

  Bridge.begin();

  Console.begin();
  
  // Initialize sensors and configs
  tembooAnalogGPIOInit(&tmb_lightsensoryun5Config, &tmb_lightsensoryun5, A0, 0, INPUT);

  // Add sensors to the monitoring table
  msg.addTembooSensor(&tmb_lightsensoryun5);

  // Set account credentials
  msg.setAccountName(TEMBOO_ACCOUNT);
  msg.setAppKeyName(TEMBOO_APP_KEY_NAME);
  msg.setAppKey(TEMBOO_APP_KEY);
  msg.setDeviceID(TEMBOO_DEVICE_NAME);

  Console.println("Setup complete.\n");
}

void loop() { 
  tembooMessagingLoop();
  if (msg.isConnected()) {
    int sensorValue = tmb_lightsensoryun5.read(&tmb_lightsensoryun5Config);
    Console.println("Sensor: " + String(sensorValue));
    if (sensorValue < 455) {
      Console.println("Calling SendMessage Choreo...");
      runSendMessage(sensorValue);
    }
  }
}

void tembooMessagingLoop() {
  // Poll for data from Temboo
  msg.poll();

  // Connect if connection has been lost
  if (!msg.isConnected()) {
    int rc = msg.initiateConnection();
    if (rc != 0 && rc != 253) {
      Console.print("Error starting messaging. Code: ");
      Console.println(rc);
    }
  }
  if (msg.isConnected()) {
    if ((millis() - lastMonitorRunTime) >= monitorInterval) {
      lastMonitorRunTime = millis();
      
      // Read the value of the sensors
      int tmb_lightsensoryun5CurrentValue = tmb_lightsensoryun5.read(&tmb_lightsensoryun5Config);
      // Send sensor values to Temboo 
      msg.updatePinValue(tmb_lightsensoryun5.getSensorPin(&tmb_lightsensoryun5Config), tmb_lightsensoryun5CurrentValue);
    }
  }
  // Print debug information from Temboo's Messenger
  while (msg.available()) {
    char c = msg.read();
    Console.print(c);
  }
}

void runSendMessage(int sensorValue) {
  TembooYunShieldChoreo SendMessageChoreo;

  // Invoke the Temboo client
  SendMessageChoreo.begin();

  // Set Temboo account credentials
  SendMessageChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendMessageChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendMessageChoreo.setAppKey(TEMBOO_APP_KEY);
  SendMessageChoreo.setDeviceName(TEMBOO_DEVICE_NAME);

  unsigned int rc = SendMessageChoreo.run();

  if(rc == 0){
  // Set Choreo inputs
  SendMessageChoreo.addInput("RefreshToken", "1/r56kA0JWI84hI9ghuGgy_4xLk5PHJGu9CUXZyMk2gsQ");
  SendMessageChoreo.addInput("ClientSecret", "IJ6Vd4iw8JJsNcawLL3hTMjM");
  SendMessageChoreo.addInput("ClientID", "921619176199-iv68itkquspgrgouqbgmup3gnt0s7485.apps.googleusercontent.com");
  SendMessageChoreo.addInput("To", "carla.mardones95@gmail.com");
  SendMessageChoreo.addInput("From", "rodrigomartinezaraya97@gmail.com");
  SendMessageChoreo.addInput("Subject", "Alerta");
  SendMessageChoreo.addInput("MessageBody", "Alerta!");
  // Identify the Choreo to run
  SendMessageChoreo.setChoreo("/Library/Google/Gmailv2/Messages/SendMessage");

  // Run the Choreo
  SendMessageChoreo.runAsynchronously();

  unsigned long startChoreoTime = millis();

  while(SendMessageChoreo.running() && (millis() - startChoreoTime < TEMBOO_CHOREO_TIMEOUT_MS)) {
    tembooMessagingLoop();
  }

  unsigned int returnCode = SendMessageChoreo.exitValue();

  // Read and print the error message
  while (SendMessageChoreo.available()) {
    char c = SendMessageChoreo.read();
    Console.print(c);
  }
  Console.println();
  SendMessageChoreo.close();
  }
  else{
    Console.println("Error :c");
  }
}
