# ---------------------------
# Makefile pour PlatformIO
# ---------------------------

# Commandes PlatformIO
PIO = pio

# Nom de l'environnement (défini dans platformio.ini)
ENV = m5stack-grey

# ---------------------------
# Targets
# ---------------------------

# Compilation seulement
build:
	$(PIO) run -e $(ENV)

# Upload sur la carte
upload:
	$(PIO) run -e $(ENV) --target upload

# Nettoyer les fichiers temporaires de build
clean:
	$(PIO) run -e $(ENV) --target clean

# Monitor série
monitor:
	$(PIO) device monitor -e $(ENV)

# Rebuild complet et upload
rebuild: clean build upload

# Aide
help:
	@echo "Makefile PlatformIO pour M5Stack"
	@echo "Targets disponibles :"
	@echo "  make build     -> Compile le projet"
	@echo "  make upload    -> Upload sur la carte"
	@echo "  make clean     -> Supprime les fichiers de build"
	@echo "  make monitor   -> Ouvre le moniteur série"
	@echo "  make rebuild   -> Clean + build + upload"