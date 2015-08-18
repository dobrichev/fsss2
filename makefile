#gnu compiler
GCXX = g++ 

#intel compiler
ICXX = /opt/intel/bin/icc

FILELIST = main.cpp t_128.cpp fsss2.cpp
TARGET = fsss2

help:
	@echo 'Usage:'
	@echo 'make { gcc_debug | gcc_profiling | gcc_release | intel_debug | intel_profiling | intel_release | clean }'
	@echo 'Where'
	@echo ' gcc = GNU C++ Compiler g++'
	@echo ' intel = Intel C++ Compiler icc'
	@echo ' debug = non-optimized debug binary'
	@echo ' profiling = non-optimized binary with profiling code injected'
	@echo ' release = optimized binary based on previous run on a test case with profiling code injected'
	@echo ''
	@echo 'Example for comiling to optimized binary using GNU C++ Compiler and test file input.txt:'
	@echo 'make clean'
	@echo 'make gcc_profiling'
	@echo './fsss2 < input.txt'
	@echo 'make gcc_release'
	@echo 'make clean'
	@echo './fsss2 < input.txt'

gcc_debug:
	@echo 'Building target $(TARGET) using Gnu C++ Debug settings'
	$(GCXX) -O0 -march=native -g -Wall -o $(TARGET) $(FILELIST)
	@echo 'Done'

gcc_profiling:
	@echo 'Building target $(TARGET) using Gnu C++ Profile Generate settings'
	$(GCXX) -flto -O3 -march=native -g -Wall -fprofile-generate -o $(TARGET) $(FILELIST)
	@echo 'Done'

gcc_release:
	@echo 'Building target $(TARGET) using Gnu C++ Profile Use settings'
	$(GCXX) -flto -O3 -march=native -msse4.2 -mavx -g -Wall -fprofile-use -o $(TARGET) $(FILELIST)
	@echo 'Done'

intel_profiling:
	@echo 'Building target $(TARGET) using Intel C++ Debug settings'
	$(ICXX) -O3 -mtune=native -inline-level=2 -prof-gen -I/usr/include/x86_64-linux-gnu/c++/4.8 -use-intel-optimized-headers -restrict -fargument-noalias -alias-const -fno-alias -xAVX -o $(TARGET) $(FILELIST)
	@echo 'Done'

intel_release:
	@echo 'Building target $(TARGET) using Intel C++ Debug settings'
	$(ICXX) -g -O3 -mtune=native -ipo -inline-level=2 -prof-use -I/usr/include/x86_64-linux-gnu/c++/4.8 -use-intel-optimized-headers -restrict -fargument-noalias -alias-const -fno-alias -xAVX -o $(TARGET) $(FILELIST)
	@echo 'Done'

intel_debug:
	@echo 'Building target $(TARGET) using Intel C++ Debug settings'
	$(ICXX) -g -O0 -I/usr/include/x86_64-linux-gnu/c++/4.8 -use-intel-optimized-headers -restrict -fargument-noalias -alias-const -fno-alias -xAVX -o $(TARGET) $(FILELIST)
	@echo 'Done'

clean:
	rm -f *.o *.gcda *.dyn pgopti.*
