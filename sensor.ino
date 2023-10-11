//ADS1115 READ

void get_sensor() {
  suhu = dht.readTemperature();
  humidity = dht.readHumidity();
  soil = map(analogRead(pin_soil),2916,866,0,100);
}
