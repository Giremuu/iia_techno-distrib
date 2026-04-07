# TP Scan de Ports en Python

Scanner de ports TCP en Python + tests unitaires PyTest.

## Fichiers

- `scanner.py` : logique principale
- `test_scanner.py` : tests unitaires
- `ports.json` : résultats générés à l'exécution
- `app.py` : création d'un serveur web en local via Flask
- `README.md` : documentation

## Utilisation

```bash
python3 scanner.py <ip>
```
Où l'IP est notre cible à tester

**Exemple :**
```bash
python3 scanner.py 127.0.0.1
```
Les résultats sont sauvegardés dans `ports.json`.

*Pour modifier la liste de ports à tester :*
- Modifier la variable "PORTS_TO_SCAN" dans `scanner.py`

## Tests

```bash
pytest test_scanner.py -v
```

## Ports scannés par défaut

| Port | Service       |
|------|---------------|
| 22   | SSH           |
| 23   | Telnet        |
| 25   | SMTP          |
| 53   | DNS           |
| 80   | HTTP          |
| 443  | HTTPS         |
| 3306 | MySQL         |
| 5432 | PostgreSQL    |

## Exemple de sortie `ports.json`

```json
[
  {
    "ip": "127.0.0.1",
    "port": 22,
    "status": "open",
    "service": "ssh"
  },
  {
    "ip": "127.0.0.1",
    "port": 80,
    "status": "closed",
    "service": null
  }
]
```
