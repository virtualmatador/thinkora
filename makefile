TARGET := thinkora
DEBUG := $(if $(shell git symbolic-ref --short HEAD | grep master), , -g)
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp, build/%.o, $(SOURCES))

.PHONY: clean, install, uninstall

all: bin/$(TARGET)

bin/$(TARGET): $(OBJECTS)
	mkdir -p bin
	$(CC) -o $@ $^ -no-pie -lstdc++

define OBJECT_RULE
build/$(subst $() \,,$(shell $(CC) -MM $(1)))
	mkdir -p build
	$(CC) $(DEBUG) -c -o $$@ $$<
endef
$(foreach src, $(SOURCES), $(eval $(call OBJECT_RULE, $(src))))

clean:
	$(RM) -rf build/ bin/

install:
	@echo "thinkora installing ..."
	@echo "thinkora installed."

uninstall:
	@echo "thinkora uninstalling ..."
	@echo "thinkora uninstalled."
