.PHONY: serverA
serverA: serverA
	./serverA

.PHONY: serverB
serverB: serverB
	./serverB

.PHONY: serverC
serverC: serverC
	./serverC

.PHONY: aws
aws: aws
	./aws

.PHONY: monitor
monitor: monitor
	./monitor


all: client.c aws.c serverA.c serverB.c
	gcc -o client client.c
	gcc -o aws aws.c
	gcc -o monitor monitor.c
	gcc -o serverA serverA.c
	gcc -o serverB serverB.c
	gcc serverC.c -lm -o serverC

clean:
	rm client aws monitor serverA serverB serverC
