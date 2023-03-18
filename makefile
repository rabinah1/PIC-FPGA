SRC_DIR = ./src
OBJ_DIR = $(SRC_DIR)/objs
TEST_DIR = ./test
TEST_DATA_DIR = ./test_data

.DELETE_ON_ERROR:
.PHONY: all build check sta test_clean clean

build:
	@echo "Compiling project..."
	$(MAKE) -C $(SRC_DIR)
	@echo "Done"
	@echo ""

check:
	@echo "Running tests..."
	$(MAKE) -C $(TEST_DIR)
	@echo "Done"
	@echo ""

sta:
	@echo "Running pylint..."
	@pylint $(TEST_DATA_DIR)/*.py
	@echo "Done"
	@echo ""
	@echo "Running flake8..."
	@flake8 $(TEST_DATA_DIR)/*.py --max-line-length=100
	@echo "Done"

all: build check sta

clean: test_clean
	-rm -f $(SRC_DIR)/main
	-rm -rf $(OBJ_DIR)

test_clean:
	$(MAKE) -C $(TEST_DIR) clean
