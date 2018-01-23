
ifeq ($(UIO_APPLICATIONS),)
 INSTALL_BASE  = /Applications/prog
else
 INSTALL_BASE  = $(UIO_APPLICATIONS)/prog
endif
INSTALL_EXE    = install -m 0755
INSTALL_BIN    = $(INSTALL_BASE)/bin
INSTALL_DATA   = $(INSTALL_BASE)/mesytec-rc/data

all:

install:
	install -d $(INSTALL_BIN) $(INSTALL_DATA)/serial
	install -m 0644 default.conf log_csv.py mrcc.py *.xpm mesytec-rc.glade $(INSTALL_DATA)
	install -m 0644 serial/*.py $(INSTALL_DATA)/serial
	$(INSTALL_EXE) mesytec-rc.py $(INSTALL_BIN)
