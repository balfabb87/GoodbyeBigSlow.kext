NAME := GoodbyeBigSlow

PROJ := $(NAME).xcodeproj
KEXT := build/Release/$(NAME).kext
BIN  := $(KEXT)/Contents/MacOS/$(NAME)
DEPS := $(PROJ)/project.pbxproj $(NAME)/Info.plist \
	$(NAME)/$(NAME).hpp $(NAME)/$(NAME).cpp
INSTALL_DIR := /Library/Extensions

# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/
# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KEXTConcept/
# https://stackoverflow.com/questions/36445097/load-os-x-kext-in-the-early-boot-process
all: $(BIN)

$(BIN): $(DEPS)
	xcodebuild -configuration Release -project $(PROJ)

install: all $(KEXT)
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp -R $(KEXT) $(INSTALL_DIR)
	sudo kextload -v $(INSTALL_DIR)/$(NAME).kext
	sudo touch $(INSTALL_DIR)

uninstall:
	sudo kextunload -v $(INSTALL_DIR)/$(NAME).kext || true
	sudo rm -v -R $(INSTALL_DIR)/$(NAME).kext

.PHONEY: all install uninstall
