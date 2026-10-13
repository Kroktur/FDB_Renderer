@echo off
REM Ne pas toucher a ce fichier

ECHO Création du dossier de compilation...
IF NOT EXIST "Build" mkdir Build
CD Build
ECHO Configuration du projet avec CMake...
cmake .. 
CD..
PAUSE
EXIT /B 0