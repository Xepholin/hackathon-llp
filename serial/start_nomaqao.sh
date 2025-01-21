#!/bin/bash
#SBATCH --job-name=test_optim       # Nom de la tâche
#SBATCH --nodes=1                # Nombre de noeuds
#SBATCH --ntasks=1               # Nombre de tâches
# #SBATCH --cpus-per-task=4        # Nombre de processeurs par tâche
#SBATCH --mem=1G                 # Mémoire par tâche
#SBATCH --time=00:10:00          # Temps limite (HH:MM:SS)
#SBATCH --partition=c8g # Partition spécifique

# Chargez les modules nécessaires (facultatif, selon votre configuration)
# module load module1
# module load module2

# Commandes a exécuter
# Les lignes s'exécutent une à une

OMP_PLACES=cores OMP_PROC_BIND=close ./tested_program.exe 1000000 1000
