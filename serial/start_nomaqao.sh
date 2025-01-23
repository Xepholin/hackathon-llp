#!/bin/bash
#SBATCH --job-name=ref.exe       # Nom de la tâche
#SBATCH --nodes=1                # Nombre de noeuds
#SBATCH --ntasks=1               # Nombre de tâches
# #SBATCH --cpus-per-task=4        # Nombre de processeurs par tâche
#SBATCH --mem=10G                 # Mémoire par tâche
#SBATCH --time=00:10:00          # Temps limite (HH:MM:SS)
#SBATCH --partition=c8g # Partition spécifique

# Chargez les modules nécessaires (facultatif, selon votre configuration)
# module load module1
# module load module2
module load openmpi5

# Commandes a exécuter sur chaque n�[m~Sud
# (remplacez ces lignes par les commandes réelles de votre script)

OMP_NUM_THREADS=96 OMP_PLACES=cores OMP_PROC_BIND=close ./tested_program.exe 1000000 1000
