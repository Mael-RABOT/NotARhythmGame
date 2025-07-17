default: help

##@ Helpers
.PHONY: help
help:  ## Display this help
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

##@ Build
.PHONY: build
build:  ## Build the project
	make -C src

##@ Clean
.PHONY: clean
clean:  ## Clean the project
	rm -f NotARhythmGame

##@ Full Clean
.PHONY: fclean
fclean: clean ## Full clean the project
	rm -f src/*.o

##@ Rebuild
.PHONY: re
re: fclean build  ## Rebuild the project
