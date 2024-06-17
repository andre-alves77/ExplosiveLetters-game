#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Dados.h"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

int acertos = 0;  
int tentativas_restantes = 0;

int in_amarelo = 5; // D1
int in_azul = 4; // D2
int in_cinza = 14; // D5
int in_ciano = 12; // D6

int out_amarelo = 16; // D0
int out_azul = 0; // D3
int out_cinza = 2; // D4
int out_ciano = 13; // D7

int outs[4] = {out_amarelo, out_azul, out_cinza, out_ciano}; // Pinos de saída
int ordem[4] = {in_amarelo, in_azul, in_cinza, in_ciano}; // Pinos de entrada

static const char htmlAntes[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Explosive Letters</title><style>body { font-family: Arial, sans-serif; background-color: #333; color: #fff; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } .container { text-align: center; background: #222; padding: 20px; border-radius: 8px; box-shadow: 0 0 20px rgba(255, 0, 0, 0.5); } h1 { color: #ffcc00; font-size: 2.5em; margin-bottom: 20px; } p { color: #ffcc00; font-size: 1.2em; } .button-container { margin-top: 20px; } .start-button { background-color: #ff0000; color: white; padding: 10px 20px; border: none; border-radius: 5px; font-size: 1.2em; cursor: pointer; transition: background-color 0.3s; } .start-button:hover { background-color: #cc0000; }</style></head><body><div class=\"container\"><h1>Explosive Letters</h1>";
static char script[] PROGMEM = "<script> var intervalo = null; var contador = document.getElementById('contador'); var tempoRestante = parseInt(localStorage.getItem(\"tempoRestante\")) || 60; function atualizarContador() { contador.textContent = `Tempo restante: ${tempoRestante}s`; } function pararTemporizador() { if (intervalo) { clearInterval(intervalo); intervalo = null; } localStorage.setItem(\"tempoRestante\", tempoRestante); } function verificar() { pararTemporizador(); /* Ir para a página de verificação */ } function iniciarTemporizador() { atualizarContador(); intervalo = setInterval(function() { if (!tempoRestante) { tempoRestante = 60; pararTemporizador(); /* Ir para a página de game over */ return; } tempoRestante--; atualizarContador(); }, 1000); } iniciarTemporizador(); </script>";

static const char htmlDepois[] PROGMEM = "</div></body></html>\r\n";

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { 
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; 
  }
  server.send(404, "text/plain", message);
}

void indexPage(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(htmlAntes);
  server.sendContent("<p>Conecte os fios corretos para desativar a bomba!</p><div class=\"button-container\"><a href=\"/startGame\"><button class=\"start-button\">Iniciar Jogo</button></a></div>");
  server.sendContent(htmlDepois);
}

void startGame() {
  // Reinicia os acertos quando o jogo começa
  acertos = 0;
  tentativas_restantes = 5;
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(htmlAntes);
  server.sendContent(script);
  server.sendContent("<p>Tentativas Restantes:");
  server.sendContent(String(tentativas_restantes));
  server.sendContent("</p>");
  server.sendContent("<p id=\"contador\"></p>");
  server.sendContent("<div id=\"game-container\"><div class=\"button-container\"><a href=\"/verifyGame\" class=\"start-button\">Verificar</a></div></div>");
  server.sendContent(htmlDepois);
}

void loseGame(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(htmlAntes);
  server.sendContent("<p>BOOOM!</p><div class=\"button-container\"><a href=\"/\" class=\"start-button\">Tente Novamente</a></div>");
  server.sendContent(htmlDepois);
}

void winGame(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(htmlAntes);
  server.sendContent("<p>Parabéns!<br> Você desativou a bomba com sucesso e salvou o dia!</p><div class=\"button-container\"><a href=\"/\" class=\"start-button\">Reiniciar</a></div>");
  server.sendContent(htmlDepois);
}

void verifyGame() {
  int acertos = 0;
  for (int i = 0; i <= 3; i++) {
    digitalWrite(outs[i], HIGH); // Ativa o pino de saída
    int x = digitalRead(ordem[i]); // Lê o valor do pino de entrada
    Serial.print("Valor lido no pino ");
    Serial.print(ordem[i]);
    Serial.print(": ");
    Serial.println(x? "ativo" : "inativo"); // Imprime se o pino está ativo (1) ou inativo (0)

    if (x == 1) {
      acertos++; // Conta um acerto se o pino estiver ativo
    }

    digitalWrite(outs[i], LOW); // Desativa o pino de saída
  }

  if (acertos == 4) {
    winGame();
  } else if (tentativas_restantes <= 1) {
    loseGame();
  } else {
    tentativas_restantes--;
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(htmlAntes);
    server.sendContent(script);
    server.sendContent("<p>Acertos:");
    server.sendContent(String(acertos));
    server.sendContent("</p>");
    server.sendContent("<p>Tentativas Restantes: ");
    server.sendContent(String(tentativas_restantes));
    server.sendContent("<p id=\"contador\"></p>");
    server.sendContent("</p><div class=\"button-container\"><a href=\"/verifyGame\" class=\"start-button\">Verificar</a></div>");
    server.sendContent(htmlDepois);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("OPA");

  for (int x = 0; x <= 3; x++) {
    pinMode(outs[x], OUTPUT);
    pinMode(ordem[x], INPUT);
  }

  Serial.println(ssid);
  Serial.println(password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Esperando pela conexão
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", indexPage);
  server.on("/startGame", startGame);
  server.on("/verifyGame", verifyGame);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  MDNS.update();
}




















