.PHONY: all test test_web clean

all:
	$(MAKE) -C src/c

test: all
	$(MAKE) -C src/c test
	$(MAKE) test_web

test_web: all
	node tests/web_regression.js
	node tests/verify_scheduler.js

clean:
	$(MAKE) -C src/c clean
