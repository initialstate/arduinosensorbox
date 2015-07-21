// Process.h gives us access to the Process class, which lets
// us communicate with the shell
#include <Bridge.h>
#include <Process.h>
#include "DHT.h"

// Temperature/Humidity Sensor
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Light Sensor
int lightPin = 0;

// Motion Sensor
int motionPin = 3;

////////////////////////////
// Initial State Streamer //
////////////////////////////
// URL to IS Bucket API
String ISBucketURL = "https://groker.initialstate.com/api/buckets";
// URL to IS Event API
String ISEventURL = "https://groker.initialstate.com/api/events";
// Access key (the one you find in your account settings):
String accessKey = "Your_Access_Key";
// Bucket key (hidden reference to your bucket that allows appending):
String bucketKey = "yun_sensor_box";
// Bucket name (name your data will be associated with in Initial State):
String bucketName = ":house_with_garden: Yun Sensor Box";
// How many signals are in your stream? You can have as few or as many as you want
const int NUM_SIGNALS = 6;
// What are the names of your signals (i.e. "Temperature", "Humidity", etc.)
String signalName[NUM_SIGNALS] = {":sweat_drops: Humidity", ":sunny: Temperature", ":fire: Heat Index", ":bulb: Light", ":wave: Motion", ":bangbang: How Am I Feeling?"};
// This array is to store our signal data later
String signalData[NUM_SIGNALS];


// This only runs once at the very beginning
void setup() 
{
  Bridge.begin();
  Serial.begin(9600);

  dht.begin();

  while (!Serial);
    // Post Bucket
    Serial.println("Posting Bucket!");
    // The postBucket() function creates a bucket 
    // (unnecessary if the bucket already exists)
    postBucket();
}

// This repeats
void loop()
{  
  // Wait 2 seconds for the sensors to initialize
  delay(2000);

  // Gather Data
  takeTempHum();
  takeLight();
  takeMotion();

  // Post Data
  Serial.println("Posting Data!");
  // The postData() function streams our events
  postData(); 
  // Wait for 41 seconds before collecting and sending the next batch
  // This makes total time between values 1 minute
  delay(41000);
}



// Here are the data bucket creation and posting functions
// They do not need to be edited - everything you would need to change for 
// your situation can be found above

void takeTempHum()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    h = 0;
    t = 0;
    f = 0;
    return;
  }

  // Compute heat index in Fahrenheit
  float hif = dht.computeHeatIndex(f, h);

  // Change the emoji displayed based on heat index ranges
  String warning;
  
  if (hif<94 && hif>80)
  {
    warning = ":sweat: :sweat_drops:";
  }
  else if (hif<105 && hif>93)
  {
    warning = ":cold_sweat: :sweat_drops:";
  }
  else if (hif<132 && hif>104)
  {
    warning = ":dizzy_face: :sweat_drops:";
  }
  else if (hif>131)
  {
    warning = ":dizzy_face: :warning:";
  }
  else
  {
    warning = ":grinning:";
  }

  signalData[0] = String(h);
  signalData[1] = String(f);
  signalData[2] = String(hif);
  signalData[5] = String(warning);
}

void takeLight()
{
  int light = analogRead(lightPin);

  signalData[3] = String(light);
}

void takeMotion()
{
  pinMode(motionPin, INPUT);
  int motion = digitalRead(motionPin);
  
  if (motion == HIGH)
  {
    signalData[4] = String(":runner:");
  }
  else
  {
    signalData[4] = String(":no_pedestrians:");
  }  
}

void postBucket()
{
  // Initialize the process
  Process isbucket;

  isbucket.begin("curl");
  isbucket.addParameter("-k");
  isbucket.addParameter("-v");
  isbucket.addParameter("-X");
  isbucket.addParameter("POST");
  isbucket.addParameter("-H");
  isbucket.addParameter("Content-Type:application/json");
  isbucket.addParameter("-H");
  isbucket.addParameter("Accept-Version:0.0.1");

  // IS Access Key Header
  isbucket.addParameter("-H");
  isbucket.addParameter("X-IS-AccessKey:" + accessKey);

  // IS Bucket Key Header
  isbucket.addParameter("-d");
  isbucket.addParameter("{\"bucketKey\": \"" + bucketKey + "\", \"bucketName\": \"" + bucketName + "\"}");
  
  isbucket.addParameter("https://groker.initialstate.com/api/buckets");
  
  // Run the process
  isbucket.run();

  Serial.flush();
}

void postData()
{
  for (int i=0; i<NUM_SIGNALS; i++)
  {
    // Initialize the process
    Process isstreamer;
    
    isstreamer.begin("curl");
    isstreamer.addParameter("-k");
    isstreamer.addParameter("-v");
    isstreamer.addParameter("-X");
    isstreamer.addParameter("POST");
    isstreamer.addParameter("-H");
    isstreamer.addParameter("Content-Type:application/json");
    isstreamer.addParameter("-H");
    isstreamer.addParameter("Accept-Version:0.0.1");

    // IS Access Key Header
    isstreamer.addParameter("-H");
    isstreamer.addParameter("X-IS-AccessKey:" + accessKey);

    // IS Bucket Key Header
    // Note that bucketName is not needed here
    isstreamer.addParameter("-H");
    isstreamer.addParameter("X-IS-BucketKey:" + bucketKey);

    isstreamer.addParameter("-d");

    // Initialize a string to hold our signal data
    String jsonData;

    jsonData = "[";
    jsonData += "{\"key\": \"" + signalName[i] + "\", \"value\": \"" + signalData[i] + "\"}";
    jsonData += "]";

    isstreamer.addParameter(jsonData);

    isstreamer.addParameter("https://groker.initialstate.com/api/events");

    // Print posted data for debug
    Serial.print("Sending event stream #");
    Serial.print(i);
    Serial.println(jsonData);

    // Run the process
    isstreamer.run();

    Serial.flush();
    delay(2000);
  }
}
