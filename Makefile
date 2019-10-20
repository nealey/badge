BOARD = --board esp8266:esp8266:nodemcuv2

verify: hello.ino
	arduino --verify $(BOARD) $<
	
install: hello.ino
	arduino --upload $(BOARD) $<
