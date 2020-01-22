//Inlkuderar bibliotek och defineierar globala variabler samt sätter värden för globala variabler
#include "EspMQTTClient.h"
#define pwm_A 5
#define dir1 0 //bakåt
#define dir2 4
#define pin 12
unsigned long previousMillis;
unsigned long currentMillis;
unsigned long deltaMillis;
float currentPulses = 0;
float Velocity = 0;
float BVelocity = 0 ;
float Kp = 9;
double Ki = 0.01;
float e = 0;
int Speed = 0;
float previousPulses;
float sum_i = 0;
bool Driving = false;
float revolution;
float dist = 0;


void onConnectionEstablished();

//Tredjeparts bibliotek för MQTT
//definierar namn, lösenord, port mm. som mikrokontrollern ska koppla upp sig till samt kopplar upp sig till det.
EspMQTTClient client(
"ABB_Indgym",           // Wifi ssid
  "7Laddaremygglustbil",           // Wifi password
  "maqiatto.com",  // MQTT broker ip
  1883,             // MQTTbroker port
  "filip.cederblad@abbindustrigymnasium.se",            // MQTT username
  "heykidgofuckaduck",       // MQTT password
  "Filip",          // Client name
  onConnectionEstablished, // Connection established callback
  true,             // Enable web updater
  true              // Enable debug messages
);



//subscribar till topicen send och publishar olika meddelanden beroende på vad den inkommande payloaden är.
void onConnectionEstablished()
{
  client.subscribe("filip.cederblad@abbindustrigymnasium.se/send", [] (const String &payload)
  {
    Serial.println(payload);
    if(payload=="starting car"){
        Driving = true;
 ;
        client.publish("filip.cederblad@abbindustrigymnasium.se/send", "car is now on");
      }
    else if(payload=="connected"){
        client.publish("filip.cederblad@abbindustrigymnasium.se/send", "Filip connected");
      }
    else if(payload=="stopping car"){
          Driving = false;

        client.publish("filip.cederblad@abbindustrigymnasium.se/send", "car is now off");
      }
    else if (strtof((payload).c_str(),0) > 0){ 
       BVelocity = strtof((payload).c_str(),0);
       String sendbvelocity;
       sendbvelocity = (String)BVelocity;
       client.publish("filip.cederblad@abbindustrigymnasium.se/send","Filip"+sendbvelocity);
    }
    else {
        String sendvelocity;
        sendvelocity = (String)Velocity;
        client.publish("filip.cederblad@abbindustrigymnasium.se/bil",sendvelocity);//"Current velocity for jenny: "+
      }
    

    }
  );
  }


//interupt funktionen som räknar pulser
ICACHE_RAM_ATTR void pulses() {
  currentPulses +=0.5;
}

//definierar pinmodes och sätter interupten
void setup() {
  Serial.begin(9600);
  pinMode(dir1, OUTPUT);
  pinMode(dir2, OUTPUT);
  pinMode(pwm_A, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pin), pulses, CHANGE);
}


//funktionen som beräknar sträckan och hastighetetn bilen åkt/åker genom att använda tiden som gått, pulserna under den tiden och omkrätsen i cm på däcken.
//den beräknar sedan den nya hastigheten genom att använda ett kp och ki värden som mulltipliceras med felet mellan bör hastigheten och ärhastigheten
//publishar sedan den beräknade hastigheten till hemsidan.
void CalculateSpeed(){
  currentMillis = millis();
  Serial.println(currentMillis);

  float deltaPulses;
  float circumferenceOfWheels = 12.5;
  deltaMillis = currentMillis - previousMillis;
  deltaPulses = currentPulses - previousPulses;
  revolution = deltaPulses/96;
  dist = revolution*circumferenceOfWheels;
  Velocity = (dist/deltaMillis)*1000;
  previousMillis = currentMillis;
  previousPulses = currentPulses;
  Serial.print(Velocity);
  Serial.println("cm/s");
  float deltaSec;
  deltaSec = deltaMillis/1000;
  e = BVelocity - Velocity;
  sum_i += e*deltaMillis;
  Speed  = (sum_i*Ki) +(Kp * e);
  Serial.println(e);
  String sendvelocity;
  sendvelocity = (String)Velocity;
  client.publish("filip.cederblad@abbindustrigymnasium.se/bil", sendvelocity);
  
}

//kör bilen med reglerings kod om bilen blivit startad från hemsidan, om bilen inte blir startad från hemsidan eller om bilden stoppas från hemsidan körs inte motorn.
void loop() {
  if (Driving == true){
    CalculateSpeed();
    digitalWrite(dir1, HIGH);
    analogWrite(pwm_A, Speed);
    delay(10);
    Serial.println(dist);
    
  }
  else{
    analogWrite(pwm_A, 0);
    }
  client.loop();
}

