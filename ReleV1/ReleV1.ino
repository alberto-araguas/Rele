#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "MiCasa"
#define wifi_password "1212cherokee"
#define mqtt_server "192.168.1.55"

#define Estado_Rele1_topic "EstadoRele1"
#define Accion_Rele1_topic "AccionRele1"
#define ONE_WIRE_BUS 14  // DS18B20 pin  D5

/*asignacion pines  ESP8266 */
const int pinRele = 5;   // D8
const int pulsador = 4;  // D7

/*instancias*/
WiFiClient espClient;
PubSubClient client(espClient);

/*Variables Globales*/
String EstadoRele = "Desconocido";
long lastMsg = 0;


/*FUNCIONES*/
void callback(char* topic, byte* payload, unsigned int length) { /*Que hacer cuadno llega un mensaje*/
 //gestionar los mensajes recibidos en los topics subscritos    
   Serial.print("Mensaje recibido [");  Serial.print(topic);  Serial.print("] ");
    String dato="";
    for (int i = 0; i < length ; i++) { 
        Serial.print((char)payload[i]);
        dato=dato+(char)payload[i];
    }
  Serial.println("");
   // Comprobar si hay que subir o bajar
    if ( dato.equals("Encender")) {
    digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
    encender();
  } else if (dato.equals("Bajar")){
    digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
    apagar();
  }  else if (dato.equals("Toggle")){
    toggle();

  }
 }

void setup() {
  pinMode(2, OUTPUT);        /* builtin led*/
  digitalWrite(2, LOW);      /*encender led  */   
  pinMode(pinRele, OUTPUT);  
  pinMode(pulsador, INPUT);  
  digitalWrite(pulsador, HIGH); /*Activar pullups*/
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  digitalWrite(2, HIGH);//apagar led
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Rele1";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("AccionRele1"); /*SUSCRIBIRSE A LOS IN TOPICS*/
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void leer_entradas_pulsadores(){
 int pSubir=digitalRead(pulsador);
 if(pSubir==HIGH){
   toggle();
 }
 delay(500); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  leer_entradas_pulsadores();
  //enviar topics ciclicamente para refrescar datos
 long now = millis();  //temporizar publicaciones
  if (now - lastMsg > 5000) {
    lastMsg = now;
    Publicar();
  }
  
}

void encender(){
 digitalWrite(pinRele,LOW);
 EstadoRele = "Encendido";
}

void apagar(){
 digitalWrite(pinRele,HIGH);
 EstadoRele = "Apagado";
}

void toggle(){
 if (EstadoRele.equals("Apagado")){
  encender();
 } else {
  apagar();
 }
}
void Publicar(){
  client.publish("EstadoRele1", String(EstadoRele).c_str(), true);
}
