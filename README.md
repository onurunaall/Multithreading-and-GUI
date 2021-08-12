# Multithreading-and-GUI

***Since the device and HEX numbers I used in this project are currently used in a defense industry company, some of the information embedded in code has been censored.*** 

This project is the project where I made serial communication with the main control circuit of a communication board of a official device. Multithreading, serial communication were applied in this project. At the same time, since more than one process is performed because multithreading is performed, the two-level data verification technique was used. The working principle of this program is quite simple. The code is basically about sending and receiving related response data. Design architecture is based on serial and ethernet communication. Basically, to receive any data from dummy communication board, first of all, what data is requested should be sent to dummy communication board. This is sent in packets of data over the serial communication channel, as was done before. The data packets sent must be entered into the system in HEX format. By using sender threads or directly sending, the data is sent to the board. After that board gives a response and through COM port and so that the response is obtained. While receiving the data, the code checks the first 6-8 bytes of incoming data byte-one-byte to verify whether the correct data obtained or not. To do that code uses the basic if-else structure. After verification, the system receives the rest of the bytes and initialize the CRC16-CCITT, which is also known as checksum verification (CRC). If CRC was done successfully, that means the correct response was received. Then, the code processes its functions and threads according to the desired work. In this project, I designed a graphical interface to make the system work more effectively, and this interface contains structures that send the relevant data package for the desired tasks. COM port and Baud Rate should be selected in the interface first. This time, the baud rate can be selected at much higher levels than 9600, since the circuit is controlled by controllers (ASIC type controller chip) that are much more powerful than Arduino. COM port can be selected manually. However, the program automatically detects which COM port the system is connected to by scanning all COM ports of the computer in the background. As the program runs, the 'operation time' counter starts to run. As the port is activated, the 'On-Port' counter is activated and the countdown starts. The 'Operation time' counter continues to count until the program is closed, while the 'On-Port' counter stops when the port connection is closed. It starts working again after restarting the port. In the system, after the data package is sent to dummy communication board, dummy communication board sends a response according to the package received. However, serial communication is very fast. If we assume that the Baud Rate is 115200 for this circuit, the transmission time of one bit is 86.8 microseconds. Therefore, it can be said that communication takes place at very high speeds. It is very important to check the accuracy of the data received at this speed. For this, it is vital to apply data validation techniques called Cyclic Redundancy Check (CRC) and Checksum. There are a lot of CRC algorithms. In this case, the CRC16-CCITT algorithm was used. As if other CRC algorithms, CRC16-CCITT has some specifications. The width of the algorithm is 16 bits, the truncated polynomial has to be 0x1021 and initial CRC value has to be 0x8000. However, by changing truncated polynomial and initial value, different variations of the algorithm can be created. The code that was used for CRC implementation is shared below. In CRC implementation, it needs to use the bitwise operator for the process because it is all about the process in between the bits.

The interface that can run multithreading commands that will work with the device can be seen below;
                    ![image](https://user-images.githubusercontent.com/74546805/129173437-987dbdc3-588b-4c3c-8c32-0a6ce9915980.png)


## Appendix
### Threads
It provides the opportunity to run more than one job in the same process environment. When a process starts running, a thread (main thread) is created and more than one (multi-thread) can be created within the process. Created threads can operate in the same network and different networks. Threads can generally be used as simulations when testing software. Thanks to the system called the asynchronous approach, tasks can be carried out in parallel, without waiting. Besides, in a multi-thread application, one of the threads stops or breaks, and the application continues to run normally through other channels. Some OSs work based on real-time operations which endanger the stability and continuity of the objectives which have to be done in a given time. Because in these systems, time is tremendously critical and the possibility of not be able to finish performing in the given time may cause interference between program components. In this way, in order not to endanger the continuity of the program which are performing, it is a logical way to use multi-threading. For example, by using a multithreaded approach, it is possible to put a data acquisition operations in a thread whereas displaying it in the user interface in another thread. With that way, the OS can perform multiple threads, namely, switches the thread to perform of data acquisition thread, while the user is operating the user interface. Besides, multi-threading is really important for increasing the performance of the program and the machine. Each processor on a computer or on a machine is capable of performing one thread at a time. That is why in single-core computers, the performance can be lower in comparison to a multi-core machine. With multi-core structure, it is possible to apply multi-threading to the program which makes it safer and much efficient. Lastly, for a particular task, using multithreading can provide an opportunity for the program to work parallel with each component of it. For example, one of the threads can receive the data to be processed whereas the other one processes them. And, resultantly, another thread can transmit the response of the received data. By using some built-in functions which focus on improving the efficiency and performance of creatşng and using threads as if LabWindows/CVI have, it is possible to work by not interfering any oıther system component by using thread-local variables etc. LabWindows / CVI has functions to make thread usage easier. The most used of these are Thread pool Thread-safe queues, Thread-safe variables, Thread locks. These functions can be much more diversified. Information on the use of these functions will be given during the presentation.

### Cyclic Redundancy Check(CRC) 
Cyclic Redundancy Check is a function that detects errors in data transmission. There should not be any error while sending or receiving data, and when it does, problems such as some missing parts may be encountered when sending a message to someone. This function is used to prevent this, and in case of any error, the data is sent again and again until it is transmitted without any problem. Thousands of errors must occur in a short time to have a speed loss or delay noticeable by human If these errors do not increase significantly, it would be extremely difficult to detect without any algorithm. Therefore, CRC algorithms should be used for full data validation. CRCs are specifically designed to protect against error types that are common in communication channels. They can realize the consistency of the delivered messages quickly and securely. However, they are not suitable for the changes made in the data by request. If data is degraded in the transmission line, it must be recognized and the data retransmitted. At the end of data transmission, the CRC tries to verify the message over and over again until it is ensured that all the data has been transmitted completely correctly. To determine a CRC code, it is necessary to define an arbitrary polynomial code. In this polynomial, the division algorithm contains divisor, divisor, division and remainder. The important thing here is that the polynomial coefficients are calculated under the arithmetic of the finite field.
