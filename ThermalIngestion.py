import paho.mqtt.client as mqtt
import pymongo
import json
import datetime
import sys

# --- 1. MQTT CONFIGURATION (The Source) ---
MQTT_BROKER = "160.187.144.142"
MQTT_PORT = 1883
MQTT_TOPIC = "student/sensor/Abimanyu3TIA"  # Your unique topic

# --- 2. MONGODB CONFIGURATION (The Destination) ---
# REPLACE THE LINE BELOW with your actual connection string from Atlas or Local
# Format for Atlas: "mongodb+srv://<username>:<password>@cluster0.abcde.mongodb.net/"
# Format for Local with Password: "mongodb://<username>:<password>@localhost:27017/"

MONGO_URI = "URI_MONGO_DB" 

DB_NAME = "DB_NAME"
COLLECTION_NAME = "COLLECTION_NAME"

# --- 3. CONNECT TO DATABASE ---
try:
    print("Connecting to MongoDB...")
    client_mongo = pymongo.MongoClient(MONGO_URI)
    db = client_mongo[DB_NAME]
    collection = db[COLLECTION_NAME]
    # Quick test to see if connection works
    client_mongo.server_info() 
    print(f"SUCCESS: Connected to database '{DB_NAME}'")
except Exception as e:
    print(f"ERROR: Could not connect to MongoDB. Check your URI and Password!")
    print(f"Details: {e}")
    sys.exit() # Stop script if DB fails

# --- 4. MQTT FUNCTIONS ---

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Connected to Broker ({MQTT_BROKER})")
        print(f"Subscribing to: {MQTT_TOPIC}")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"Bad connection. Code: {rc}")

def on_message(client, userdata, msg):
    try:
        # Decode message
        payload = msg.payload.decode("utf-8")
        print(f"\n[NEW DATA] {payload}")
        
        # Convert to JSON/Dictionary
        data = json.loads(payload)
        
        # Add precise timestamp
        data["timestamp"] = datetime.datetime.now()
        
        # Save to MongoDB
        result = collection.insert_one(data)
        print(f" -> Saved to MongoDB ID: {result.inserted_id}")
        
    except Exception as e:
        print(f" -> Error saving data: {e}")

# --- 5. RUN THE PROCESS ---
client_mqtt = mqtt.Client()
client_mqtt.on_connect = on_connect
client_mqtt.on_message = on_message

print(f"Connecting to MQTT Broker...")
try:
    client_mqtt.connect(MQTT_BROKER, MQTT_PORT, 60)
    client_mqtt.loop_forever() # Keep running until you press Ctrl+C
except Exception as e:
    print(f"CRITICAL ERROR: {e}")