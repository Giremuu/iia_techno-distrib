import socket, json, sys


PORTS_TO_SCAN = [22, 23, 25, 53, 80, 443, 3306, 5432, 5000]


def check_port(ip: str, port: int, timeout: float = 1.0) -> dict:
    """Vérifie si un port est ouvert sur une IP donnée"""
    if not isinstance(ip, str):
        raise TypeError(f"'ip' doit être une str, reçu {type(ip).__name__}")
    if not isinstance(port, int) or isinstance(port, bool):
        raise TypeError(f"'port' doit être un int, reçu {type(port).__name__}")

    match port:
        case p if p < 1 or p > 65535:
            raise ValueError(f"'port' doit être entre 1 et 65535, reçu {port}")

    try:
        socket.inet_aton(ip)
    except socket.error:
        raise ValueError(f"Adresse IP invalide : '{ip}'")

    result = {"ip": ip, "port": port, "status": "closed", "service": None}

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(timeout)
            if s.connect_ex((ip, port)) == 0:
                result["status"] = "open"
                try:
                    result["service"] = socket.getservbyport(port)
                except OSError:
                    result["service"] = "unknown"
    except socket.error as e:
        result["status"] = "error"
        result["error"] = str(e)

    return result


def scan_ports(ip: str, ports: list, output_file: str = "ports.json") -> list:
    """Scanne une liste de ports et écrit les résultats dans un fichier JSON."""
    if not isinstance(ip, str):
        raise TypeError(f"'ip' doit être une str, reçu {type(ip).__name__}")
    if not isinstance(ports, list):
        raise TypeError(f"'ports' doit être une list, reçu {type(ports).__name__}")
    if not ports:
        raise ValueError("La liste de ports est vide")

    results = []

    for port in ports:
        result = check_port(ip, port)
        results.append(result)

        if result["status"] == "open":
            service = result["service"] or "unknown"
            print(f"[OUVERT]  Port {port:5d}  ({service})")
        elif result["status"] == "closed":
            print(f"[FERMÉ]   Port {port:5d}")
        else:
            print(f"[ERREUR]  Port {port:5d}  ({result.get('error', '')})")

    with open(output_file, "w") as f:
        json.dump(results, f, indent=2, ensure_ascii=False)

    print(f"\nRésultats sauvegardés dans '{output_file}'")
    return results


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage : python {sys.argv[0]} <ip>")
        sys.exit(1)

    ip_address = sys.argv[1]
    scan_ports(ip_address, PORTS_TO_SCAN)
