# Stat App for [Kublet](https://thekublet.com)

This is a simple app that demonstrates how to use the Kublet API to get a value from an InfluxDB database and display it on a Kublet.

## Usage

You will need to set up the development environment for the Kublet. You can find instructions on how to do that [here](https://developers.thekublet.com/docs/installation/installation/).

**Note:** You will need to already have [VSCode](https://code.visualstudio.com/) installed on your machine, as well as [PlatformIO](https://platformio.org/).

## Configuration

Clone this repository:

  ```bash
  git clone https://github.com/davidgs/stat.git
  ```

Open the project in VSCode:

  ```bash
  code stat
  ```

Open the `src/main.cpp` file and update the following lines with your InfluxDB credentials:

  ```cpp
#define INFLUXDB_URL "YOUR_INFLUXDB_URL"
#define INFLUXDB_TOKEN "YOUR_INFLUX_DB_TOKEN"
#define INFLUXDB_ORG "YOUR_INFLUXDB_ORG"
#define INFLUXDB_BUCKET "YOUR_INFLUXDB_BUCKET"
#define QUERY "YOUR_INFLUXDB_QUERY"
```

The easiest way to set up your query is to use the [InfluxDB UI](https://cloud2.influxdata.com/orgs) to create a query that returns the data you want to display on your Kublet.

![InfluxDB Query Builder](/images/query-builder.png)

Once you have your query, click the Script Editor button to get the query in the correct format for the Kublet.

![InfluxDB Script Editor](/images/script-editor.png)

Finally, make sure to add:

```
|> tail(n: 30, offset: 0)
```

to the end of your query to limit the number of results returned to 30, since the Kublet Graph only handles 30 values.

![InfluxDB Script Editor with tail](/images/script-editor-tail.png)

Finally, change the range to something reasonable for your data. For example, if you are querying data that is updated every 1 minute, you might want to set the range to 35 minutes.

![InfluxDB Script Editor with range](/images/script-with-range.png)

```
from(bucket: "telegraf")
  |> range(start: -90m)
  |> filter(fn: (r) => r["_measurement"] == "cpu")
  |> filter(fn: (r) => r["cpu"] == "cpu-total")
  |> filter(fn: (r) => r["_field"] == "usage_user")
  |> filter(fn: (r) => r["host"] == "davidgs.dev")
  |> tail(n: 30, offset: 0)
```

You can eliminate the `|> yield()` line from the query, as it is not needed for the Kublet, and it will save you a few bytes of code. The `|> aggregateWindow(every: v.windowPeriod, fn: mean)` line is also not needed and can be eliminated.

You will want to edit lines 49 and 50 in `main.cpp` to set the proper labels for your data. The labels are set in the `drawGraph` function.

```cpp
  // Set the labels for the graph
  ui.drawText(textSpr, "Humidity", Arial_28, TFT_BLUE, 45, 0);
  ui.drawText(textSpr, "Last:", Arial_18, TFT_BLUE, 0, 50);
```

You can also set the colors for the labels here.

**Note:** Pay close attention to lines 60 and 61 and adjust according to the kiind of data you are displaying. It is currently set up to display `double` values like 9.98, etc. If you are displaying integers, you will want to adjust the `vector` and `a` variable accordingly.

```cpp
  std::vector<float> arr;
  float a[30];
```

Then, down at line 71:

```cpp
  double v = value.getDouble(); // if your value isnt a Double, you can use getBool(), getInt(), getString() or getULong() here instead
  Serial.print(v);
  Serial.print(", ");
  a[i++] = v;
  arr.push_back((float)v);
```

If your types don't match, you will need to adjust the `getDouble()` function to match your data type.

## Building and Uploading

Once you have your query set up, you can build and upload the code to your Kublet. Make sure that you have deployed the DeveloperUI Kublet app to your device first! It will then display the IP Address of your Kublet on the screen.

```bash
krate build
krate send <IP Address>
```

If all goes well, you should see a message in green that you have successfully connected to InfluxDB, and 5 seconds later, the display should start displaying data. If you see a message in red, you may have an issue with your InfluxDB credentials or query.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
```