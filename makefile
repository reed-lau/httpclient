all:http_client

http_client:http_client.cc
	g++ $< -o $@ -O2 -g 
