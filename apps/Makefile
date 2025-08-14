SOURCES := $(wildcard */Makefile)
apps := $(foreach source,$(SOURCES),$(dir $(source)))

assets:
	cp ./lib/assets/*.* ../../../bin

$(apps):
	$(MAKE) -C $@ $(TARGET)

all:
	$(foreach app, $(apps), $(MAKE) -C $(app) $@;)

clean:
	@echo "Cleaning build artifacts in $(PWD)"
	@find . -type f -name '*.o' -exec rm -f {} +
	@rm -f *.bin *.elf *.dis

install:
	$(foreach app, $(apps), $(MAKE) -C $(app) $@;)

.PHONY: $(apps) all clean install assets