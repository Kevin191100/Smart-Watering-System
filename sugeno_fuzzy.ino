#define BLYNK_TEMPLATE_ID "TMPL6uz88UC1e"
#define BLYNK_TEMPLATE_NAME "Monitoring"
#define BLYNK_AUTH_TOKEN "dv42DyG4aDf8UU2orMr530FjcyrAptxK"
//-------------------------

#include <DHT.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


// sambungan pin
#define DHTPIN 18
#define pin_pompa 26
#define pin_kipas 27
#define pin_soil 34
#define DHTTYPE DHT22

// parameter fuzzy
#define brs_suhu 3 //jumlah membership function dari inputan suhu
#define brs_humidity 3
#define brs_soil 3


#define min_suhu 0
#define max_suhu 40

#define min_humidity 0
#define max_humidity 100

#define min_soil 0
#define max_soil 100


//poisi membership function
//mf output
#define off 0
#define sangat_singkat 1
#define singkat 2
#define lama 3
#define sangat_lama 4

//mf untuk input
#define dingin_suhu 0
#define normal_suhu 1
#define panas_suhu 2

//mf posisi input humidity
#define kering_humidity 0
#define normal_humidity 1
#define lembab_humidity 2

#define kering_soil 0
#define normal_soil 1
#define basah_soil 2

//mf posisi output
#define mati 0
#define mendinginkan 1
#define menyiram 1


#define pompa_on digitalWrite(pin_pompa,HIGH)
#define pompa_off digitalWrite(pin_pompa,LOW)

#define kipas_on digitalWrite(pin_kipas,HIGH)
#define kipas_off digitalWrite(pin_kipas,LOW)

//#define led_on digitalWrite(BUILTIN_LED,HIGH)
//#define led_off digitalWrite(BUILTIN_LED,LOW)


#define pendingin 0.1


//variable fuzzy
double rule[30];
double bobot_suhu[brs_suhu]; // menampung intensitas dari nilai masing2 mf
double bobot_humidity[brs_humidity];
double bobot_soil[brs_soil];

//+++++membership function
double mf_sen_suhu[3][3] = {
  {0, 10, 19 }, //dingin_suhu
  { 15, 25, 32 },//normal_suhu
  { 31, 35, 40}//panas_suhu
};

double mf_sen_humidity[3][3] = {
  { 0, 5, 45 },//kering
  { 45, 55, 70 },//NORMAL
  { 68.5, 96.5, 100  }//lembab
};

double mf_sen_soil[5][3] = {
  {  0, 4, 36 },//kering
  { 30, 40, 65},//normal
  { 64, 96, 100 }//basah

};

const char* ssid     = "Waras"; // wifi komputer
const char* pass = "Bayarwifijapas"; // password wifi komputer


double mf_pendingin[] = {0, 1};
double mf_penyiram[] = {0, 1};

long old_millis;
int total_rule;
bool kondisi;

int jml_loop_suhu, jml_loop_humidity, jml_loop_soil;
float suhu = 0;
float humidity = 0;
float soil = 0;
String koneksi_blynk = "FAIL";
int rkipas, rpompa;

//memanggil fungsi yang ada didalam library
WiFiClient client_wifi;
DHT dht(DHTPIN, DHTTYPE);

long last_millis_wifi = 0;

int ulang;
bool terhubung;
String koneksi_wifi;
long last_millis;
int tampilan;
int koneksi_ulang;


void setup() {
  Serial.begin(9600);
  //pinMode(BUILTIN_LED, OUTPUT);
  pinMode(pin_pompa, OUTPUT); // menjadikan pompa menjadi output
  pinMode(pin_kipas, OUTPUT); // menjadikan pompa menjadi output

  pompa_off;
  dht.begin();

  int index = 0;
  connecting();

}

long last_on;
int minute, second;
float daya = 0;


void loop() {
  Blynk.run();
  reconnecting_wifi();
  cek_blynk();
  get_sensor(); // membaca sensor INA219

  //  suhu = 30;
  //  humidity = 20;
  //  soil = 59;

  //Fuzzifikasi
  get_bobot_humidity (humidity, 3);
  get_bobot_suhu(suhu, 3);
  get_bobot_soil(soil, 3);

  //Rulebase
  rule[1] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[kering_humidity], bobot_soil[kering_soil] );
  rule[2] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[kering_humidity], bobot_soil[normal_soil] );
  rule[3] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[kering_humidity], bobot_soil[basah_soil] );
  rule[4] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[normal_humidity], bobot_soil[kering_soil] );
  rule[5] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[normal_humidity], bobot_soil[normal_soil] );
  rule[6] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[normal_humidity], bobot_soil[basah_soil] );
  rule[7] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[lembab_humidity], bobot_soil[kering_soil] );
  rule[8] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[lembab_humidity], bobot_soil[normal_soil] );
  rule[9] = min_val( bobot_suhu[dingin_suhu], bobot_humidity[lembab_humidity], bobot_soil[basah_soil] );

  rule[10] = min_val( bobot_suhu[normal_suhu], bobot_humidity[kering_humidity], bobot_soil[kering_soil] );
  rule[11] = min_val( bobot_suhu[normal_suhu], bobot_humidity[kering_humidity], bobot_soil[normal_soil] );
  rule[12] = min_val( bobot_suhu[normal_suhu], bobot_humidity[kering_humidity], bobot_soil[basah_soil] );
  rule[13] = min_val( bobot_suhu[normal_suhu], bobot_humidity[normal_humidity], bobot_soil[kering_soil] );
  rule[14] = min_val( bobot_suhu[normal_suhu], bobot_humidity[normal_humidity], bobot_soil[normal_soil] );
  rule[15] = min_val( bobot_suhu[normal_suhu], bobot_humidity[normal_humidity], bobot_soil[basah_soil] );
  rule[16] = min_val( bobot_suhu[normal_suhu], bobot_humidity[lembab_humidity], bobot_soil[kering_soil] );
  rule[17] = min_val( bobot_suhu[normal_suhu], bobot_humidity[lembab_humidity], bobot_soil[normal_soil] );
  rule[18] = min_val( bobot_suhu[normal_suhu], bobot_humidity[lembab_humidity], bobot_soil[basah_soil] );

  rule[19] = min_val( bobot_suhu[panas_suhu], bobot_humidity[kering_humidity], bobot_soil[kering_soil] );
  rule[20] = min_val( bobot_suhu[panas_suhu], bobot_humidity[kering_humidity], bobot_soil[normal_soil] );
  rule[21] = min_val( bobot_suhu[panas_suhu], bobot_humidity[kering_humidity], bobot_soil[basah_soil] );
  rule[22] = min_val( bobot_suhu[panas_suhu], bobot_humidity[normal_humidity], bobot_soil[kering_soil] );
  rule[23] = min_val( bobot_suhu[panas_suhu], bobot_humidity[normal_humidity], bobot_soil[normal_soil] );
  rule[24] = min_val( bobot_suhu[panas_suhu], bobot_humidity[normal_humidity], bobot_soil[basah_soil] );
  rule[25] = min_val( bobot_suhu[panas_suhu], bobot_humidity[lembab_humidity], bobot_soil[kering_soil] );
  rule[26] = min_val( bobot_suhu[panas_suhu], bobot_humidity[lembab_humidity], bobot_soil[normal_soil] );
  rule[27] = min_val( bobot_suhu[panas_suhu], bobot_humidity[lembab_humidity], bobot_soil[basah_soil] );

  //defuzzi
  double total_rule = 0;
  for (int xx = 1; xx <= 27; xx++) {
    total_rule = rule[xx] + total_rule;
  }

  double defuzzy_pendingin = 0;
  defuzzy_pendingin = rule[1] * mf_pendingin[mendinginkan] +
                      rule[2] * mf_pendingin[mendinginkan] +
                      rule[3] * mf_pendingin[mendinginkan] +
                      rule[4] * mf_pendingin[mati] +
                      rule[5] * mf_pendingin[mati] +
                      rule[6] * mf_pendingin[mati] +
                      rule[7] * mf_pendingin[mati] +
                      rule[8] * mf_pendingin[mati] +
                      rule[9] * mf_pendingin[mati] +
                      rule[10] * mf_pendingin[mendinginkan] +
                      rule[11] * mf_pendingin[mendinginkan] +
                      rule[12] * mf_pendingin[mendinginkan] +
                      rule[13] * mf_pendingin[mati] +
                      rule[14] * mf_pendingin[mati] +
                      rule[15] * mf_pendingin[mati] +
                      rule[16] * mf_pendingin[mati] +
                      rule[17] * mf_pendingin[mati] +
                      rule[18] * mf_pendingin[mati] +
                      rule[19] * mf_pendingin[mendinginkan] +
                      rule[20] * mf_pendingin[mendinginkan] +
                      rule[21] * mf_pendingin[mendinginkan] +
                      rule[22] * mf_pendingin[mendinginkan] +
                      rule[23] * mf_pendingin[mendinginkan] +
                      rule[24] * mf_pendingin[mendinginkan] +
                      rule[25] * mf_pendingin[mendinginkan] +
                      rule[26] * mf_pendingin[mendinginkan] +
                      rule[27] * mf_pendingin[mendinginkan];

  double defuzzy_penyiram = 0;
  defuzzy_penyiram = rule[1] * mf_penyiram[menyiram] +
                     rule[2] * mf_penyiram[mati] +
                     rule[3] * mf_penyiram[mati] +
                     rule[4] * mf_penyiram[menyiram] +
                     rule[5] * mf_penyiram[mati] +
                     rule[6] * mf_penyiram[mati] +
                     rule[7] * mf_penyiram[menyiram] +
                     rule[8] * mf_penyiram[mati] +
                     rule[9] * mf_penyiram[mati] +
                     rule[10] * mf_penyiram[menyiram] +
                     rule[11] * mf_penyiram[mati] +
                     rule[12] * mf_penyiram[mati] +
                     rule[13] * mf_penyiram[menyiram] +
                     rule[14] * mf_penyiram[mati] +
                     rule[15] * mf_penyiram[mati] +
                     rule[16] * mf_penyiram[menyiram] +
                     rule[17] * mf_penyiram[mati] +
                     rule[18] * mf_penyiram[mati] +
                     rule[19] * mf_penyiram[menyiram] +
                     rule[20] * mf_penyiram[mati] +
                     rule[21] * mf_penyiram[mati] +
                     rule[22] * mf_penyiram[menyiram] +
                     rule[23] * mf_penyiram[mati] +
                     rule[24] * mf_penyiram[mati] +
                     rule[25] * mf_penyiram[menyiram] +
                     rule[26] * mf_penyiram[mati] +
                     rule[27] * mf_penyiram[mati];


  defuzzy_penyiram = defuzzy_penyiram / total_rule;
  defuzzy_pendingin = defuzzy_pendingin / total_rule;

  Serial.print("Defuzzy pendingin:");
  Serial.println(defuzzy_pendingin);

  Serial.print("Defuzzy penyiram:");
  Serial.println(defuzzy_penyiram);

  if (isnan(defuzzy_pendingin )) {
    defuzzy_pendingin = 0;
  }

  if (isnan(defuzzy_penyiram )) {
    defuzzy_penyiram = 0;
  }

  if (kondisi == 0) {
    if (defuzzy_pendingin <= 0.4) {
      kipas_on;
    } else {
      kipas_off;
    }

    if (defuzzy_penyiram <= 0.4) { 
      pompa_on;
    } else {
      pompa_off;
    }
  } else {
    if (rpompa == 1) {
      pompa_off;
    } else {
      pompa_on;
    }

    if (rkipas == 1) {
      kipas_off;
    } else {
      kipas_on;
    }
  }
  Serial.print("T:");
  Serial.print( suhu);
  Serial.print(" H:");
  Serial.print( humidity);
  Serial.print(" S:");
  Serial.print( soil);
  Serial.print(" Penyiram:");
  Serial.print( defuzzy_penyiram);
  Serial.print(" Pendingin:");
  Serial.print( defuzzy_pendingin);
  Serial.print(" Mode:");
  Serial.print(kondisi);
  Serial.print(rpompa);
  Serial.println(rkipas);
  delay(3000);

  
  Blynk.virtualWrite(V0, suhu);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, soil);
  Blynk.virtualWrite(V3, defuzzy_penyiram);
  Blynk.virtualWrite(V4, defuzzy_pendingin);

}


BLYNK_CONNECTED() {
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);

}

BLYNK_WRITE(V5) {
  kondisi = param.asInt();
}

BLYNK_WRITE(V6) {
  rpompa = param.asInt();
}
BLYNK_WRITE(V7) {
  rkipas = param.asInt();
}
