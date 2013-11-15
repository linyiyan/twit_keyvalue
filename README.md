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
