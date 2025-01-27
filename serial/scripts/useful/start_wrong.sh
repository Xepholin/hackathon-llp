#!/bin/bash
#SBATCH --job-name=test_optim    # Nom de la tâche
#SBATCH --nodes=1                # Nombre de noeuds
#SBATCH --ntasks=1               # Nombre de tâches
#SBATCH --mem=10G                # Mémoire par tâche
#SBATCH --time=00:10:00          # Temps limite (HH:MM:SS)
#SBATCH --partition=c8g          # Partition spécifique

module load openmpi5

OMP_NUM_THREADS=96 OMP_PLACES=cores OMP_PROC_BIND=close OMP_SCHEDULE=static,1000 ./maqao OV -R1 --executable="tested_program.exe" --run-command="<executable> 1000000 1000"
