# @Author Andrej Pavlovič
# @Email <xpavlo14@vutbr.cz>
# @Project DNS Tunneling
# @Program Makefile
# @Details Compiles runnable programs into 'app/', intermediate build files are compiled into 'build/'

.PHONY: all sender receiver clean_build clean

DIR_GUARD=@mkdir -p $(@D)

# "Hack" to compile also header files when changed
HEADERS = \
src/common/base16.h \
src/common/err.h \
src/common/arguments.h \
src/common/definitions.h \
src/sender/dns_sender_events.h \
src/receiver/dns_receiver_events.h

# Usable targets
all: sender receiver # Builds sender & receiver
sender: app/dns_sender # Builds sender
receiver: app/dns_receiver # Builds receiver
clean: # Cleans all compiled files
	@rm -rf build/ app/
	@echo cleaned: build/ app/
clean_build: # Cleans build folder
	@rm -rf build/
	@echo cleaned: build/

# Linking
app/dns_sender: build/dns_sender.o build/base16.o build/err.o build/arguments.o build/dns_sender_events.o build/events.o
	$(DIR_GUARD)
	@gcc -o app/dns_sender build/dns_sender.o build/base16.o build/err.o build/arguments.o build/dns_sender_events.o build/events.o
	@echo built: app/dns_sender
app/dns_receiver: build/dns_receiver.o build/base16.o build/err.o build/arguments.o build/dns_receiver_events.o build/events.o
	$(DIR_GUARD)
	@gcc -o app/dns_receiver build/dns_receiver.o build/base16.o build/err.o build/arguments.o build/dns_receiver_events.o build/events.o
	@echo built: app/dns_receiver

# Sender files (compile & assemble)
build/dns_sender.o: src/sender/dns_sender.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/dns_sender.o src/sender/dns_sender.c
build/dns_sender_events.o: src/sender/dns_sender_events.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/dns_sender_events.o src/sender/dns_sender_events.c

# Receiver files (compile & assemble)
build/dns_receiver.o: src/receiver/dns_receiver.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/dns_receiver.o src/receiver/dns_receiver.c
build/dns_receiver_events.o: src/receiver/dns_receiver_events.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/dns_receiver_events.o src/receiver/dns_receiver_events.c

# Common files (compile & assemble)
build/base16.o: src/common/base16.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/base16.o src/common/base16.c
build/err.o: src/common/err.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/err.o src/common/err.c
build/arguments.o: src/common/arguments.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/arguments.o src/common/arguments.c
build/events.o: src/common/events.c $(HEADERS)
	$(DIR_GUARD)
	@gcc -c -o build/events.o src/common/events.c