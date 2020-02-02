#build an executable named aws from aws.c
.PHONY:serverA
.PHONY:serverB
.PHONY:aws
.PHONY:client


all: serverA.cpp serverB.cpp aws.cpp client.cpp

	g++ serverA.cpp -o serverA

	g++ serverB.cpp -o serverB

	g++ aws.cpp -o  aws

	g++ client.cpp -o client



serverA:
	./serverA

serverB: 
	./serverB

aws: 
	./aws





