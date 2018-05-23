#include <SPI.h>
#include <Ethernet.h>

int ledPin =  8;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 25, 6);

EthernetServer server(80);

char* parametersNames[4];
char* parametersValues[4];

int const COMAND = 0; 
int const PARAMETER1 = 1;
int const PARAMETER2 = 2; 
int const PARAMETER3 = 3; 

int const PARAMETERSSIZE = 4; 

#define bufferMax 128
int bufferSize;
char buffer[bufferMax];
char result[750];

void setup() {
    Serial.begin(9600);
    
    Ethernet.begin(mac, ip);
    server.begin();
    
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
    
    setupSamplePorts();
}

void setupSamplePorts(){
    pinMode(ledPin, OUTPUT);
}

void loop() {
    EthernetClient client = server.available();
    if (client) {
        Serial.println("new client");
        WaitForRequest(client);
        
        PerformRequestedCommands(client);
        
        client.stop();
        Serial.println("client disonnected");
    }
}

void WaitForRequest(EthernetClient client) { 
    
    bufferSize = 0;  // Sets buffer[] and bufferSize
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            if (c == 'n')
            break;
            else if (bufferSize < bufferMax)
            buffer[bufferSize++] = c;
            else
            break;
        }
    }
    
    ParseReceivedRequest();
}

void ParseReceivedRequest(){
    //Received buffer contains "GET /cmd/param1/param2 HTTP/1.1".  Break it up.
    
    char* metodoHttp = (char*)strtok(buffer, " ");
    char* source = (char*)strtok(NULL, " ");
    char* versaoHttp = (char*)strtok(NULL, " ");
    
    char* slash1;
    char* slash2;
    char* slash3;
    char* slash4;
    
    slash1 = strstr(source, "/") + 1; // Look for first slash
    slash2 = strstr(slash1, "/") + 1; // second slash
    slash3 = strstr(slash2, "/") + 1; // third slash
    slash4 = strstr(slash3, "/") + 1; // fourth slash
    
    // strncpy does not automatically add terminating zero, but strncat does! So start with blank string and concatenate.
    char param0[15];
    char param1[15];
    char param2[15];
    char param3[15];

    param0[0] = 0;
    param1[0] = 0;
    param2[0] = 0;
    param3[0] = 0;
     
    strncat(param0, slash1, slash2-slash1-1);
    strncat(param1, slash2, slash3-slash2-1);
    strncat(param2, slash3, slash4-slash3-1);
    strcat(param3, slash4);
    
    parametersValues[0] = param0;
    parametersValues[1] = param1;
    parametersValues[2] = param2;
    parametersValues[3] = param3;
}

void PerformRequestedCommands(EthernetClient client) {
    if ( strcmp( parametersValues[COMAND], "digitalWrite") == 0 ){
        renderResponse(client, remoteDigitalWrite());
    }
}

char* remoteDigitalWrite(){
    char responseRendered[100];
    responseRendered[0] = 0;
    
    int pin = atoi(parametersValues[PARAMETER1]);
    int state = atoi(parametersValues[PARAMETER2]);
    digitalWrite(pin, state);
    
    if(state != 0){
        strcat(responseRendered, "Pin ");
        strcat(responseRendered, parametersValues[PARAMETER1]);
        strcat(responseRendered, " On");
    }
    else{
        strcat(responseRendered, "Pin ");
        strcat(responseRendered, parametersValues[PARAMETER1]);
        strcat(responseRendered, " Off");
    }
    
    return responseRendered;
}

void renderResponse(EthernetClient client, char* response){
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();
    
    client.println(response);
    client.println();
}

void renderResponseHtml(EthernetClient client, String response){
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();
    
    client.println("<!DOCTYPE HTML>");
    client.println("<html><body>");
    client.println(response);
    client.println("</body></html>");
    client.println();
}

void PrintString(char* label, char* str){
    Serial.print(label);
    Serial.print("=");
    Serial.println(str);
}

void PrintNumber(char* label, int number){
    Serial.print(label);
    Serial.print("=");
    Serial.println(number, DEC);
}
