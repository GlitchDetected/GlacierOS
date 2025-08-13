SOURCES := $(wildcard */Makefile)
apps := $(foreach source,$(SOURCES),$(dir $(source)))

assets:
	cp ./lib/assets/*.* ../../../bin

$(apps):
	$(MAKE) -C $@ $(TARGET)

all:
	$(foreach app, $(apps), $(MAKE) -C $(app) $@;)

clean:
	$(foreach app, $(apps), $(MAKE) -C $(app) $@;)

install:
	$(foreach app, $(apps), $(MAKE) -C $(app) $@;)

.PHONY: $(apps) all clean install assets