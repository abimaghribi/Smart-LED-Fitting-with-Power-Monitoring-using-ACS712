
#define IN_PIN A0
#define WINDOW_SIZE 60
#define lamp D3
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;
int PIR = D0;
float SP;
const int Sensor_Pin = A0;
float Sensitivity =185;   // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
float Vpp = 0; // peak-peak voltage 
float Vrms = 0; // rms voltage
float Irms = 0; // rms current
float Supply_Voltage = 220;           // reading from DMM
float Vcc = 5.0;         // ADC reference voltage // voltage at 5V pin 
float power = 0;         // power in watt              
float Wh =0.0 ;             // Energy in kWh
unsigned long last_time =0;
unsigned long current_time =0;
unsigned int calibration = 100;  // V2 slider calibrates this
unsigned int pF = 59;           // Power Factor default 95
int INDEX1 = 0;
float VALUE1 = 0;
float SUM1 = 0;
float READINGS1[WINDOW_SIZE];
float AVERAGED1 = 0;
int INDEX2 = 0;
float VALUE2 = 0;
float SUM2 = 0;
float READINGS2[WINDOW_SIZE];
float AVERAGED2 = 0;
int INDEX3 = 0;
float VALUE3 = 0;
float SUM3 = 0;
float READINGS3[WINDOW_SIZE];
float AVERAGED3 = 0;
float KWhmax;

float outfuz, utotal;
float ON,OZ,OP,ONS,OPS;
float e,de, e_before;
float keluaran;
float uNe,uZe,uPe;
float uNde,uZde,uPde;
float pwm, pwm_max, pwm_min;
bool Ne, Ze,Pe ,Nde,Zde,Pde;



void getACS712() {  // for AC
  Vpp = getVPP();
  Vrms = (Vpp/2.0) *0.707; 
  Vrms = Vrms - (calibration / 10000.0);     // calibtrate to zero with slider
  Irms = (Vrms * 1000)/Sensitivity*0.34;
  if((Irms > -0.015) && (Irms < 0.005)){  // remove low end chatter
    Irms = 0.0;
  }
  power= (Supply_Voltage * Irms) * (pF/ 100.0);
  Wh =Wh+ power *(( current_time -last_time) /3600000000.0); 
  last_time = current_time;
  current_time = millis();    
  //Serial.print("KWh :");
  //Serial.print(Wh,3);
  //Serial.println("KWh");
  //Serial.print("Vrms:");
  //Serial.print(Vrms,2);
  //Serial.println("Volt");
  Serial.print("Irms:  "); 
  Serial.print(String(Irms, 3));
  Serial.println(" A");
  Serial.print("Power: ");
  Serial.print(String(power,3));
  Serial.println("Watt");
  

   
}

float getVPP()
{
  float result; 
  int readValue;                
  int maxValue = 0;             
  int minValue = 1024;          
  uint32_t start_time = millis();
  while((millis()-start_time) <130) //read every 0.95 Sec
  {
     readValue = analogRead(Sensor_Pin);    
     if (readValue > maxValue) 
     {         
         maxValue = readValue; 
     }
     if (readValue < minValue) 
     {          
         minValue = readValue;
     }
  } 
   result = ((maxValue-minValue) * Vcc) / 1023.0;  

   return result;
 }
 



void setup() {
  pinMode(PIR, INPUT);
  pinMode(IN_PIN, INPUT);
  pinMode(lamp, OUTPUT);
  Wire.begin();
  lightMeter.begin();
  Serial.begin(115200);
  KWhmax = 0.141;

  Ne = 0;
  Ze= 0;
  Pe = 0;
  Nde = 0;
  Zde= 0;
  Pde = 0;
  OP = 10;
  OZ = 0;
  ON = -10;
  OPS = 5;
  ONS = -5;
  //pwm = 1;
  pwm_max = 1200;
  pwm_min = 0;
}

void measure() {
  
  getACS712();
  SUM1 = SUM1 - READINGS1[INDEX1];       // Remove the oldest entry from the sum
  VALUE1 = Irms;        // Read the next sensor value
  READINGS1[INDEX1] = VALUE1;           // Add the newest reading to the window
  SUM1 = SUM1 + VALUE1;                 // Add the newest reading to the sum
  INDEX1 = (INDEX1+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED1 = SUM1/WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

  Serial.print("Iavg  :");
  Serial.print(AVERAGED1,3);
  Serial.println("A");

  SUM2 = SUM2 - READINGS2[INDEX2];       // Remove the oldest entry from the sum
  VALUE2 = power;        // Read the next sensor value
  READINGS2[INDEX2] = VALUE2;           // Add the newest reading to the window
  SUM2 = SUM2 + VALUE2;                 // Add the newest reading to the sum
  INDEX2 = (INDEX2+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED2 = SUM2 / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

  Serial.print("Poweravg :");
  Serial.print(AVERAGED2,3);
  Serial.println("Watt");

  SUM3 = SUM3 - READINGS3[INDEX3];       // Remove the oldest entry from the sum
  VALUE3 = Wh;        // Read the next sensor value
  READINGS3[INDEX3] = VALUE3;           // Add the newest reading to the window
  SUM3 = SUM3 + VALUE3;                 // Add the newest reading to the sum
  INDEX3 = (INDEX3+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED3 = SUM3 / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

  Serial.print("KWhavg :");
  Serial.print(AVERAGED3,3);
  Serial.println("KWh");

}

void loop()
{
  if(digitalRead(PIR) == HIGH)
  {
    measure();
    if(AVERAGED3 > KWhmax)
    {
      SP = 53;
      Serial.println("Detected 53");
      utama();
    }
    else
    {
      SP = 56;
      Serial.println("Detected 56");
      utama();
    }
  }
  else
  {
    Serial.println("Not Detected");
    utama();
    measure();
    /*if (pwm < pwm_min)
    {
      pwm = pwm_min;
    }
    else if(pwm > pwm_max)
    {
      pwm = pwm_max;
    }
    else
    {
      pwm = pwm-1;
    }
    analogWrite(led, pwm);
    Serial.print(keluaran); Serial.print(" __ ");
    Serial.println(pwm);
    delay(100);*/
  }
}

void utama() 
{
  // put your main code here, to run repeatedly:
  float intensity = lightMeter.readLightLevel(); //baca intensitas
  float bh_value = intensity;
  Serial.print("intensity= ");Serial.println(bh_value);
  e  = SP - bh_value;
  de = e - e_before;
  fuzifikasi();
  inference();
  defuzifikasi();
  Serial.print("error = ");Serial.println(e);
  Serial.print("delta error = ");Serial.println(de);
  Serial.print("keluaran=");Serial.println(keluaran);
  analogWrite(lamp, pwm);
  Serial.println(pwm);
  clr();
  e_before = e;
  
  Serial.println(" ");
  //delay (130);
}

void clr()
{
  Ne = 0;
  Ze = 0;
  Pe = 0;
  Nde = 0;
  Zde = 0;
  Pde = 0;
  uNe = 0;
  uZe = 0;
  uPe = 0;
  uNde = 0;
  uZde = 0;
  uPde = 0;
  utotal =0;
  outfuz =0; 
}

void fuzifikasi()
{
  //cek bagian error
  // cek bagian Ne
  if (e<0)  
  {
    Ne =1;
    if(e<= -60)uNe = 1;
    else uNe = -e/60;
    //Serial.print("uNe=");Serial.println(uNe);
  }
  
  //cek bagian Ze
  if (e<1 && e>-1)
  {
    Ze= 1;
    if (e>0)uZe = -(e-1)/1;
    else uZe = (e+1)/1;
    //Serial.print("uZe=");Serial.println(uZe);
  }

  //cek bagian Pe
  if (e>0)
  {
    Pe =1;
    if(e>= 60)uPe = 1;
    else uPe = e/60;
    //Serial.print("uPe=");Serial.println(uPe);
  }

  //cek bagian derror
  if (de<0)
  {
    Nde =1;
    if(de<= -60)uNde = 1;
    else uNde = -de/60;
    //Serial.print("uNde=");Serial.println(uNde);
  }

  //cek bagian Ze
  if (de<1 && de>-1)
  {
    Zde= 1;
    if (de>0)uZde = -(de-1)/1;
    else uZde = (de+1)/1 ;
    //Serial.print("uZde=");Serial.println(uZde);
  }

  //cek bagian Pe
  if (de>0)
  {
    Pde =1;
    if(de>= 60)uPde = 1;
    else uPde = de/60;
    //Serial.print("uPde=");Serial.println(uPde);
  } 
}

void inference()
{
  if (Ne ==1 && Nde ==1)
  {
    outfuz = outfuz + (min(uNe, uNde)*ON);
    utotal = utotal + min(uNe,uNde);
  }
  if (Ne ==1 && Zde ==1)
  {
    outfuz = outfuz + (min(uNe, uZde)*ON);
    utotal = utotal + min(uNe,uZde);
  }
  if (Ne ==1 && Pde ==1)
  {
    outfuz = outfuz + (min(uNe, uPde)*ON);
    utotal = utotal + min(uNe,uPde);
  }
  
  if (Ze ==1 && Nde ==1)
  {
    outfuz = outfuz + (min(uZe, uNde)*ONS);
    utotal = utotal + min(uZe,uNde);
  }

  
  if (Ze ==1 && Zde ==1)
  {
    outfuz = outfuz + (min(uZe, uZde)*OZ);
    utotal = utotal + min(uZe,uZde);
  }
  if (Ze ==1 && Pde ==1)
  {
    outfuz = outfuz + (min(uZe, uPde)*OPS);
    utotal = utotal + min(uZe,uPde);
  }
  
  if (Pe ==1 && Nde ==1)
  {
    outfuz = outfuz + (min(uPe, uNde)*OP);
    utotal = utotal + min(uPe,uNde);
  }
  if (Pe ==1 && Zde ==1)
  {
    outfuz = outfuz + (min(uPe, uZde)*OP);
    utotal = utotal + min(uPe,uZde);
  }
  if (Pe ==1 && Pde ==1)
  {
    outfuz = outfuz + (min(uPe, uPde)*OP);
    utotal = utotal + min(uPe,uPde);
  }
}

void defuzifikasi()
{
  keluaran = outfuz/utotal;
  
  if (pwm < pwm_min)
  {
    pwm = pwm_min;
  }
  else if(pwm > pwm_max)
  {
    pwm = pwm_max;
  }
  else
  {
    pwm = pwm + keluaran;
  }
}
