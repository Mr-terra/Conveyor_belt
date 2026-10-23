import os

Import("env")

REQUIRED_VARS = [
    "DOLIBARR_URL",
    "DOLIBARR_API_KEY",
    "WIFI_SSID",
    "WIFI_PASS",
]


def load_env_file(path):
    if not os.path.exists(path):
        raise Exception(f"[env_script] {path} not found")

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue

            key, val = line.split("=", 1)
            val = val.strip()
            # Remove one pair of matching quotes, keep the rest of the value as written
            if len(val) >= 2 and val[0] == val[-1] and val[0] in ('"', "'"):
                val = val[1:-1]
            os.environ[key.strip()] = val


def c_string_literal(value):
    if "\n" in value or "\r" in value:
        raise Exception("[env_script] values in .env must be on a single line")
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_config_header(path):
    # The values go into a generated header instead of -D flags: the build tool
    # would otherwise need extra escaping for quotes and backslashes.
    lines = ["#pragma once", ""]
    for var in REQUIRED_VARS:
        lines.append(f"#define {var} {c_string_literal(os.getenv(var))}")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


load_env_file(os.path.join(env["PROJECT_DIR"], ".env"))

missing = [var for var in REQUIRED_VARS if not os.getenv(var)]
if missing:
    raise Exception(f"[env_script] Missing variables in .env: {', '.join(missing)}")

print("[env_script] Loaded variables:")
for var in REQUIRED_VARS:
    print(f"  - {var}: OK")

config_header = os.path.join(env["PROJECT_DIR"], ".pio", "build", env["PIOENV"], "env_config.h")
write_config_header(config_header)

# Force-include the generated header in every translation unit
env.Append(CPPFLAGS=["-include", config_header])
