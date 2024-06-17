
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

