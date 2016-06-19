# variables
CC = gcc

UDPSERVER = server
UDPCLIENT = client
LIBCOMMON = libcommon
LIBSERVER = libserver
LIBCLIENT = libclient

# rules
all: clean ${UDPSERVER} ${UDPCLIENT} cleanobj

udp: clean ${UDPSERVER} ${UDPCLIENT} cleanobj

${UDPSERVER}: ${LIBCOMMON}.o 
${UDPCLIENT}: ${LIBCOMMON}.o 
${LIBCOMMON}.o: ${LIBCOMMON}.h


clean:
	rm -f *.o ${UDPSERVER} ${UDPCLIENT}
cleanobj:
	rm -f *.o

