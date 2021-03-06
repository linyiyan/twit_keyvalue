twit_keyvalue
=============

- A memcached like key-value storage system.
	- A scala implemented front-end proxy server, to take HTTP request. It implements subscribe-post style of message posting.
	- A runtime support for front-end, namely libstore, to do consistent hashing, maintain lease-based cache, and fetch/update data to one of the back-end server.
	- A distributed in-memory key-value storage server.  
	
### Compile
For both backend and libstore, just compile with C++11.
> g++ -std=c++11 -o main *.cpp

For the scala front-end, you can run using sbt
> sbt run

### Configuration
Specify a configuration file for backend for set of ip address and ports, on which backend applications would execute.
Each line is a pair of ip address and port, separated by white space. 
> localhost 8090 

### Test
To test the backend key-value storage, you can send certain http request to the front-end which implements a simple post-subscribe application. Here are some examples.

- Create a user
	- curl "http://localhost:8085/?method=createUser&usrId=usr1"
- Post a message
  - curl "http://localhost:8085/?method=postTwit&usrId=usr1&content=hello"
- Get posted messages
	- curl "http://localhost:8085/?method=getTwits&usrId=usr1"
- Subscribe to another user
  - curl "http://localhost:8085/?method=addSubscription&usrId=usr1&subscribeTo=usr2"
- Get messages from subscriptions
  - curl "http://localhost:8085/?method=getTwitsBySubscription&usrId=usr1"
