void connecting() {

  /*lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting....");*/
  Serial.println("Connecting....");
  /*lcd.setCursor(0, 1);
  lcd.print(ssid);*/
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);


  /*lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WiFi connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());*/
  Serial.println(WiFi.localIP());
  delay(3000);
  //lcd.clear();
}

void cek_blynk() {
  if (!Blynk.connected() ) {
    koneksi_blynk = "FAIL";
  } else {
    koneksi_blynk = "OK";
  }
}

void reconnecting_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    koneksi_wifi = "OK";
    delay(100);
  } else {
    koneksi_wifi = "FAIL";
    delay(100);
  }

  if ((millis() - last_millis) > 2000) {
    tampilan++;
    if (koneksi_wifi == "FAIL") {
      koneksi_ulang++;
    }
    last_millis = millis();

  }

  if (koneksi_ulang > 10) {
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    delay(100);
    koneksi_ulang = 0;
  }

}
