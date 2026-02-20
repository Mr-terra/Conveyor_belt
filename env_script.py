import os

Import("env")


def load_env_file(path=".env"):
    if not os.path.exists(path):
        print(f"[env_script] No {path} found")
        return
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, val = line.split("=", 1)
            key = key.strip()
            val = val.strip().strip('"').strip("'").rstrip(";")  # au cas où
            os.environ[key] = val


load_env_file(".env")

dolibarr_url = os.getenv("DOLIBARR_URL")
dolibarr_key = os.getenv("DOLIBARR_API_KEY")

print(f"[env_script] DOLIBARR_URL={'OK' if dolibarr_url else 'MISSING'}")
print(f"[env_script] DOLIBARR_API_KEY={'OK' if dolibarr_key else 'MISSING'}")

if not dolibarr_url or not dolibarr_key:
    raise Exception("Missing DOLIBARR_URL or DOLIBARR_API_KEY in .env")

env.Append(
    CPPDEFINES=[
        ("DOLIBARR_URL", f'\\"{dolibarr_url}\\"'),
        ("DOLIBARR_API_KEY", f'\\"{dolibarr_key}\\"'),
    ]
)
