SRC_DIR = ./src
TEST_DIR = ./test

.DELETE_ON_ERROR:
.PHONY: all build_app test_run test_clean clean

build_app:
	$(MAKE) -C $(SRC_DIR)

test_run:
	$(MAKE) -C $(TEST_DIR)

all: build_app test_run

clean: test_clean
	-rm $(SRC_DIR)/*.o
	-rm -f $(SRC_DIR)/main

test_clean:
	$(MAKE) -C $(TEST_DIR) clean
