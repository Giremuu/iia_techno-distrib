# TP Connexion Distante — Sockets TCP en C

Deux programmes C illustrant les bases de la programmation réseau avec les sockets POSIX

## Fichiers

| Fichier | Rôle |
|---------|------|
| `server.c` | Attend une connexion entrante et affiche les ordres reçus |
| `client.c` | Se connecte à un serveur distant et lui envoie des messages |
| `Makefile` | Compilation automatique |

## Compilation

```bash
make
```

Produit deux exécutables : `server` et `client`.

```bash
make clean   # supprime les exécutables
```

## Utilisation

### 1. Lancer le serveur

```bash
./server           # écoute sur le port 4444 par défaut
./server 8080      # écoute sur un port personnalisé
```

Le serveur attend une connexion, puis affiche chaque ordre reçu dans le terminal.

### 2. Lancer le client

```bash
./client <adresse_ip> [port]
```

```bash
./client 127.0.0.1          # connexion en local, port 4444
./client 192.168.1.10 8080  # connexion distante, port 8080
```

Tape les ordres à envoyer, puis **Ctrl+D** pour terminer.

## Exemple de test en local

**Terminal 1 — Serveur :**
```
$ ./server
[*] Serveur en écoute sur le port 4444...
[+] Connexion reçue de 127.0.0.1:52341
[ordre] bonjour
[ordre] ls -la
[-] Client déconnecté.
```

**Terminal 2 — Client :**
```
$ ./client 127.0.0.1
[*] Connexion vers 127.0.0.1:4444...
[+] Connecté à 127.0.0.1:4444
Entrez les ordres à envoyer (Ctrl+D pour quitter) :
bonjour
ls -la
^D
[-] Déconnexion.
```

## Fonctions socket utilisées

| Fonction | Rôle |
|----------|------|
| `socket()` | Crée le socket TCP |
| `bind()` | Attache le socket à une IP/port |
| `listen()` | Met le serveur en attente de connexions |
| `accept()` | Accepte une connexion entrante |
| `connect()` | Établit la connexion côté client |
| `send()` | Envoie des données |
| `recv()` | Reçoit des données |
