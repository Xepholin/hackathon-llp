
compil:
	g++ -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-all-loops -ffinite-math-only -funsafe-math-optimizations -fno-math-errno -ftree-vectorize -finline-functions -flto -I/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/include -larmpl -L/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/lib -larmpl_mp -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

no_omp:
	g++ -mcpu=neoverse-512tvb -O3 -funroll-all-loops -ffinite-math-only -funsafe-math-optimizations -fno-math-errno -ftree-vectorize -finline-functions -flto -I/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/include -larmpl -L/tools/acfl/24.10/armpl-24.10.1_AmazonLinux-2_gcc/lib -larmpl_mp -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

armclang:
	armclang++ -mcpu=neoverse-512tvb -O3 -fopenmp -funroll-loops -fvectorize -finline-functions -armpl -lamath -lm -g -fno-omit-frame-pointer BSM.cxx -o tested_program.exe

run:
	sbatch start_nomaqao.sh

maqao:
	sbatch start.sh

maqao_onethread:
	sbatch start_maq1.sh

maqao_mid:
	sbatch start_maqmid.sh

.PHONY: compil run maqao
