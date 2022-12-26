/**************************************************************************
   Ejemplo de Souliss con 3 leds, un led RGB, un pulsador, 
   entrada analógica y sensores de temperatura y humedad
        
***************************************************************************/


#include "SoulissFramework.h"

// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "linksys"
#define WiFi_Password           ""   

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// This identify the number of the LED logic            
#define LEDCONTROL          0              // This is the memory slot for the logic that handle the light
#define LEDRED              1               // This is the memory slot for the logic that handle the light
#define LEDGREEN            2               // This is the memory slot for the logic that handle the light
#define LEDBLUE             3               // This is the memory slot for the logic that handle the light


// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 0, 50};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 0, 1};
#define myvNet_address  ip_address[3]       // The last byte of the IP address (50) is also the vNet address
#define myvNet_subnet   0xFF00


#define DEADBAND                0.05        // Deadband value 5%  


void setup()
{   
    Initialize();

     // Set network parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    SetAsGateway(myvNet_address);  // Set this node as gateway for SoulissApp

   
    SetAsPeerNode(52.50,1);

    Set_LED_Strip(LEDCONTROL);                  // Set a logic to control a LED strip
    
    // Define inputs, outputs pins
    
    pinMode(3, OUTPUT);                 // Power the LED
    pinMode(5, OUTPUT);                 // Power the LED
    pinMode(6, OUTPUT);                 // Power the LED    

    pinMode(2, INPUT);                  // Hardware pulldown required
}

void loop()
{ 
    // Here we start to play
 
    EXECUTEFAST() {                     
        UPDATEFAST();   


           FAST_10ms() {
        
            // Use Pin2 as command
            //DigIn(2, Souliss_T1n_ToggleCmd, LEDCONTROL);        
            //  DigIn(2, Souliss_T1n_OffCmd, LEDCONTROL);
                                            
            // Execute the logic that handle the LED
            Logic_LED_Strip(LEDCONTROL);

            // Use the output values to control the PWM
            analogWrite(3, 255 - mOutput(LEDRED));   // ex 5 es verde
            analogWrite(5, 255 - mOutput(LEDGREEN)); // ex 6 es azul
            analogWrite(6, 255 - mOutput(LEDBLUE)); // ex3 es rojo

            // Just process communication as fast as the logics
            ProcessCommunication();
        }
 
        // Process data communication
        FAST_GatewayComms();
    }

    
  

    
} 
