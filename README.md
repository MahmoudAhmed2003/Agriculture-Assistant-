# Agriculture Assistant

An autonomous robot that utilizes AI for greenhouse plant monitoring and disease detection, designed to minimize crop loss and enhance yields. The project integrates AI-driven disease detection with real-time updates through a mobile app. 

## My Role:
I was responsible for developing and implementing the **hardware** components of the system, ensuring seamless integration between the robot's motion and image-capturing functionality.

## Features:
- **Hardware**: Powered by ESP32 for robot motion control and ESP32Cam for capturing images, sending data, and receiving plant status.
- **AI Disease Detection**: Images captured by the ESP32Cam are analyzed using an AI model to detect plant diseases, with results sent to a mobile app for real-time updates.

## How It Works:
1. **Initialization**: 
   - When the robot is powered on, it waits for the ESP32Cam to connect to the server and Firebase.
   - Only after successful connections does the robot begin its movement.

2. **Navigation**: 
   - The robot follows a black line on the ground, stopping at intersections between plant rows.
   - Each intersection corresponds to a pair of plants, marked by the orthogonal crossing of black lines.

3. **Plant Monitoring**:
   - At each stop, the robot signals the ESP32Cam to take pictures in both directions.
   - The ESP32Cam uploads the images to Firebase, and the URLs are sent to the server.
   - The server processes the images using an AI model to detect any diseases and relays the results back to the robot.

4. **Data Reporting**:
   - The mobile app presents real-time results of plant health, ensuring users are always informed about the status of their crops.

## Technologies Used:
- **ESP32**: For main robot motion control.
- **ESP32Cam**: For image capturing and communication.
- **Firebase**: For cloud storage and real-time updates.
- **AI Model**: For disease detection from plant images.
- **Mobile App**: For displaying real-time monitoring results.
