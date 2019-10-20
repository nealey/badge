BOARD = --board esp8266:esp8266:nodemcuv2

verify: badge.ino
	arduino --verify $(BOARD) $<
	
install: badge.ino
	arduino --upload $(BOARD) $<
