# NOTE: THE NAME CANNOT BE CHANGED IN JUST ONE PLACE #
NAME := GoodbyeBigSlow

KEXT_ID      := jakwings.kext.$(NAME)
KEXT_VERSION := 2022.5.31

MACOS_VERSION_MIN := 11.6

PROJ := $(NAME).xcodeproj
INSTALL_DIR := /Library/Extensions

KEXT_DIR  := build/Release/$(NAME).kext
KEXT_BIN  := $(KEXT_DIR)/Contents/MacOS/$(NAME)
KEXT_DEPS := Makefile \
             $(PROJ)/project.pbxproj \
             $(NAME)/entitlements.xml \
             $(NAME)/$(NAME).c \
             $(NAME)/$(NAME).hpp \
             $(NAME)/$(NAME).cpp \
             $(NAME)/Info.plist \

PLUGIN_DIR  := $(KEXT_DIR)/Contents/PlugIns/X86PlatformShim.kext
PLUGIN_DEPS := Makefile \
               $(NAME)/X86PlatformShim.plist

# https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/Introduction/Introduction.html
# https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution/resolving_common_notarization_issues/
# https://apple.stackexchange.com/questions/343912/should-i-sign-open-source-code-myself
# https://superuser.com/questions/1436370/how-to-codesign-gdb-on-os-x-mojave
# https://github.com/radareorg/radare2/blob/master/doc/macos.md
CODE_SIGN_IDENTITY := -

# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/
# https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KEXTConcept/
# https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/IOKitFundamentals/
# https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/AccessingHardware/
# https://stackoverflow.com/questions/36445097/load-os-x-kext-in-the-early-boot-process
# $ kextlibs -xml -compatible-versions -undef-symbols -unsupported <kext>
# $ kextstats -a | less
# $ ioreg -bfilw0 -p IOService | less
# https://github.com/apple/darwin-xnu
# $ nm /System/Library/Kernels/kernel | less
# $ otool -xV /System/Library/Kernels/kernel | less
# $ kextfind -rsym _pmCPUControl
all: $(KEXT_BIN)

$(KEXT_BIN): $(KEXT_DEPS) # $(PLUGIN_DIR)
ifeq ($(XCODE),ON)
	xcodebuild -verbose -project $(PROJ) -target GoodbyeBigSlow -configuration Release MARKETING_VERSION=$(KEXT_VERSION) PRODUCT_BUNDLE_IDENTIFIER=$(KEXT_ID) MODULE_NAME=$(KEXT_ID) MODULE_VERSION=$(KEXT_VERSION) MACOSX_DEPLOYMENT_TARGET=$(MACOS_VERSION_MIN) CODE_SIGN_IDENTITY="-"
else
	mkdir -p $(KEXT_DIR)/Contents/MacOS
	sed -e 's/\$$(PRODUCT_BUNDLE_IDENTIFIER)/$(KEXT_ID)/g' -e 's/\$$(MARKETING_VERSION)/$(KEXT_VERSION)/g' -e 's/\$$(MACOSX_DEPLOYMENT_TARGET)/$(MACOS_VERSION_MIN)/g' <$(NAME)/Info.plist >$(KEXT_DIR)/Contents/Info.plist
	$(CXX) $(CFLAGS) $(CPPFLAGS) -DXCODE_OFF -DKEXT_ID=$(KEXT_ID) -DKEXT_VERSION=$(KEXT_VERSION) -nostdinc -std=c++11 -stdlib=libc++ -Os -fno-builtin -fno-exceptions -fno-rtti -fno-common -mkernel -fapple-kext -fasm-blocks -fstrict-aliasing -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -isystem "$(shell xcrun --sdk macosx --show-sdk-path)/System/Library/Frameworks/Kernel.framework/Headers" -mmacosx-version-min=$(MACOS_VERSION_MIN) -static $(NAME)/$(NAME).cpp -o $(KEXT_BIN) -Xlinker -kext -nostdlib -lkmodc++ -lkmod -lcc_kext -pedantic -Wall -Wextra -Wno-extra-semi
endif
	xcrun codesign --force --deep --sign "$(CODE_SIGN_IDENTITY)" --entitlements $(NAME)/entitlements.xml --timestamp=none $(KEXT_DIR)

# TODO: add to GoodbyeBigSlow.xcodeproj ?
$(PLUGIN_DIR): $(PLUGIN_DEPS)
	mkdir -p $(PLUGIN_DIR)/Contents
	sed -e 's/\$$(PRODUCT_BUNDLE_IDENTIFIER)/$(KEXT_ID)/g' -e 's/\$$(MARKETING_VERSION)/$(KEXT_VERSION)/g' -e 's/\$$(MACOSX_DEPLOYMENT_TARGET)/$(MACOS_VERSION_MIN)/g' <$(NAME)/X86PlatformShim.plist >$(PLUGIN_DIR)/Contents/Info.plist
	xcrun codesign --force --deep --sign "$(CODE_SIGN_IDENTITY)" --timestamp=none $(PLUGIN_DIR)

install: all
	./other/check_cpuid.sh
	sudo true
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp -R $(KEXT_DIR) $(INSTALL_DIR)
	sudo kextcache -v 4 -i / || sudo touch $(INSTALL_DIR)
	sudo kextload -v 4 -b $(KEXT_ID)

uninstall:
	sudo true
	sudo kextunload -v 4 -b $(KEXT_ID) || true
	sudo rm -v -R -f $(INSTALL_DIR)/$(NAME).kext
	sudo kextcache -v 4 -i / || sudo touch $(INSTALL_DIR)

load:
	sudo kextload -v 4 -b $(KEXT_ID)

unload:
	sudo kextunload -v 4 -b $(KEXT_ID)

clean:
	rm -v -R -f build

.PHONEY: all install uninstall clean
