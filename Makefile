CPPC     = g++
CPPFLAGS = -Wall -pedantic -std=gnu++11 -DNANOPASS_LINUX `pkg-config --cflags gtk+-3.0`
CPPLIBS  = `pkg-config --libs gtk+-3.0`
CC       = gcc
CFLAGS   = -Wall -pedantic -std=gnu99
OUT      = out/linux
EXE      = $(OUT)/nanopass
OBJS     = $(OUT)/main_app.o $(OUT)/main.o $(OUT)/key.o $(OUT)/message.o $(OUT)/create_db.o $(OUT)/decrypt_db.o \
           $(OUT)/editor.o $(OUT)/tabtree.o $(OUT)/tabtreeparser.o $(OUT)/rijndael.o $(OUT)/aes_ctr_256.o       \
           $(OUT)/hmac_sha3_256.o $(OUT)/pbkdf.o $(OUT)/db.o $(OUT)/config.o

all: out $(OUT) $(OBJS) $(EXE)

out:
	mkdir -p out

${OUT}:
	mkdir -p ${OUT}

$(EXE): $(OBJS)
	$(CPPC) $(CPPFLAGS) -o $@ $(OBJS) $(CPPLIBS)

$(OUT)/main_app.o: src/main_app.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/main.o: src/gtk3/main.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/key.o: src/resource/key.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/message.o: src/gtk3/message.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/create_db.o: src/gtk3/create_db.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/decrypt_db.o: src/gtk3/decrypt_db.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/editor.o: src/gtk3/editor.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/config.o: src/config.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/db.o: src/db.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/tabtree.o: src/tabtree/tabtree.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/tabtreeparser.o: src/tabtree/tabtreeparser.cpp
	$(CPPC) $(CPPFLAGS) -c -o $@ $< $(CPPLIBS)

$(OUT)/rijndael.o: src/crypto/rijndael.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/aes_ctr_256.o: src/crypto/aes_ctr_256.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/hmac_sha3_256.o: src/crypto/hmac_sha3_256.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/pbkdf.o: src/crypto/pbkdf.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OUT)
