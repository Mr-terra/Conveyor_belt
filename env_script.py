import os

Import("env")


def load_env_file(path=".env"):
    if not os.path.exists(path):
        raise Exception(f"[env_script] {path} not found")

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue

            key, val = line.split("=", 1)
            key = key.strip()
            val = val.strip().strip('"').strip("'").rstrip(";")
            os.environ[key] = val


# Charger le .env
load_env_file(os.path.join(env["PROJECT_DIR"], ".env"))

# Récupération des variables
required_vars = [
    "DOLIBARR_URL",
    "DOLIBARR_API_KEY",
    "WIFI_SSID",
    "WIFI_PASS",
]

missing = []

for var in required_vars:
    if not os.getenv(var):
        missing.append(var)

if missing:
    raise Exception(f"[env_script] Missing variables in .env: {', '.join(missing)}")

print("[env_script] Loaded variables:")
for var in required_vars:
    print(f"  - {var}: OK")

# Injection dans le compilateur
env.Append(
    CPPDEFINES=[
        ("DOLIBARR_URL", f'\\"{os.getenv("DOLIBARR_URL")}\\"'),
        ("DOLIBARR_API_KEY", f'\\"{os.getenv("DOLIBARR_API_KEY")}\\"'),
        ("WIFI_SSID", f'\\"{os.getenv("WIFI_SSID")}\\"'),
        ("WIFI_PASS", f'\\"{os.getenv("WIFI_PASS")}\\"'),
    ]
)
