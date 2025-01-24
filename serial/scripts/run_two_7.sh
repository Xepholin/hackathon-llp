#!/bin/bash
#SBATCH --job-name=prog_c7g       # Nom de la tâche
#SBATCH --nodes=1                # Nombre de noeuds
#SBATCH --ntasks=1               # Nombre de tâches
#SBATCH --cpus-per-task=64        # Nombre de processeurs par tâche
#SBATCH --threads-per-core=1
#SBATCH --mem=90G                 # Mémoire par tâche
#SBATCH --time=24:00:00          # Temps limite (HH:MM:SS)
#SBATCH --partition=c7g # Partition spécifique

# Chargez les modules nécessaires (facultatif, selon votre configuration)
# module load module1
# module load module2
module load openmpi5

# Commandes a exécuter sur chaque n�[m~Sud
# (remplacez ces lignes par les commandes réelles de votre script)

export SRUN_CPUS_PER_TASK=64
OMP_NUM_THREADS=64 OMP_PLACES=cores OMP_PROC_BIND=close ./c7g.exe 1000000 1000000
