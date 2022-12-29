/**************************************************************************
    Souliss - Ethernet
    
    Código compatible con el módulo Ethernet ENC28J60. 
      
***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethENC28J60.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node

// Include framework code and libraries
#include <SPI.h>

#include <Wire.h> // Incluimos las librerias necesarias para el Display LCD 16x2
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <SimpleDHT.h>
int pinDHT11 = 7;
SimpleDHT11 dht11;

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {100, 90, 10, 50};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {100, 90, 10, 1};
#define Gateway_address 50
#define Peer_address    51
#define myvNet_address  ip_address[3]       // The last byte of the IP address (77) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address

#define PEERLIGHT           0               // Tambien definimos la luz que está conectada en el otro nodo, para poder comandarlo desde el pulsador

#define RELE1               0               
#define RELE2               1               
#define RELE3               2               
#define LEDCONTROL          3               // This is the memory slot for the logic that handle the light
#define LEDRED              4               // This is the memory slot for the logic that handle the light
#define LEDGREEN            5               // This is the memory slot for the logic that handle the light
#define LEDBLUE             6               // This is the memory slot for the logic that handle the light
#define ANALOGDAQ           7               // This is the memory slot used for the execution of the logic
#define TEMPERATURA         9               // memory slot para temperatura 9 y 10
#define HUMEDAD             11              // memory slot para humedad 11 y 12

void setup()
{   
    Initialize();
    
    // Set network parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp  

    SetAsPeerNode(Peer_address, 1);     // NodeMCU ESP8266
    SetAsPeerNode(Peer_address, 2);
    SetAsPeerNode(Peer_address, 3);
    SetAsPeerNode(Peer_address, 4);
    SetAsPeerNode(Peer_address, 5);
    SetAsPeerNode(Peer_address, 6);
    SetAsPeerNode(Peer_address, 7);
    SetAsPeerNode(Peer_address, 8);
    SetAsPeerNode(Peer_address, 9);

    Set_SimpleLight(RELE1);        // Define a simple LED light logic
    Set_SimpleLight(RELE2);        // Define a simple LED light logic
    Set_SimpleLight(RELE3);        // Define a simple LED light logic

    Set_LED_Strip(LEDCONTROL);                  // Set a logic to control a LED strip

    Set_AnalogIn(ANALOGDAQ);                    // Definimos una entrada analógica para el potenciómetro

    Set_T52(TEMPERATURA);
    Set_T53(HUMEDAD);
    
    //lcd.init(); // INICIAMOS EL DISPLAY LCD Y ENCENDEMOS LA LUZ DE FONDO 
    //lcd.backlight();

    // Definimos las salidas y entradas que utilizaremos
    
    pinMode(4, OUTPUT);                 // RELE
    pinMode(8, OUTPUT);                 // RELE
    pinMode(1, OUTPUT);                 // RELE

    pinMode(3, OUTPUT);                 // LEDRGB
    pinMode(5, OUTPUT);                 // LEDRGB
    pinMode(6, OUTPUT);                 // LEDRGB 

    // We connect a pushbutton between 5V and pin2 with a pulldown resistor 
    // between pin2 and GND, the LED is connected to pin9 with a resistor to
    // limit the current amount
    
    pinMode(2, INPUT);                  // Pulsador pulldown

}

void loop()
{   
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   

        FAST_10ms() {
        
            Logic_LED_Strip(LEDCONTROL);

            // Use the output values to control the PWM
            analogWrite(3, 255 - mOutput(LEDRED));   // ex 5 es verde
            analogWrite(5, 255 - mOutput(LEDGREEN)); // ex 6 es azul
            analogWrite(6, 255 - mOutput(LEDBLUE)); // ex3 es rojo

            // Just process communication as fast as the logics
            ProcessCommunication();
        }
        
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
            if(DigIn(2, Souliss_T1n_ToggleCmd, RELE1))                                                // Use the pin2 as ON/OFF toggle command
                Send(Peer_address, PEERLIGHT, Souliss_T1n_ToggleCmd);
            
            Logic_SimpleLight(RELE1);                          // Drive the LED as per command
            Logic_SimpleLight(RELE2);                          // Drive the LED as per command
            Logic_SimpleLight(RELE3);                          // Drive the LED as per command 
             
            DigOut(4, Souliss_T1n_Coil, RELE1);                // Use the pin9 to give power to the LED according to the logic
            DigOut(8, Souliss_T1n_Coil, RELE2);                // Use the pin8 to give power to the LED according to the logic
            DigOut(1, Souliss_T1n_Coil, RELE3);
        }

        FAST_110ms()
        {
            // Compare the acquired input with the stored one, send the new value to the
            // user interface if the difference is greater than the dead-band
            Read_AnalogIn(ANALOGDAQ);
        }

        FAST_910ms()    
        {
            // Acquire temperature from the microcontroller ADC
            AnalogIn(A0, ANALOGDAQ, 0.097, 0);   // The raw data is 0-1024, scaled as 0-100% without bias (100 / 1024 = 0.09)
     
        }
        
        // Process data communication
        FAST_GatewayComms();
    }
    
    EXECUTESLOW()
    {   
        UPDATESLOW();
        
        SLOW_10s()  {

            byte temp = 0;
            byte hum = 0;
            float temperature;
            float humidity;
            int temperatura_int;
            int humedad_int;
            
            dht11.read(pinDHT11, &temp, &hum, NULL);

            temperature = temp;
            humidity = hum;

            Souliss_ImportAnalog(memory_map, TEMPERATURA, &temperature);
            Souliss_ImportAnalog(memory_map, HUMEDAD, &humidity);
 
            Logic_T52(TEMPERATURA);
            Logic_T53(HUMEDAD);
/*
            // Programamos la pantalla LCD para mostrar la temperatura y humedad
            temperatura_int = temperature;
            humedad_int = humidity;
            lcd.clear();
            lcd.setCursor(0,0); // Configuramos el cursor en la primera fila y columna
            lcd.print("Humedad= ");
            //lcd.print(humedad_int); // Escribimos la humedad
            lcd.print("%");
            lcd.setCursor(0,1); // Ahora configuramos el cursor en la segunda fila y primera columna
            lcd.print("Temp= ");
            //lcd.print(temperatura_int);// Escribimos la temperatura
            lcd.print("C");
        */    
                            
        }       
}} 
