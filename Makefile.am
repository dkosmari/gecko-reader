EXTRA_DIST = \
	bootstrap \
	README.md \
	hbc/meta.xml.in \
	hbc/


AM_CPPFLAGS = $(DEVKITPRO_CPPFLAGS)

AM_CXXFLAGS = \
	-Wall -Wextra -Werror \
	$(DEVKITPRO_CXXFLAGS)

AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)

LIBS = $(DEVKITPRO_LIBS)


noinst_PROGRAMS = gecko-reader.elf

gecko_reader_elf_SOURCES = \
	src/main.cpp \
	src/socket.cpp src/socket.hpp

DOL_FILE = gecko-reader.dol

all-local: $(DOL_FILE)


.PHONY: run
run: $(DOL_FILE)
	WIILOAD=tcp:wii wiiload $<


ZIP_FILE = gecko-reader.zip
ZIP_DIR = $(ZIP_FILE:.zip=)
.PHONY: zip
zip: $(ZIP_FILE)
CLEANFILES = $(ZIP_FILE)
$(ZIP_FILE): $(DOL_FILE) hbc/meta.xml hbc/icon.png
	mkdir -p $(ZIP_DIR)
	cp $^ $(ZIP_DIR)/
	mv $(ZIP_DIR)/gecko-reader.dol $(ZIP_DIR)/boot.dol
	zip -r $(ZIP_FILE) $(ZIP_DIR)
	$(RM) -r $(ZIP_DIR)

install-exec-hook: $(ZIP_FILE)
	WIILOAD=tcp:wii wiiload $<


.PHONY: company
company: compile_flags.txt
compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt



@INC_AMINCLUDE@

DISTCLEANFILES = $(AMINCLUDE)
