#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>

#define PIC_TIMEOUT 500000

const char* ssid = "BUZOVA1";
const char* password = "1234567890";
int temp[4] = { 0,0,0,0 }; 

ESP8266WebServer server(80);
const int led = 13;

IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 0, 1);

char * temp_correct(char * str){
    static char temp[4][6];
    static char result[32];
    
    
    if (str[0]=='-' && str[1]=='-' && str[2]=='-') {
      
      return str;
    
    } else {
      int i=0; // индекс входящей/исходящей строки
      int ii=0; // индекс по записи в массиве
      int j=0;    // индекс для массива температур
    
      // разбили на четыре отдельных слова 
      for (j=0; j<4; j++) {
          temp[j][0]='\0';
          while (str[i]!='|' && str[i]!='\0') {
              if (str[i]=='-'
                      || str[i]=='+'
                      || str[i]=='0'
                      || str[i]=='1'
                      || str[i]=='2'
                      || str[i]=='3'
                      || str[i]=='4'
                      || str[i]=='5'
                      || str[i]=='6'
                      || str[i]=='7'
                      || str[i]=='8'
                      || str[i]=='9') {
                  temp[j][ii] = str[i];
                  ii++;
              };
              i++;
          };
          temp[j][ii]='\0';     
          i++;
          ii=0;
      };
      
      // формируем строку результата
      result[0]='\0';
      i = 0;  // индекс в исходящей строке
      ii = 0; // индекс по записи в массиве
      j = 0;
      
      for (j=0; j<4; j++) {
            if (temp[j][0]!='-') {
                result[i] = '+';
                i++;
            };

            while(temp[j][ii]!='\0') {
              result[i] = temp[j][ii];
              i++;
              ii++;
            };
 
            // ставим точку
            result[i] = result[i-1];
            if ((ii==2 && temp[j][0]=='-') || ii==1) {
              result[i-1] = '0';
              result[i+1] = result[i];
              result[i] = '.';
              i++;
            } else {
              result[i-1] = '.';
            };
          
          i++;
          if (j<3 ) result[i] = '|';
          i++;
          ii=0;
          temp[j][0] = '\0';
      };
      
      
      result[i-1]='\0';
      return result;
    };
};


char * UART_read_answer() {  // чтение ответа по UART от термометра, возвращает указатель на array с ответом
  
  static char buffer[32];    // буффер для приема строки ответа  
  char a;             // буффер для приема символа ответа
  int i = 0;              // счётчик для номера символа ответа
  double t = 0;       // счётчик для таймера таймаута
  bool flag = true;   // флаг разрешения приема
  
      while(flag) {
         if (Serial.available() > 0) {
           a = Serial.read();
           t = 0;
           switch(a){
              case -1:
                break;
              case 13: // прекращение приема строки по символу \r от UART
                flag = false;
                buffer[i] = '\0';
                i=0;
                break;
              default:
                buffer[i] = a;
                i++;
                break;
           };    
         } else {
            t++;
            if (t > PIC_TIMEOUT) {
              flag = false;
              t = 0;
              buffer[0] = '-'; 
              buffer[1] = '-'; 
              buffer[2] = '-'; 
              buffer[3] = '\0';
            };
         };
      };
   
    return buffer;
};


void UART_clean_answer() { // очищаем всё что есть в буффере для чтения по UART 
      char a; 
      while(Serial.available()) {
        a = Serial.read();
      };
};


void handleRoot() { // титульная страница для термометра

  char * answer;  // указатель на буффер с ответом
  String message;

    if (server.argName(0)=="name" && server.arg(0)=="AT R") {
      //очищаем полностью буфер от старых сообщений
      UART_clean_answer();
      // Делаем сброс мин и макс температур
      Serial.println("AT R");  // отсылаем на UART комманду сброса температуры
      // Получаем ответ или таймаут
      answer = UART_read_answer();      
  };






  
  message = "<!DOCTYPE html>";
  message += "<html>";
  message += "  <head>";
  message += "        <title>Thermometer</title>";
  message += "        <meta charset='windows-1251'>";
  message += "        <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  message += "        <meta http-equiv='refresh' content='5' >";
  message += "  </head>";
  message += "    <body";
  message += "        style='text-align: center;background-color: darkblue;color: cyan;font-family: monospace;font-size: 28px;'>";
  message += "        <div>";
  message += "            <hr/>";


  UART_clean_answer();
  Serial.println("AT 1");    //читаем текущую температуру из UART
  answer = UART_read_answer();
  message += "<h3>temp: ";
  //message += answer;
  //message += "<br/>";
  message += temp_correct(answer);
  message += "C</h3>";
  
  message += "<hr/>";

  UART_clean_answer();
  Serial.println("AT 2");    //читаем мин температуру из UART
  answer = UART_read_answer();
  message += "min: ";
  //message += answer;
  //message += "<br/>";
  message += temp_correct(answer);
  message += "C<br/>";
  
  UART_clean_answer();
  Serial.println("AT 3");    //читаем макс температуру из UART
  answer = UART_read_answer();
  message += "max: ";
  //message += answer;
  //message += "<br/>";
  message += temp_correct(answer);
  message += "C<br/>";
  
  
   UART_clean_answer();
  Serial.println("AT 4");    //читаем активные датчики
  answer = UART_read_answer();
  message += "active: ";
  message += answer;
  message += "<hr/>";
  
  message += "<form method='post'>"; 
  message += "<input type='hidden' name='name' value='AT R'/>"; 
  message += "<input style='height: 50px;width: 200px;font-size: 22px;color: darkblue;background-color: #0088cc;' ";
  message += "  type='submit' value='RESET'/>"; 
  message += "</form></div></body></html>";
  
  server.send(200, "text/html", message );

};



void handleCommand() {

  char empty = '\0';
  char * answer;  // указатель на буффер с ответом
  String message;

  if ( server.arg(0)=="" ) {
    answer = &empty;
  } else {
    // Пришла команда по HTTP 
    //очищаем полностью буфер от старых сообщений
    UART_clean_answer();
    //если есть команда, то отдаём её в UART
    Serial.print(server.arg(0));
    Serial.println("");
    //Считываем ответ из UART
    answer = UART_read_answer();
  };
  
  message = "<html><body><h1>";
  message += answer;
  message += "</h1>";
  message += "<form method='post'>"; 
  message += "Command: "; 
  message += "<input type='text' name='name' value=''/>"; 
  message += "<input type='submit' value='Send'/>"; 
  message += "</form></body></html>";
  
  server.send(200, "text/html", message );

}

void handleNotFound(){
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}



void setup(void){

  Serial.begin(9600);
  
  WiFi.mode(WIFI_AP_STA);             // клиент и точка доступа
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  };

  //delay(3000);
  
  //Serial.println("");
  //Serial.print("Connected to ");
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    //Serial.println("MDNS responder started");
  }

  server.on("/command", handleCommand);

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "Temp is OK");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}



void loop(void){
  server.handleClient();
}
