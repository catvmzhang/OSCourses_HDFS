# OSCourses_HDFS

Apache Hadoop is a collection of open-source software utilities that facilitate using a network of many computers to solve problems involving massive amounts of data and computation.
In this homework, we are going to setup a Hadoop distributed file system with a real time server to handle the multiple data streaming. 

## Components of server:
* File receiver
  * Receive files from multiple clients at the same time.
  * The maximum number of the clients is 25.
  * The size of the file is about 150MB.
* Data processor
  * Saves the data to the HDFS while file receiver receiving the data.

![image](https://github.com/catvmzhang/OSCourses_HDFS/blob/master/os_hw4.PNG)
