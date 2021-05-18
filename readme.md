# Neuroduino

Neuroduino is arduino project used to relay signals between PC and neuroscientific hardware through arduino. It works on basic serial communication and I am providing code for .NET 4.1 (mainly for Unity3D) and python to connect and communicate with the platform. 

This repository containes only the arduino code and designs to construct the device, specific implementations are then in their respecive repositories:

- [Python](https://github.com/hejtmy/neuroduino-python)
- [.NET](https://github.com/hejtmy/neuroduino-net)

## Description

The main reason why not use some existing solutions like fimrata protocol are simply in the timing precision and custom messages necessary in neuroscientific research. Although I would have prefered to use firmata for research, there are some drawbacks. The firmata for example does not by default return internal time of the stimulus and some implementations, namely pymata and pymata-express, are dependent on OS asynchronous monotonic timing solutions, which lack on some platforms (looking at you windows with your 16ms delay). Neuroduino provide arguably simpler solution, but with some added functionality, directed at neuroscientific research (such as in device photoresistor calibration).