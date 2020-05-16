#include <SoftwareSerial.h>
#include <TinyGPS++.h>

TinyGPSPlus gps_tiny;
SoftwareSerial gps(5, 6);   //RX , TX for SIM28 Sensor
SoftwareSerial gprs(2, 3); // RX, TX  for SIM800 GPRS Module


int8_t answer;
int onModulePin= 4;

uint8_t NumberSatelleit ;
static float gps_lat,gps_lng,gps_alt; 
static int gps_sat;
static uint32_t Date ;
static uint32_t Time ;

static uint32_t Tt ;

String  TXmsg;
                                            
const String PROGMEM w = "AT+HTTPPARA=""URL"",""http://xxxxxxx.cloudfunctions.net/write?device=TrackingUnit";

     String x ;
     String jour ;
     String y ;
     String heure;
     String Lat;
     String Long;

void setup() {
      delay(3000);
      pinMode(onModulePin, OUTPUT);
        digitalWrite(onModulePin,HIGH);
         delay(3000);
        digitalWrite(onModulePin,LOW);
    gprs.begin(4800);

    Serial.begin(9600);
   Serial.println(F("Starting..."));
   gprs.listen();
    power_on();
      // sets APN , user name and password
    sendATcommand2("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", "ERROR" , 2000);
    sendATcommand2("AT+SAPBR=3,1,\"APN\",\"APN\"APN_Name\"", "OK","ERROR", 2000);
    
    sendATcommand2("AT+SAPBR=0,3", "OK","ERROR", 20000);
    delay(500);
    // gets the GPRS bearer
    Serial.println(F("Opening GPRS bearer"));
      int i =0 ;
    while ((sendATcommand2("AT+SAPBR=1,1", "OK","ERROR", 20000) != 1)&&(i <10))
    {
        delay(1000);
        Serial.print(F("."));
        i++;
        if (i==7){
          sendATcommand2("AT+SAPBR=0,1","OK","ERROR", 5000);
          delay(3000);
        }
    }
    
   gprs.stopListening();  
    delay(3000);
     
    gps.begin(9600);
    
    Serial.println(F("Setup Complete"));

}
//############################################### Loop #################################################################################################################
void loop() {
  gps.listen();

      delay(3000);

    while (!coordonnees(&gps_lat,&gps_lng,&gps_alt,&gps_sat,&Date,&Time)){
      Serial.println(F(""));
      Serial.print(F("Still waiting for  "));
       Serial.print(F(gps_sat));      
      Serial.print(F("  satilites to acquire valid data. "));
      delay(200);
    }
 
      Date=gps_tiny.date.value();
      Time=gps_tiny.time.value();
    

      gps.stopListening(); 
     
      
      x = String (Time);
      jour =x.substring(0,4);
      y = String (Time);
      heure=y.substring(0,4);
      Lat=String(gps_lat,7);
      Long=String(gps_lng,7);

  
    gprs.listen();
       Serial.println(F(" "));
         Serial.print(F("Date: "));
           Serial.print(Date);
            Serial.print(F(" || "));
              Serial.print(F("Heure "));
                Serial.print(Time);
                Serial.println(F(" "));

   send_HTTP (heure, jour, Long , Lat);
   
     heure= " ";
     jour = " ";
     Long = " ";
     Lat  = " ";
      
     Serial.println(F("Next transmission is programmed after 1 minute ")); 
      Serial.println(F("")); 

     
      
    gprs.stopListening();    

  delay(60000);
  Serial.println(F("transmission began")); 

}


//############################## functions #################################################################################################################################

bool coordonnees(float * latitude,float *longitude ,float * altitude, int * gps_sat_number, uint32_t *Date , uint32_t *Temps )
{ 

  while(gps.available())
    {
      gps_tiny.encode(gps.read());
      
      *gps_sat_number=gps_tiny.satellites.value();
     
      if((gps_tiny.satellites.value()>2)&&((gps_tiny.location.lat())!=0))
      {
        delay(2000);
        *latitude= gps_tiny.location.lat();
        *longitude= gps_tiny.location.lng();
        *altitude= gps_tiny.altitude.meters();  
        return 1;
      }
  }
  return 0;
}


//######################################################################################################################""

void power_on(){

    uint8_t answer=0;

    // checks if the module is started
    answer = sendATcommand2("AT", "OK", "OK", 2000);
    if (answer == 0)
    {
        // power on pulse
           digitalWrite(onModulePin,HIGH);
        delay(3000);
           digitalWrite(onModulePin,LOW);

        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand2("AT", "OK", "OK", 2000);
        }
    }
    Serial.println(F("Waiting for the GPRS Module to initialize"));
    for (int i =0 ;i <6 ;i++){
      Serial.print(5-i);
       Serial.print(F(" "));
      delay(1000);
    }

    Serial.println(F(""));

     Serial.println(F("Connecting to BTS"));
  while( sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 1000)== 0 )
    {
      Serial.print(F("."));
    }
    Serial.println(F("Connected to Network (BTS) "));
}

//################################################################################################################################
int8_t sendATcommand2(char* ATcommand, char* expected_answer1,
        char* expected_answer2, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, ' ', 100);    // Initialize the string

    delay(100);

    while( gprs.available() > 0) gprs.read();    // Clean the input buffer

    gprs.println(ATcommand);    // Send the AT command


    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(gprs.available() != 0){
            response[x] = gprs.read();
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else if (strstr(response, expected_answer2) != NULL)
            {
                answer = 2;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));
    
    return answer;
}

//#################################################################################################################"

bool send_HTTP(String Time ,String Date ,String Longitude ,String Latitude){

                              TXmsg=" ";
                              TXmsg=w;
                              TXmsg+= "&Coord" ;
                              TXmsg+= (String)Date + (String)Time +"=" ;
                              TXmsg+= (String)Latitude + "." + (String)Longitude;
                              
   gprs.println(F("AT+HTTPINIT"));
   delay(1000);
   gprs.println(F("AT+HTTPPARA=""CID"",1"));
   delay(1000);
   Serial.println(F("La trame est "));
   Serial.println(TXmsg);
    gprs.println(TXmsg);
   delay(1000);
   gprs.println(F("AT+HTTPACTION=0"));
   delay(1500);
   gprs.println(F("AT+HTTPTERM"));
   delay(1000);
    
}

//##########################################################################################################################"
void SendSMS (){
   gprs.println(F("AT+HTTPINIT"));
   delay(1000);
   gprs.println(F("AT+HTTPPARA=""CID"",1"));
   delay(1000);
   Serial.println(F("La trame est "));
   Serial.println(TXmsg);
    gprs.println(TXmsg);
   delay(1000);
   gprs.println(F("AT+HTTPACTION=0"));
   delay(1500);
   gprs.println(F("AT+HTTPTERM"));
}
