# Alcool Test
This is the individual project for the IoT course 2022 of the master degree "Engineering in computer science" of the "Sapienza" university.<br>
The project was born by observing that in most of the services of car sharing, people can drive the car even if they are drunk since there is not a check on their conditions. Indeed to drive a car you need just to open it with the mobile application, take the keys inside and start to drive. To solve the problem I created a cloud-based IoT breathalyzer connected to a box containing the keys of the car; if the test returns a negative value the box will open otherwise it remains closed.

## Architecture of the system

### Sensors
The sensors used are the ultrasonic sensor and the MQ-3 alcohol sensor.

- #### Ultrasonic Sensor (HC SR04):
  It is used to allow the alcohol sensor to compute a correct measure. Indeed it is located near the MQ 3 sensor and only if the distance from the sensors to the person is smaller than 5 cm, the MQ 3 module will start to take measurements of the person’s blood-alcohol level when he breathes out. The distance is estimated by sending a trigger signal and receiving echo signal; the time (in us) computed divided by 58 is the distance value in cm of the object in front of the ultrasonic sensor. It can measure distances in the range of 2-400 cm and the ranging accuracy can reach to 3 mm. (the datasheet of the ultrasonic sensor can be found [here](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)).
- #### MQ 3 sensor:
  It measures the concentration of alcohol in the air. Its detection range goes from 0.04 to 4 mg/l alcohol. It is a metal oxide semiconductor which detects the presence of alcohol vapors in the surroundings by changing resistance. Indeed when the concentration of alcohol becomes higher also the conductivity of the sensor rises. This change in conductivity is converted to an output value that indicates the level of the alcohol. In particular when the value returned is greater than 450, the alcohol level is considered too high and the box key will remain closed. The sensor has both analog output and digital output, but for this project the analog one is used. (the datasheet of the MQ 3 sensor can be found [here](https://www.pololu.com/file/0J310/MQ3.pdf)).


Since the alcohol test can be done at any moment by the person who wants to drive the car, a periodic sensing is done by the ultrasonic sensor (every 5 second a new measure is performed) as soon as the car is opened through the mobile app (by giving power to the system). When the box containing the keys has been opened, the sensor stops to take measures. When the keys are put again in the box and the last one is locked by pressing the button on it, the sensor will start sensing again. The MQ 3 sensor takes measurements only when the distance computed by the ultrasonic sensor is smaller than 5 cm, so a correct measure can be taken from the alcohol sensor.


### Actuators
The actuators used in the system are the servo motor, three leds (mini traffic light), a button and a buzzer.
- #### Servo motor KY66:
  The servo motor is used to open or close the box containing the keys of the car. If the alcohol sensor returns a value smaller than 450, the box will open so the keys can be taken. If the value measured is greater than 450 the box keys will remain closed.
- #### Mini traffic light:
  It has three leds: red, yellow, green. They are used to provide feedback on the distance measured by the ultrasonic sensor. The red led is turned on when the distance is greater than 15 cm; the yellow one is turned on when the distance is between 5cm and 15 cm; the green one is turned on when the distance is smaller than 5 cm. When the green led is on it means that the person is close enough to the sensors and can proceed to make the alcohol test, so the MQ 3 sensor is activated and can measure the alcohol level. (the datasheet of the MQ 3 sensor can be found [here](https://cdn.shopify.com/s/files/1/1509/1638/files/LED_Ampel_Modul_Datenblatt_AZ-Delivery_Vertriebs_GmbH.pdf?v=1607630369)).
- #### Button:
  It is used to close the box keys. When it is pressed the servo motor is activated and the box keys will close. (the datasheet of the button can be found [here](https://www.arduino.cc/documents/datasheets/Button.pdf)).
- #### Buzzer:
  It is used to provide feedback when the breathalyzer returns a value over the limits. When the MQ 3 sensor measures a value greater than 450, the buzzer is turned on for 1 second. (the datasheet of the buzzer can be found [here](https://www.farnell.com/datasheets/2171929.pdf)).

### Overall system

<img src="img/diagram.png" width="700">
In the above image there is a schema of the components that make up the system both at IoT device level and at cloud level. The sensors and the actuators are connected to a SMT NUCLEO-f401re board and the overall IoT device is developed using RIOT-OS. The cloud level is developed entirely using the AWS ecosystem.<br><br>

The IoT device level and the cloud one exchange messages through a communication protocol based on a publish/subscribe mechanism. The board sends the measures taken by the alcohol sensor to the Mosquitto broker using the MQTT-SN protocol. The messages are published under the topic “alcool_level”. Moreover, the board is subscribed to the topic “topic_in” to receive the messages sent from outside, that are used to close or open the box containing the keys. Mosquitto exchanges messages using MQTT with the AWS ecosystem through a transparent bridge, a python script that works as a bridge between Mosquitto and AWS IoT Core. Indeed it publishes the messages of “alcool_level” from the board to IoT Core and takes as input messages published from IoT Core under the topic “topic_in” which are directed to the board.<br>

The messages coming from the board are then stored from IoT Core directly to the DynamoDB by setting a proper rule. They are then displayed on the web dashboard through a call of the REST API, which triggers the lambda function (“get_data_from_db.py”) that gets the data from the database. From the web dashboard it is possible to close or open the box keys by publishing messages under the topic “topic_in”. The messages are published to IoT Core by invoking the REST API which uses another lambda function (“publish_to_iotcore.py”) to perform this action.<br>

AWS Amplify is used to host all the static web content of the web dashboard.



## How the system work






## Network performance





## Walkthrough
