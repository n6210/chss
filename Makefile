INSTALL_DIR   = /usr/local/bin/
#CROSS-COMPILE = arm-linux-gnueabihf-

CC            = $(CROSS-COMPILE)gcc
STRIP         = $(CROSS-COMPILE)strip
AR            = $(CROSS-COMPILE)ar
CFLAGS        = -Wall -DDEBUG -D_GNU_SOURCE -D_XOPEN_SOURCE=500

STATIC	      += -static

MODULES_P     = chss.c
OUTPROG       = chss
LIB			  =


all:
	@echo "============================================================"
	@echo "| Compiling: $(MODULES) $(MODULES_P)"
	@echo "| Linking --> $(OUTPROG)"
	@echo "============================================================"
	$(CC) $(CFLAGS) $(MODULES_P) -c
	$(CC) *.o -o $(OUTPROG) $(LIB) $(STATIC)
	$(STRIP) $(OUTPROG)
	@echo "============================================================"
	@echo "| Done"
	@echo " ==========================================================="

clean:
	rm -f *.o $(OUTPROG) $(PROGRAM1)

install:
	install -s $(OUTPROG) $(INSTALL_DIR)
	@echo -e "Installation complete"

deinstall:
	rm -f $(INSTALL_DIR)/$(OUTPROG)
	@echo -e "Deinstallation complete"

