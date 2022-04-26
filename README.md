# Alcool Test
This is the individual project for the IoT course 2022 of the master degree "Engineering in computer science" of the "Sapienza" university.<br>
The project was born by observing that in most of the services of car sharing, people can drive the car even if they are drunk since there is not a check on their conditions. Indeed to drive a car you need just to open it with the mobile application, take the keys inside and start to drive. To solve the problem I created a cloud-based IoT breathalyzer connected to a box containing the keys of the car; if the test returns a negative value the box will open otherwise it remains closed.

## Architecture

### Sensors
The sensors used are the ultrasonic sensor and the MQ-135 Gas sensor (which also measures alcohol level in the air).
- #### Ultrasonic Sensor (HC SR04)
It is used for computing a correct measure by the alcohol sensor. Indeed only if the distance is smaller than 5 cm the MQ-135 sensor will be activated and can measure the correct level of the alcohol of the person who is doing the alcohol test. The distance is estimated by sending a trigger signal and receiving echo signal; the time (in us) computed divided by 58 is the distance value in cm of the object in front of the ultrasonic sensor. It can measure distances in the range of 2-400 cm and the ranging accuracy can reach to 3 mm. 
- #### MQ 135
It measures the alcohol level of the person that is blowing on the sensor. It is analog 

As soon as the car is opened through the mobile app, the sensing done by the ultrasonic sensor is periodic because the alcohol test can be done at any moment by the person who wants to drive the car. When the box containing the keys has been opened, the sensor stops to continuously take measures until they are put again in the box and the last one is locked by pressing the button on it. The MQ 135 sensor is activated only when the distance computed by the ultrasonic sensor is smaller than 5 cm, so a correct measure can be taken.
