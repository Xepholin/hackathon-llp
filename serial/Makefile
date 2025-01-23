
compil:
	g++ -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-loops -ftree-vectorize -finline-functions -I/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/include -larmpl -L/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/lib -larmpl_mp -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

armclang:
	armclang++ -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-loops -fvectorize -finline-functions -armpl -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

run:
	sbatch start_nomaqao.sh

maqao:
	sbatch start.sh

.PHONY: compil run maqao
