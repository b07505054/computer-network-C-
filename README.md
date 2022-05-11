# -junior-csie-computer-network-C
csie computer network with C/C++ in linux  
## HW1  
![image](https://user-images.githubusercontent.com/68935450/167392891-8986c999-ee65-44fa-ad70-91dd0bfdc6ba.png)
![image](https://user-images.githubusercontent.com/68935450/167392958-9ea13183-98b3-43ed-9ee6-4cfda820d36d.png)  
## HW2  (lose my code)
implement a simple Network Storage System, with these functions:  
1.  Client can watch a “.mpg” video (streaming) on the server  
2.  Client can upload files to server  
3.  Client can download files from server  
4.  Client can list what files are in the folder of server
For video steaming, you don’t need to send audio. You can just send raw frames (or encoded frames).  
Video streaming requires buffering at both client side and server side. (bonus)  
After upload and download, you need to ensure the files are identical between source and destination.  
In this assignment, all the transmission should be implemented by the socket of TCP.
![image](https://user-images.githubusercontent.com/68935450/167395591-57a6cd5d-2a1e-461c-961f-d8a2cf278b69.png)
## HW3 (go-back-n with congestion control) (an extension homework of HW2)
![image](https://user-images.githubusercontent.com/68935450/167398135-7dbb2080-72c1-4614-a8c3-ce7e7dcfd843.png)  
![image](https://user-images.githubusercontent.com/68935450/167398206-898b023c-221e-4155-9f14-ae36b9fb772f.png)  
![image](https://user-images.githubusercontent.com/68935450/167397913-8a530f99-efa1-45e3-8a5b-dfe2ee9742ad.png)  
![image](https://user-images.githubusercontent.com/68935450/167398001-ec16a7cb-0266-49ea-b846-e98ea24a9795.png)  
what I achive :   
1. Extract video into frames
2. Send frames with go-back-n and congestion control while agent.c is setted sometimes loss  
3. Correctly sends files (ex. .png , .txt ...)  
  
what I need to advance :  
1. Recontract frames into video and playing well
2. buffer of client still needs to go 
