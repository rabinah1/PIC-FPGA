SRC_DIR = ./src
TEST_DIR = ./test

.PHONY: all build_app test_run test_clean clean

build_app:
	make -C $(SRC_DIR)

test_run:
	make -C $(TEST_DIR)

all: build_app test_run

clean: test_clean
	-rm $(SRC_DIR)/*.o
	-rm -f $(SRC_DIR)/main

test_clean:
	make -C $(TEST_DIR) clean
