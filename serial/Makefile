
compil:
	g++ -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-loops -ftree-vectorize -finline-functions -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

armclang:
	armclang -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-loops -f-vectorize -finline-functions -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

run:
	sbatch start_nomaqao.sh

maqao:
	sbatch start.sh

.PHONY: compil run maqao
