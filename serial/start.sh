#!/bin/bash
#SBATCH --job-name=test_optim       # Nom de la tâche
#SBATCH --nodes=1                # Nombre de noeuds
#SBATCH --ntasks=1               # Nombre de tâches
#SBATCH --cpus-per-task=96        # Nombre de processeurs par tâche
#SBATCH --threads-per-core=1
#SBATCH --mem=10G                 # Mémoire par tâche
#SBATCH --time=00:10:00          # Temps limite (HH:MM:SS)
#SBATCH --partition=c8g # Partition spécifique

# Chargez les modules nécessaires (facultatif, selon votre configuration)
# module load module1
# module load module2
module load openmpi5

# Commandes a exécuter sur chaque n�[m~Sud
# (remplacez ces lignes par les commandes réelles de votre script)

SRUN_CPUS_PER_TASK=96 OMP_NUM_THREADS=96 OMP_PLACES=cores OMP_PROC_BIND=close OMP_SCHEDULE=static,10000 ./maqao OV -R1 --executable="tested_program.exe" --run-command="<executable> 1000000 1000"
