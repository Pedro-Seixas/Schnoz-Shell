OBJECTS = shell.o main.o

TARGET = schnoz
all = schnoz

shell.o: shell.cpp shell.h
	g++ -g -c shell.cpp

main.o: main.cpp shell.h 
	g++ -g -c main.cpp

schnoz: main.o shell.o
	g++ -o schnoz main.o shell.o -lncurses

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

uninstall: $(TARGET)
	sudo rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
