#
# author         Oliver Blaser
# date           03.03.2021
# copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser
#

CC = g++
LINK = g++

CFLAGS = -c -I../../src --std=c++17 -O3 -pedantic
LFLAGS = -O3 -pedantic

OBJS = main.o arg.o job.o processor.o cliTextFormat.o util.o version.o
EXE = potoroo

BUILDDATE = $(shell date +"%Y-%m-%d-%H-%M")




$(EXE): $(OBJS)
	$(LINK) $(LFLAGS) -o $(EXE) $(OBJS)

main.o: ../../src/main.cpp ../../src/project.h ../../src/application/arg.h ../../src/application/job.h ../../src/application/processor.h
	$(CC) $(CFLAGS) ../../src/main.cpp

arg.o: ../../src/application/arg.cpp ../../src/application/arg.h ../../src/project.h
	$(CC) $(CFLAGS) ../../src/application/arg.cpp

job.o: ../../src/application/job.cpp ../../src/application/job.h ../../src/project.h ../../src/middleware/cliTextFormat.h
	$(CC) $(CFLAGS) ../../src/application/job.cpp

processor.o: ../../src/application/processor.cpp ../../src/application/processor.h ../../src/project.h ../../src/middleware/cliTextFormat.h
	$(CC) $(CFLAGS) ../../src/application/processor.cpp

cliTextFormat.o: ../../src/middleware/cliTextFormat.cpp ../../src/middleware/cliTextFormat.h ../../src/project.h
	$(CC) $(CFLAGS) ../../src/middleware/cliTextFormat.cpp

util.o: ../../src/middleware/util.cpp ../../src/middleware/util.h ../../src/project.h ../../src/middleware/cliTextFormat.h
	$(CC) $(CFLAGS) ../../src/middleware/util.cpp

version.o: ../../src/middleware/version.cpp ../../src/middleware/version.h ../../src/project.h
	$(CC) $(CFLAGS) ../../src/middleware/version.cpp




all: $(EXE)
	

run: $(EXE)
	@echo ""
	@echo -e "\033[94m--================# run #================--\033[39m"
	./$(EXE) -v

release: $(EXE)
	tar -czf potoroo_$(BUILDDATE).tar.gz potoroo

clean:
	rm $(OBJS)
	rm $(EXE)

help:
	@echo -e "possible targets:"
	@echo -e "- executables   \033[94m$(EXE) all\033[39m"
	@echo -e "- objects       \033[94m$(OBJS)\033[39m"
	@echo -e "- commands      \033[94mrun release clean help\033[39m"
