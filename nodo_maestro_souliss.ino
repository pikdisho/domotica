/**************************************************************************
   Ejemplo de Souliss con 3 relés, un led RGB, un pulsador, 
   entrada analógica y sensores de temperatura y humedad
        
***************************************************************************/


#include "SoulissFramework.h"

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethENC28J60.h"               // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node

#include <Wire.h> // Incluimos las librerias necesarias para el Display LCD 16x2
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Include framework code and libraries
#include <SPI.h>

// para usar sensor temperatura y humedad DHT11 en pin 7
#include <SimpleDHT.h>
int pinDHT11 = 7;
SimpleDHT11 dht11;

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// This identify the number of the LED logic
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

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 50};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
#define myvNet_address  ip_address[3]       // The last byte of the IP address (77) is also the vNet address
#define myvNet_subnet   0xFF00


#define DEADBAND                0.05        // Deadband value 5%  


void setup()
{   
    Initialize();

     // Set network parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    SetAsGateway(myvNet_address);                                  // Set this node as gateway for SoulissApp  
     

    // Set this node as gateway for SoulissApp 
    SetAsPeerNode(51.50,1); // ESP8266
    SetAsPeerNode(52.50,2);
    SetAsPeerNode(53.50,3);
    SetAsPeerNode(54.50,4);
    SetAsPeerNode(55.50,5);
    SetAsPeerNode(56.50,1);
    SetAsPeerNode(57.50,2);
    SetAsPeerNode(58.50,3);
    SetAsPeerNode(59.50,4);
    SetAsPeerNode(60.50,5);






    Set_SimpleLight(RELE1);        // Define a simple LED light logic
    Set_SimpleLight(RELE2);        // Define a simple LED light logic
    Set_SimpleLight(RELE3);        // Define a simple LED light logic

    Set_LED_Strip(LEDCONTROL);                  // Set a logic to control a LED strip

    lcd.init(); // INICIAMOS EL DISPLAY LCD Y ENCENDEMOS LA LUZ DE FONDO 
    lcd.backlight();
    
    // Define inputs, outputs pins
    
    pinMode(3, OUTPUT);                 // Power the LED
    pinMode(5, OUTPUT);                 // Power the LED
    pinMode(6, OUTPUT);                 // Power the LED    
    pinMode(4, OUTPUT);                 // Power the RELE
    pinMode(8, OUTPUT);                 // Power the RELE
    pinMode(1, OUTPUT);                 // Power the RELE    
    
    // We connect a pushbutton between 5V and pin4 with a pulldown resistor 
    // between pin4 and GND, the LED is connected to pin9 with a resistor to
    // limit the current amount
    pinMode(2, INPUT);                  // Hardware pulldown required




    Set_AnalogIn(ANALOGDAQ);                    // Set an analog input value

    Set_T52(TEMPERATURA);
    Set_T53(HUMEDAD);
   
}

void loop()
{ 
    // Here we start to play
 
    EXECUTEFAST() {                     
        UPDATEFAST();   


           FAST_10ms() {
        
            // Use Pin2 as command
            DigIn(2, Souliss_T1n_ToggleCmd, LEDCONTROL);        
                      
            // Execute the logic that handle the LED
            Logic_LED_Strip(LEDCONTROL);

            // Use the output values to control the PWM
            analogWrite(3, 255 - mOutput(LEDRED));   // ex 5 es verde
            analogWrite(5, 255 - mOutput(LEDGREEN)); // ex 6 es azul
            analogWrite(6, 255 - mOutput(LEDBLUE)); // ex3 es rojo

            // Just process communication as fast as the logics
            ProcessCommunication();
        }


        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
            //DigIn(4, Souliss_T1n_ToggleCmd, LED1);            // Use the pin4 as ON/OFF toggle command
        
            Logic_SimpleLight(RELE1);                          // Drive the LED as per command
            Logic_SimpleLight(RELE2);                          // Drive the LED as per command
            Logic_SimpleLight(RELE3);                          // Drive the LED as per command 
             
            DigOut(4, Souliss_T1n_Coil, RELE1);                // Use the pin9 to give power to the LED according to the logic
            DigOut(8, Souliss_T1n_Coil, RELE2);                // Use the pin8 to give power to the LED according to the logic
            DigOut(1, Souliss_T1n_Coil, RELE3);                // Use the pin7 to give power to the LED according to the logic
            
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

 /*
       byte temp = 0;
       byte hum = 0;
       float temperature;
       float humidity;
       
      dht11.read(pinDHT11, &temp, &hum, NULL);
      temperature = temp;
      humidity = hum;
  
      Souliss_ImportAnalog(memory_map, TEMPERATURA, &temperature);
      
      Souliss_ImportAnalog(memory_map, HUMEDAD, &humidity);          
      
      Logic_T52(TEMPERATURA);
      Logic_T53(HUMEDAD);
        
        */
        }       
    
        // Process data communication
        FAST_GatewayComms();
    }

     EXECUTESLOW() {
        UPDATESLOW();

        SLOW_10s() {  // Read temperature and humidity from DHT every 110 seconds  
                //Timer_DimmableLight(MYLED);
                byte temp = 0;
         byte hum = 0;
         float temperature;
         float humidity;
       
         dht11.read(pinDHT11, &temp, &hum, NULL);
      temperature = temp;
      humidity = hum;
      lcd.clear();
      lcd.setCursor(0,0);// set the cursor on the first row and column
      lcd.print("Humedad= ");
      lcd.print((float)hum);//print the humidity
      lcd.print("%");
      lcd.setCursor(0,1);//set the cursor on the second row and first column
      lcd.print("Temp= ");
      lcd.print((float)temp);//print the temperature
      lcd.print("C");   
   
      Souliss_ImportAnalog(memory_map, TEMPERATURA, &temperature);
      
      Souliss_ImportAnalog(memory_map, HUMEDAD, &humidity);          
      
      Logic_T52(TEMPERATURA);
      Logic_T53(HUMEDAD);

        
        }
        
        // If running as Peer
        if (!IsRuntimeGateway())
            SLOW_PeerJoin();
    } 
      
    
  

    
} 
