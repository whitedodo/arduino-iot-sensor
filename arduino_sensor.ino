/*
  Project: 아두이노 센서 프로젝트
  Author: Dodo(rabbit.white@daum.net)
  Create Date: 2017-12-20 18:16
  Description: 
  2017-12-20 / Jaspers / DHT 센서
  2017-12-20 / Jaspers / EtherCard 구현
*/

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <EtherCard.h>

const int SHOW_SUCCESS = 1;
const int SHOW_FAILOVER = 2;
const int SHOW_UPLOAD = 3;

// 핀(Pin)
#define DHTPIN            4         // 센서 - 디지털핀
#define LEDX              5         // 게이트 - X 변수
#define LEDY              6         // 게이트 - Y 변수

// DHT - 사용자 센서 타입
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22       // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

// 이더넷
byte Ethernet::buffer[700];

const byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "192.168.0.4";               // 웹 사이트 주소
const static uint8_t dns[] = {192,168,0,1};
const static uint8_t hisip[] = {192,168,0,4};               // 원격 아이피
//const static int hisport = 8080;                            // 원격 포트

uint32_t delayMS;        // 타이머(DHT 센서)
static uint32_t timer;   // 타이머(Ethernet 장치)

static my_callback(byte status, word off, word len){
  ledShow( SHOW_UPLOAD );                                   // 업로드 표시
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

boolean initEthernet(){

  // 이더넷 장치 지원 여부
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
    return false;
  }

  // DHCP 설정
  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
  
  if (!ether.staticSetup (ether.myip, ether.gwip, dns, ether.netmask));
  
  if (!ether.dnsLookup(website))          // DNS로 서버 ip를 설정하려고 시도한다.
  {
     Serial.println("DNS failed");
     ether.copyIp(ether.hisip, hisip);    // DNS가 실패하면 수동으로 구성됩니다.
  } // end of if
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
  ether.printIp("SRV: ", ether.hisip);
  ether.printIp("PORT: ", ether.hisport);

  return true;
  
}

void webBrowser(String parameter, int hisport){
  
  ether.hisport = hisport;                 // 8080포트로 설정
  ether.packetLoop(ether.packetReceive()); // 이더넷 카드의 메인 루프를 활성화하십시오.
  
  if (millis() > timer)                    // 프로세스가 5초마다 실행되는지 확인하십시오.
  {
      timer = millis() + 5000;             // 5초로 제한
     
      Serial.println();
      Serial.println("<<< REQ ");
      Serial.println( parameter.c_str() );
      
      ether.browseUrl(PSTR("/JFoodWeb/Iot/Sensor/collection?"), parameter.c_str(), website, my_callback);      // 웹 보내기
  } // end of if
  
}

void printMac(){
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.println();
}

void initDht(){
  
  Serial.println("DHTxx Unified Sensor");
  sensor_t sensor;

  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  
  delayMS = sensor.min_delay / 1000;
  
}

String getDhtParameter(){

  const int CHOOSE_TEMP = 0;
  const int CHOOSE_HUMIDITY = 1;
  char chResult[2][6];
  String strParameter;
 
  delay(delayMS);  
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    snprintf(chResult[CHOOSE_TEMP], sizeof(chResult[CHOOSE_TEMP]) - 1, "%s", String(event.temperature, 2).c_str());
    Serial.println(" *C");
  }
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    snprintf(chResult[CHOOSE_HUMIDITY], sizeof(chResult[CHOOSE_HUMIDITY]) - 1, "%s", String(event.relative_humidity, 2).c_str());
    //sprintf(chResult[CHOOSE_HUMIDITY], "%s", String(event.relative_humidity, 2).c_str() );
    Serial.print(event.relative_humidity);
    
    Serial.println("%");
  }

  strParameter = "auth=";
  strParameter += "hama";
  strParameter += "&temperature=";
  strParameter += chResult[CHOOSE_TEMP];
  strParameter += "&humidity=";
  strParameter += chResult[CHOOSE_HUMIDITY];
  strParameter += "&area_id=";
  strParameter += "1";
  strParameter += "&device_id=";
  strParameter += "1";
  
  return strParameter;
}

void ledShow( int choose ){

  switch ( choose ){

    case SHOW_SUCCESS:
      // Start Show
      delay(1000);
      digitalWrite(LEDX, LOW);
      delay(1000);
      digitalWrite(LEDY, HIGH);
      delay(1000);
      digitalWrite(LEDY, LOW);
      digitalWrite(LEDX, HIGH);
      delay(1000);
      digitalWrite(LEDY, HIGH);
    
      // Mid Show
      delay(1000);
      digitalWrite(LEDY, LOW);
      delay(1000);
      digitalWrite(LEDX, LOW);
      
      // Highlight Ending Show
      delay(2000);
      digitalWrite(LEDX, HIGH);
      digitalWrite(LEDY, HIGH);
      delay(3000);
      digitalWrite(LEDY, LOW);
      delay(1000);
      digitalWrite(LEDX, LOW);
      delay(3000);
      digitalWrite(LEDX, HIGH);
      break;
    
    case SHOW_FAILOVER:
      digitalWrite(LEDX, LOW);
      
      for(int i = 0; i< 5; i++){
        digitalWrite(LEDY, HIGH);
        delay(2000);
        digitalWrite(LEDY, LOW);
        delay(2000);
      }
      
      break;

    case SHOW_UPLOAD:
      for(int i = 0; i< 3; i++){
        digitalWrite(LEDY, HIGH);
        delay(100);
        digitalWrite(LEDY, LOW);
        delay(100);
      }
      
      break;
  }
    
}

void setup(){
  
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("--------------------------------------------");
  Serial.println(F("[Ethernet DHCP],[DHTxx Unified Sensor]"));
  Serial.println(F("                           정도윤(Jungdy)"));
  Serial.println(F("--------------------------------------------"));

  pinMode(LEDX, OUTPUT); // LED-X 출력
  pinMode(LEDY, OUTPUT); // LED-Y 출력

  printMac();

  // 이더넷 장치가 준비되었을 때
  if ( initEthernet() ){
    ledShow( SHOW_SUCCESS );
  }else{
    ledShow( SHOW_FAILOVER );
  } 
  
  initDht();
}

void loop () {
  
  String parameter;
  parameter = getDhtParameter();          // DHT 파라메터 값 가져오기
  webBrowser(parameter, 8080);            // 웹 브라우저 호출
  
}

