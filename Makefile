# NOTE: THE NAME CANNOT BE CHANGED IN JUST ONE PLACE #
NAME := GoodbyeBigSlow

KEXT_ID      := jakwings.kext.$(NAME)
KEXT_VERSION := 2022.2.26

MACOS_VERSION_MIN := 11.6

PROJ := $(NAME).xcodeproj
KEXT := build/Release/$(NAME).kext
BIN  := $(KEXT)/Contents/MacOS/$(NAME)
DEPS := $(PROJ)/project.pbxproj $(NAME)/Info.plist $(NAME)/entitlements.xml \
        $(NAME)/$(NAME).hpp $(NAME)/$(NAME).cpp
INSTALL_DIR := /Library/Extensions

XCODEBUILD_OPTIONS := MARKETING_VERSION=$(KEXT_VERSION) \
                      PRODUCT_NAME=$(NAME) PRODUCT_BUNDLE_IDENTIFIER=$(KEXT_ID) \
                      MODULE_NAME=$(KEXT_ID) MODULE_VERSION=$(KEXT_VERSION) \
                      MACOSX_DEPLOYMENT_TARGET=$(MACOS_VERSION_MIN)

# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/
# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KEXTConcept/
# https://stackoverflow.com/questions/36445097/load-os-x-kext-in-the-early-boot-process
all: $(BIN)

$(BIN): $(DEPS) Makefile
ifeq ($(XCODE),ON)
	xcodebuild -configuration Release -project $(PROJ) $(XCODEBUILD_OPTIONS)
else
	mkdir -p $(KEXT)/Contents/MacOS
	sed -e 's/\$$(MARKETING_VERSION)/$(KEXT_VERSION)/g' -e 's/\$$(PRODUCT_BUNDLE_IDENTIFIER)/$(KEXT_ID)/g' <$(NAME)/Info.plist >$(KEXT)/Contents/Info.plist
	$(CXX) -std=c++11 -stdlib=libc++ -Os $(CFLAGS) $(CPPFLAGS) $(MARCH) -isystem '$(shell xcrun --sdk macosx --show-sdk-path)/System/Library/Frameworks/Kernel.framework/Headers' -mmacosx-version-min=$(MACOS_VERSION_MIN) -static $(NAME)/$(NAME).cpp -o $(BIN) -Xlinker -kext -nostdlib -lkmodc++ -lkmod -lcc_kext -Wall -pedantic
	codesign --force --sign - --entitlements $(NAME)/entitlements.xml --timestamp=none $(KEXT)
endif

install: all $(KEXT)
	./other/check_cpuid.sh
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp -R $(KEXT) $(INSTALL_DIR)
	sudo kextload -v $(INSTALL_DIR)/$(NAME).kext
	sudo touch $(INSTALL_DIR)

uninstall:
	sudo kextunload -v $(INSTALL_DIR)/$(NAME).kext || true
	sudo rm -v -R $(INSTALL_DIR)/$(NAME).kext

clean:
	rm -v -R build

.PHONEY: all install uninstall clean
