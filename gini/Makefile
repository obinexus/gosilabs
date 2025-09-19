# GINI Project Makefile
# "Simple is better than complex" - Python Zen via GINI

.PHONY: all build test clean run help

help: ## Show this help
	@echo "🦜 GINI Build System"
	@echo "==================="
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}'

all: build test ## Build everything and run tests

build: ## Build all components
	@echo "🔨 Building GINI..."
	@cd scripts && ./build.sh

test: ## Run all tests
	@echo "🧪 Testing consciousness threshold..."
	@go test ./src/go/... -v
	@python -m pytest tests/ -v

run: ## Run GINI server
	@echo "🦜 Starting GINI server..."
	@go run src/go/obinexus-gini-server.go

clean: ## Clean build artifacts
	@echo "🧹 Cleaning..."
	@rm -rf build/ dist/
	@find . -name "*.pyc" -delete
	@find . -name "__pycache__" -delete

install: ## Install dependencies
	@echo "📦 Installing dependencies..."
	@go mod download
	@pip install -r requirements.txt

favicon: ## Generate GINI favicon
	@echo "🎨 Creating GINI favicon..."
	@python scripts/merge_favicon.py
