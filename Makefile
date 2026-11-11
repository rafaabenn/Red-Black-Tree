# Compilation : make        Execution : make demo / make bench
CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall
ifeq ($(OS),Windows_NT)
  EXE = .exe
  MKDIR = if not exist bin mkdir bin
  RM = if exist bin rmdir /s /q bin
else
  EXE =
  MKDIR = mkdir -p bin
  RM = rm -rf bin
endif
PROGS = DemoRBT Benchmark
BINS = $(addprefix bin/,$(addsuffix $(EXE),$(PROGS)))
HEADERS = src/ArbreBinaireRecherche.h src/ArbreRougeNoir.h

all: $(BINS)

bin/%$(EXE): src/%.cpp $(HEADERS)
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

demo: all
	./bin/DemoRBT$(EXE)

bench: all
	./bin/Benchmark$(EXE)

clean:
	$(RM)

.PHONY: all demo bench clean
