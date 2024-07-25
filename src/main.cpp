
#include <Arduino.h>
#include <otaserver.h>
#include <kgfx.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define INFLUXDB_URL "YOUR_INFLUXDB_URL"
#define INFLUXDB_TOKEN "YOUR_INFLUX_DB_TOKEN"
#define INFLUXDB_ORG "YOUR_INFLUXDB_ORG"
#define INFLUXDB_BUCKET "YOUR_INFLUXDB_BUCKET"
#define QUERY "YOUR_INFLUXDB_QUERY"
// Example:  "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -90m) |> filter(fn: (r) => r._measurement == \"mqtt_consumer\" and r._field == \"uplink_message_decoded_payload_humidity\") |> tail(n:30, offset: 0)"

WiFiMulti wifiMulti;
#define DEVICE "ESP32"
OTAServer otaserver;
KGFX ui;
InfluxDBClient InfluxClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
TFT_eSprite textSpr = ui.createSprite(240, 150);

void setup() {
  Serial.begin(460800);
  Serial.println("Starting app");

  otaserver.connectWiFi(); // DO NOT EDIT.
  otaserver.run(); // DO NOT EDIT
  ui.init();
  ui.clear();
  // Check server connection
  if (InfluxClient.validateConnection()) {
    ui.drawText(textSpr, "Connected to InfluxDB", Arial_28, TFT_GREEN, 0, 0);
    Serial.println(InfluxClient.getServerUrl());
  }
  else {
    ui.drawText(textSpr, "InfluxDB connection failed", Arial_28, TFT_RED, 0, 0);
    Serial.println(InfluxClient.getLastErrorMessage());
    while(1) {
      delay(1000); // Infinite loop
      if ((WiFi.status() == WL_CONNECTED)) { // DO NOT EDIT
        otaserver.handle(); // DO NOT EDIT
      }
    }
  }
  delay(5000); // Wait 5s to show message on screen
  ui.clear();
  ui.createChartSpriteLarge(240, 150);
  ui.drawText(textSpr, "Humidity", Arial_28, TFT_BLUE, 45, 0);
  ui.drawText(textSpr, "Last:", Arial_18, TFT_BLUE, 0, 50);
}

void loop() {
  if((WiFi.status() == WL_CONNECTED)) {
    otaserver.handle(); // DO NOT EDIT
  }
  Serial.println("==== List results ====");
  // Send query to the server and get result
  FluxQueryResult result = InfluxClient.query(QUERY);
  std::vector<float> arr;
  float a[30];
  int i = 0;
  // Iterate over rows. Even there is just one row, next() must be called at least once.
  while (result.next()) {
    // Print the value of the "_value" column
    FluxValue value = result.getValueByName("_value");
    double v = value.getDouble(); // if your value isnt a Double, you can use getBool(), getInt(), getString() or getULong() here instead
    Serial.print(v);
    Serial.print(", ");
    a[i++] = v;
    arr.push_back((float)v);
  }
  Serial.println();
  int color = TFT_GREEN;
  if (a[29] > 30) {
    color = TFT_RED; // high humidity!
  }
  else if (a[29] < 30 && a[29] > 20) {
    color = TFT_YELLOW; // ok humidity
  }
  else {
    color = TFT_GREEN; // low humidity
  }
  String f = String(a[29]).c_str();
  int len = f.length();
  char buffer[len+2];
  sprintf(buffer, "%s%%", String(a[29]).c_str());
  ui.drawText(textSpr, buffer, Arial_24_Bold, color, 90, 50);
  //  String(a[29]).c_str(), Arial_18_Bold, color, 90, 50);
  ui.drawChartLarge(arr, color, 100);
  // Check if there was an error
  if (result.getError().length() > 0)
  {
    ui.drawText("Query result error: ", Arial_28, TFT_RED, 0, 0);
    Serial.println(result.getError());
  }

  // Close the result
  result.close();

  // Wait 60s
  Serial.println("Wait 60s");
  delay(60000);

}
