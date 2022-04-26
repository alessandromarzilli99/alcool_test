import paho.mqtt.client as mqtt
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import json
import paho.mqtt.publish as publish


TOPIC_IN = "topic_in"
TOPIC_OUT1 = "alcool_level"
TOPIC_OUT2 = "box"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(TOPIC_OUT1)
    client.subscribe(TOPIC_OUT2)

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    message = {}
    message['message'] = str(msg.payload)[2:-1]
    messageJson = json.dumps(message)
    myAWSIoTMQTTClient.publish(msg.topic, messageJson, 1)

def customCallback(client, userdata, message):
    jsonmessage = json.loads(message.payload)
    publish.single(TOPIC_IN, jsonmessage["message"], hostname="localhost", port=1886, client_id="2")

host = "ak5gv5ndtx775-ats.iot.us-east-1.amazonaws.com"
rootCAPath = "root-CA.crt"
certificatePath = "alcooltest.cert.pem"
privateKeyPath = "alcooltest.private.key"
clientId = "basicPubSub"
port=8883

myAWSIoTMQTTClient = None
myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
myAWSIoTMQTTClient.configureEndpoint(host, port)
myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath)
myAWSIoTMQTTClient.connect()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

myAWSIoTMQTTClient.subscribe(TOPIC_IN, 1, customCallback)

client.connect("localhost", 1886, 60)
client.loop_forever()