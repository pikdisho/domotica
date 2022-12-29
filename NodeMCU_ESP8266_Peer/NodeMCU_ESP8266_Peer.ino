/**************************************************************************
    Souliss - NodeMCU

    Código compatible con la placa NodeMCU y el core 2.4.0 del ESP8266.
      
***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Definimos el SSID y contraseña de la red Wi-Fi a conectar ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "IITABKP"
#define WiFi_Password           "85206c19"    

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include <SimpleDHT.h> //Incluimos la librería para el sensor DHT11
SimpleDHT11 dht11(D3); //Definimos el pin D3 para la conexión del sensor


/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// Definimos la configuración de red de acuerdo al router y asignaremos una IP estática a la placa.
uint8_t ip_address[4]  = {100, 90, 10, 51};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {100, 90, 10, 1};
#define Gateway_address 50
#define Peer_address    51
#define myvNet_address  ip_address[3]       // The last byte of the IP address (77) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address
 
#define MYLIGHT             0               // This identify the number of the logic on this node
#define PEERLIGHT           0               // This identify the number of the logic on peer node

#define PEERLEDCONTROL          1              // This is the memory slot for the logic that handle the light
#define PEERLEDRED              2               // This is the memory slot for the logic that handle the light
#define PEERLEDGREEN            3               // This is the memory slot for the logic that handle the light
#define PEERLEDBLUE             4               // This is the memory slot for the logic that handle the light

#define PEERTEMPERATURA         5               // Slot para sensor de temperatura en el peer
#define PEERHUMEDAD             7              // Slot para sensor de temperatura en el peer


void setup()
{   
    Initialize();
    
    // Set network parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);

    Set_SimpleLight(MYLIGHT);           // Define a simple LED light logic
    
    // We connect a pushbutton between 5V and pin2 with a pulldown resistor 
    // between pin2 and GND, the LED is connected to pin9 with a resistor to
    // limit the current amount
    pinMode(D0, INPUT);                  // Hardware pulldown required
    pinMode(D2, OUTPUT);                 // Power the LED

    Set_T52(PEERTEMPERATURA);
    Set_T53(PEERHUMEDAD);

    Set_LED_Strip(PEERLEDCONTROL);
    pinMode(D5, OUTPUT);                 // Power the LED
    pinMode(D6, OUTPUT);                 // Power the LED
    pinMode(D7, OUTPUT);                 // Power the LED 
}

void loop()
{   
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   

        FAST_10ms() {                              
            // Execute the logic that handle the LED
            Logic_LED_Strip(PEERLEDCONTROL);

            // Use the output values to control the PWM
            analogWrite(D7, 255 - mOutput(PEERLEDRED));   // ex 5 es verde
            analogWrite(D5, 255 - mOutput(PEERLEDGREEN)); // ex 6 es azul
            analogWrite(D6, 255 - mOutput(PEERLEDBLUE)); // ex3 es rojo

            // Just process communication as fast as the logics
            ProcessCommunication();
        }
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
            if(DigIn(D0, Souliss_T1n_ToggleCmd, MYLIGHT))                                                // Use the pin2 as ON/OFF toggle command
                Send(Gateway_address, PEERLIGHT, Souliss_T1n_ToggleCmd);             // and replicate the command on the peer node
            
            Logic_SimpleLight(MYLIGHT);                         // Drive the relay coil as per command
            DigOut(D2, Souliss_T1n_Coil, MYLIGHT);               // Use the pin9 to give power to the coil according to the logic        
        }
        
        // Process data communication
        FAST_PeerComms();
    }
    
    EXECUTESLOW()
    {   
        UPDATESLOW();

        SLOW_10s()  {

            Logic_T52(PEERTEMPERATURA);
            Logic_T53(PEERHUMEDAD);
     
            byte temperature = 0;
            byte humidity = 0;
            float temperatura_esp;
            float humedad_esp;
            
            dht11.read(&temperature, &humidity, NULL);

            temperatura_esp = temperature;
            humedad_esp = humidity;

            Souliss_ImportAnalog(memory_map, PEERTEMPERATURA, &temperatura_esp);
            Souliss_ImportAnalog(memory_map, PEERHUMEDAD, &humedad_esp);
                     
        }
        
        SLOW_50s()  {
          
        } 
        //if (!IsRuntimeGateway())
            //SLOW_PeerJoin();
        SLOW_PeerJoin();  
    }        
} 
