#include <avr/pgmspace.h>
#include "TEE_UC20.h"
#include "SoftwareSerial.h"
#include <AltSoftSerial.h>
#include "internet.h"
#include "File.h"
#include "http.h"
INTERNET net;
UC_FILE file;
HTTP http;
#define START_PIN 4

#include "gnss.h"
GNSS gps;

AltSoftSerial mySerial;
unsigned long bd[10]={1200,2400,4800,9600,19200,38400,57600,115200};
unsigned char bd_index=7;
unsigned char now_bd;
unsigned char flag_now_loop=1;

//SIM TRUE  internet
#define APN "internet"
#define USER ""
#define PASS ""
String my_url="http://www.settakan.com/test/index.html";

typedef enum
{
  WAIT=0,
  TERMINAL,
  ONOFF,
  SEARCH_BD,
}State_process;

unsigned char state_process;
unsigned long previousMillis = 0; 
const long interval = 1000;

unsigned char gps_flag=0;
unsigned char gps_rec=0;
void setup() 
{
  Serial.begin(9600);
  
  gsm.begin(&mySerial,bd[3]);
  state_process = SEARCH_BD;
  help();
}

void loop() 
{
  rx_command();
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) 
  {
    if(gps_flag)
    {
      print_(gps.GetPosition()+"\r\n");
    }
     previousMillis = currentMillis; 
  }
  
}

void rx_command()
{
  //flag_now_loop
   if (Serial.available()) 
   {
     String rx_buf = Serial.readStringUntil('\r');
     process(rx_buf);  
   }
   if(gsm.available())
   {
     String rx_buf_ = gsm.readStringUntil('\r');
     print_(rx_buf_);  
   }
}
void process(String ps)
{
    if(ps.indexOf(F("help"))!= -1)
     {
        help();
     }
     
     if(ps.indexOf(F("stop"))!= -1)
     {
       if(gps_rec)
       {
         gsm.println("AT+QAUDRD=0");
         gps_rec=0;
       }
       flag_now_loop = 0; 
       gps_flag=0;
     }
     if(ps.indexOf(F("onoff"))!= -1)
     {
        print_(F("ON/OFF\r\n")); 
        pinMode(START_PIN,OUTPUT);
        digitalWrite(START_PIN, HIGH);
        delay(1000);
        digitalWrite(START_PIN, LOW);
        delay(1000);
     }
     if(ps.indexOf(F("searchbd"))!= -1)
     {
         search_bd();
     }
     if(ps.indexOf(F("setbd115200"))!= -1)
     {
       Auto_Set_baudrate(bd[now_bd],115200);
     }
     if(ps.indexOf(F("setbd9600"))!= -1)
     {
       Auto_Set_baudrate(bd[now_bd],9600);
     }
     if(ps.indexOf(F("mybd9600"))!= -1)
     {
       print_("Chang bg 9600");
       now_bd = 3;
       gsm.begin(&mySerial,9600);
     }
     if(ps.indexOf(F("mybd115200"))!= -1)
     {
       print_("Chang bg 115200");
       now_bd = 7;
       gsm.begin(&mySerial,115200);
     }
     if(ps.indexOf(F("closeecho"))!= -1)
     {
       print_("Close Echo\r\n");
       gsm.println("ATE0");
       
     }
     if(ps.indexOf(F("setqurcfg"))!= -1)
     {
       print_("Set QURCFG\r\n");
       gsm.println(F("AT+QURCCFG=\"urcport\",\"uart1\""));
     }
     if(ps.indexOf(F("setsound"))!= -1)
     {
       print_("Set sound\r\n");
       gsm.println(F("AT+QDAI=2"));
     }
     if(ps.indexOf(F("testsound"))!= -1)
     {
       print_(F("Test Sound\r\n"));
       gsm.println(F("AT+QTTS=2,\"Thai Easy Elec dot com\""));
     }
     if(ps.indexOf(F("save"))!= -1)
     {
       print_("Save config\r\n");
       gsm.println(F("AT&W"));
     }
     if(ps.indexOf(F("signal"))!= -1)
     {
       print_("Signal\r\n");
       gsm.println(F("AT+CSQ"));
     }
     if(ps.indexOf(F("cops"))!= -1)
     {
       print_("Get cops\r\n");
       gsm.println(F("AT+COPS"));
     }
     if(ps.indexOf(F("testnet"))!= -1)
     {
        print_(F("Disconnect net\r\n"));
        net.DisConnect();
        print_(F("Set APN and Password\r\n"));
        net.Configure(APN,USER,PASS);
        print_(F("Connect net\r\n"));
        net.Connect();
        print_(F("Show My IP\r\n"));
        print_(net.GetIP());
        print_(F("Start HTTP\r\n"));
        http.begin(1);
        http_get();
     }
     if(ps.indexOf(F("gps"))!= -1)
     {
       gps.Start();
       gps_flag=1;
       print_(F("GPS Start"));
     }
     if(ps.indexOf(F("record"))!= -1)
     {
       print_(F("Record Sound\r\n"));
       print_(F("Delete All File in RAM\r\n"));
       gsm.println("AT+QFDEL=\"RAM:*\"");
       delay(1000);
       print_(F("Start record"));
       gsm.println("AT+QAUDRD=1,\"RAM:A.amr\",3");
       gps_rec=1; 
     }
     if(ps.indexOf(F("play"))!= -1)
     {
       print_(F("Play Sound\r\n"));
       gsm.println("AT+QAUDPLAY=\"RAM:A.amr\",0,7");
     }
     if(ps.indexOf(F("list"))!= -1)
     {
       print_(F("List File in RAM\r\n"));
       gsm.println("AT+QFLST=\"RAM:*\"");
     }
     if(ps.indexOf(F("clearram"))!= -1)
     {
       print_(F("Delete All File in RAM\r\n"));
       gsm.println("AT+QFDEL=\"RAM:*\"");
     }
     
     
     
     if(ps.indexOf(F("+++"))!= -1)
     {
       print_("AT Mode\r\n");
       flag_now_loop=1;
       while(flag_now_loop)
       {
          if (gsm.available())
          {
              Serial.write(gsm.read());
          } 
          if (Serial.available())
          {
            String rx_buf = Serial.readStringUntil('\r');
            if(rx_buf.indexOf(F("+++"))!= -1)
              flag_now_loop=0;
            gsm.print(rx_buf+"\r\n");
          } 
       }
       print_("Exit\r\n");
     }
}

void http_get()
{
  print_(F("Send HTTP GET\r\n"));
  http.url(my_url);
  print_("get....\r\n");
  int ret = http.get();
  if(ret == 200)
  {
    print_(F("Clear data in RAM\r\n"));
    file.Delete(RAM,"*");
    print_(F("Save HTTP Response To RAM\r\n"));
    http.SaveResponseToMemory(RAM,"web.hml");
    print_(F("Read data in RAM\r\n"));
    read_file(RAM,"web.hml");
  }
  else
  {
    if(ret==-2)
    {
      print_("------->Time out<-------\r\n");
    }
    print_(ret+"\r\n");
  }
  print_("\r\n<---------END------------>\r\n");
}

void read_file(String pattern,String file_name)
{
  file.DataOutput =  data_out;
  file.ReadFile(pattern,file_name);
}
void data_out(char data)
{
  if(data==0x0a)
  {
    Serial.write(0x0d);
  }
  Serial.write(data);
}

void terminal()
{
   if (gsm.available())
  {
    Serial.write(gsm.read());
  } 
  if (Serial.available())
  {
    char c = Serial.read();
    gsm.write(c);
    
  } 
}
void search_bd()
{
  unsigned long previousMillis = 0;  
  const long interval = 1000;
  unsigned long currentMillis;
  flag_now_loop=1;
  while(flag_now_loop)
  {
    currentMillis = millis();
    if(currentMillis - previousMillis >= interval) 
    {
      print_(F("Serial baud rate scan"));
      print_(String(bd[bd_index])+"\r\n");
      gsm.begin(&mySerial,bd[bd_index]);
      gsm.println(F("AT"));
      previousMillis = currentMillis;
      bd_index--;
      if(bd_index<=0)
      {
        print_(F("Can not Find Serial baud rate\r\n"));
        flag_now_loop=0;
        bd_index=7;
      }
    }
     if (gsm.available()) 
    {
       String rx_buf = gsm.readStringUntil('\r');
       if(rx_buf.indexOf(F("OK"))!= -1)
       {
          Serial.print("My Serial baud rate = ");
          Serial.println(bd[bd_index+1]);
          now_bd = bd_index+1;
          flag_now_loop=0;
          bd_index=7;
       }
       else
       {
          print_(rx_buf+"\r\n");
       }
    }
  }
}

void print_(String dat)
{
  Serial.print(dat);
}

void Auto_Set_baudrate(long oldbd,long newbd)
{
  print_("Now BD = ");
  print_(String(bd[now_bd])+"\r\n");
  flag_now_loop=1;
  unsigned char out_loop=0;
  const long interval = 5000; 
  unsigned long previousMillis = millis(); 
  unsigned long currentMillis;
  
   gsm.println(F("ATE0"));
   delay(1000);
   gsm.print(F("AT+IPR="));
   delay(1000);
   gsm.println(String(newbd));
   delay(1000);
   gsm.begin(&mySerial,newbd);
   delay(1000);
   gsm.println(F("AT+IFC=0,0"));
   delay(1000);
   print_("end\r\n");
}
bool wait_ok()
{
  unsigned char x=0;
  while(1)
{
     if(gsm.available()>0)
     {
       String req = gsm.readStringUntil('\n');
       print_(req+"\r\n");
       if(req.indexOf(F("OK")) != -1)
         return(true);
       else
       { 
           x++;
           if(x>3)
           return(false);
       }
     }
    }
}
void help()
{
  Serial.println(F("\r\n###########################################################################"));
  Serial.println(F("Config UCxx Command"));
  Serial.println(F("help --> Use for List Command"));
  Serial.println(F("onoff --> Use for toggle On / Off UCxx Module"));
  Serial.println(F("searchbd --> Use for search current Baudrate"));
  Serial.println(F("setbd115200 --> Use for Set Baudrate UCxx and Arduino to baud rate 115200"));
  Serial.println(F("setbd9600 --> Use for Set Baudrate UCxx and Arduino to baud rate 9600"));
  Serial.println(F("mybd115200 --> Use for Set Baudrate Arduino to baud rate 115200"));
  Serial.println(F("mybd9600 --> Use for Set Baudrate Arduino to baud rate 9600"));
  Serial.println(F("closeecho --> Use for Close Echo"));
  Serial.println(F("setqurcfg --> Use for Set qurcfg to UART1"));
  Serial.println(F("setsound --> Use for Set DAC card (UC20 Only)"));
  Serial.println(F("testsound --> Use for Test play sound"));
  Serial.println(F("save --> Use for Save All Setting"));
  Serial.println(F("signal --> Use for display signal quality"));
  Serial.println(F("cops --> Use for display current network operator"));
  Serial.println(F("testnet --> Use for test internet (httpget)"));
  Serial.println(F("\r\n###########################################################################"));
  Serial.println();
  Serial.println();
  
  
  
  
  
}
