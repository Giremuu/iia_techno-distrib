# iia_techno-distrib

Dépôt de travaux pratiques sur les **systèmes distribués et la programmation réseau** réalisés en école.

---

## Structure du dépôt

```mermaid
graph TD
    Root["iia_techno-distrib/"]

    Root --> A["TP_Connexion-Distante Sockets TCP en C"]
    Root --> B["TP-TCP-3WyasHandshakes Handshake TCP en C"]
    Root --> C["TP-TLS-Handshake Handshake TLS 1.2 en C"]
    Root --> D["TP_Scan-Port Scanner de ports Python"]
    Root --> E["TP-Websocket Chat temps réel Node.js"]
    Root --> F["TP-k8s Orchestration Kubernetes"]
    Root --> G["TP-Analyse-Wireshark-1 Analyse TCP — fenêtre glissante"]
    Root --> H["TP-Analyse-Wireshark-3 Capture Firefox — analyse TLS/TCP"]
    Root --> I["TP-algorithme Algorithmes distribués — théorie"]
    Root --> J["TCP/ UI Socket.IO — Node.js"]
```

---

## Progression pédagogique

```mermaid
flowchart LR
    subgraph Couche Réseau
        A["Sockets TCP (C)"]
        B["Handshake TCP (C)"]
        C["Handshake TLS 1.2 (C)"]
        A --> B --> C
    end

    subgraph Couche Application
        D["Scanner de ports (Python)"]
        E["Chat WebSocket (Node.js)"]
    end

    subgraph Systèmes Distribués
        F["Kubernetes (Minikube)"]
        I["Algorithmes (Lamport, Bully…)"]
    end

    subgraph Analyse
        G["Wireshark 1 (TCP / fenêtre)"]
        H["Wireshark 3 (TLS / TCP flags)"]
    end

    C --> D
    C --> E
    E --> F
    F --> I
    B --> G
    C --> H
```

---

## TPs en détail

### TP_Connexion-Distante — Sockets TCP (C)
Introduction à l'API POSIX des sockets. Un client et un serveur s'échangent des messages en TCP.

```bash
make && ./server &  ./client
```

---

### TP-TCP-3WyasHandshakes — Handshake TCP (C)
Simulation manuelle du three-way handshake : SYN → SYN-ACK → ACK avec génération des numéros de séquence.

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server
    C->>S: SYN (seq=x)
    S->>C: SYN-ACK (seq=y, ack=x+1)
    C->>S: ACK (ack=y+1)
    note over C,S: Connexion établie
```

> Requiert les droits root (raw sockets).

---

### TP-TLS-Handshake — Handshake TLS 1.2 (C)
Simulation d'une négociation TLS 1.2 complète : échange de clés, suite de chiffrement, dérivation du secret.

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server
    C->>S: ClientHello (cipher suites, random)
    S->>C: ServerHello (cipher choisi, random)
    S->>C: Certificate
    S->>C: ServerKeyExchange (ECDHE)
    C->>S: ClientKeyExchange (pre-master secret)
    C->>S: ChangeCipherSpec + Finished
    S->>C: ChangeCipherSpec + Finished
    note over C,S: Canal chiffré AES-256-GCM / SHA-384
```

---

### TP_Scan-Port — Scanner de ports (Python)
Scan des ports courants d'une cible, export JSON, suite de tests pytest incluse.

```bash
pip install -r requirements.txt
python scanner.py <ip>
```

Ports analysés : 22, 80, 443, 3306, 5432…

---

### TP-Websocket — Chat temps réel (Node.js)
Application de chat avec Socket.IO, clustering (2 workers), persistance SQLite et récupération des messages après reconnexion.

```mermaid
flowchart LR
    Browser -->|WebSocket| Worker1["Worker :3000 (Socket.IO)"]
    Browser -->|WebSocket| Worker2["Worker :3001 (Socket.IO)"]
    Worker1 --- DB[(SQLite)]
    Worker2 --- DB
```

```bash
npm install && node index.js
```

---

### TP-k8s — Orchestration Kubernetes (Minikube)
Déploiement d'un service nginx, mise à l'échelle, load balancing et auto-guérison des pods.

```mermaid
flowchart TD
    User -->|NodePort| SVC[Service Kubernetes]
    SVC --> P1[Pod nginx 1]
    SVC --> P2[Pod nginx 2]
    SVC --> P3[Pod nginx 3]
    P2 -.->|crash| Restart["Recréé automatiquement"]
```

```bash
kubectl apply -f deployment.yaml
kubectl apply -f service.yaml
kubectl scale deployment nginx-deployment --replicas=3
```

---

### TP-Analyse-Wireshark-1 & 3 — Analyse réseau
Exercices d'analyse de captures pcapng : fenêtre glissante TCP, calcul de débit, flags TCP, handshake TLS dans du trafic Firefox réel.

---

### TP-algorithme — Algorithmes distribués (théorie)
Exercices sur les concepts fondamentaux des systèmes distribués :

| Algorithme | Concept |
|---|---|
| Horloges de Lamport | Ordre causal des événements |
| Horloges vectorielles | Causalité distribuée |
| Algorithme de Bully | Élection de leader |
| Consensus | Accord malgré les pannes |
| Exclusion mutuelle | Accès aux ressources partagées |
| Quorum | Réplication et cohérence |
| Heartbeat | Détection de pannes |

---

## Technologies

| Langage / Outil | Usage |
|---|---|
| C | Simulation de protocoles réseau (TCP, TLS) |
| Python | Scanner de ports, tests unitaires (pytest) |
| Node.js / Socket.IO | Chat WebSocket temps réel |
| Kubernetes / Minikube | Orchestration de conteneurs |
| Wireshark | Analyse de captures réseau (pcapng) |
| SQLite | Persistance des messages |
| Docker | Conteneurs nginx pour k8s |
| Make | Compilation des projets C |