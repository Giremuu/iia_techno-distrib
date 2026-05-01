# iia_techno-distrib

![Version](https://img.shields.io/badge/version-V1-purple)
![License](https://img.shields.io/badge/license-MIT-yellow)
![Stack](https://img.shields.io/badge/stack-C%20%7C%20Python%20%7C%20Node.js%20%7C%20Kubernetes-lightgrey)

Practical labs on **distributed systems and network programming**, done at school. Covers TCP/TLS socket programming in C, port scanning in Python, real-time WebSocket chat in Node.js, Kubernetes orchestration, Wireshark traffic analysis, and distributed algorithm theory.

---

## Overview

```mermaid
graph TD
    Root["iia_techno-distrib/"]
    Root --> A["TP_Connexion-Distante - TCP sockets in C"]
    Root --> B["TP-TCP-3WyasHandshakes - TCP handshake in C"]
    Root --> C["TP-TLS-Handshake - TLS 1.2 handshake in C"]
    Root --> D["TP_Scan-Port - Port scanner in Python"]
    Root --> E["TP-Websocket - Real-time chat in Node.js"]
    Root --> F["TP-k8s - Kubernetes orchestration"]
    Root --> G["TP-Analyse-Wireshark-1 - TCP sliding window analysis"]
    Root --> H["TP-Analyse-Wireshark-3 - Firefox capture - TLS/TCP analysis"]
    Root --> I["TP-algorithme - Distributed algorithms - theory"]
    Root --> J["TCP/ - Socket.IO UI in Node.js"]
```

### Learning progression

```mermaid
flowchart LR
    subgraph Network Layer
        A["TCP Sockets (C)"]
        B["TCP Handshake (C)"]
        C["TLS 1.2 Handshake (C)"]
        A --> B --> C
    end
    subgraph Application Layer
        D["Port Scanner (Python)"]
        E["WebSocket Chat (Node.js)"]
    end
    subgraph Distributed Systems
        F["Kubernetes (Minikube)"]
        I["Algorithms (Lamport, Bully...)"]
    end
    subgraph Analysis
        G["Wireshark 1 (TCP / window)"]
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

## Usage

### TP_Connexion-Distante - TCP sockets (C)

Introduction to the POSIX socket API. A client and a server exchange messages over TCP.

```bash
make && ./server & ./client
```

---

### TP-TCP-3WyasHandshakes - TCP handshake (C)

Manual simulation of the three-way handshake: SYN - SYN-ACK - ACK with sequence number generation.

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server
    C->>S: SYN (seq=x)
    S->>C: SYN-ACK (seq=y, ack=x+1)
    C->>S: ACK (ack=y+1)
    note over C,S: Connection established
```

> Requires root privileges (raw sockets).

---

### TP-TLS-Handshake - TLS 1.2 handshake (C)

Full TLS 1.2 negotiation simulation: key exchange, cipher suite selection, secret derivation.

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server
    C->>S: ClientHello (cipher suites, random)
    S->>C: ServerHello (cipher chosen, random)
    S->>C: Certificate
    S->>C: ServerKeyExchange (ECDHE)
    C->>S: ClientKeyExchange (pre-master secret)
    C->>S: ChangeCipherSpec + Finished
    S->>C: ChangeCipherSpec + Finished
    note over C,S: Encrypted channel AES-256-GCM / SHA-384
```

---

### TP_Scan-Port - Port scanner (Python)

Scans common ports of a target, exports JSON results, includes a pytest test suite.

```bash
pip install -r requirements.txt
python scanner.py <ip>
```

Ports scanned: 22, 80, 443, 3306, 5432...

---

### TP-Websocket - Real-time chat (Node.js)

Chat application with Socket.IO, clustering (2 workers), SQLite persistence and message recovery after reconnection.

```mermaid
flowchart LR
    Browser -->|WebSocket| W1["Worker :3000 (Socket.IO)"]
    Browser -->|WebSocket| W2["Worker :3001 (Socket.IO)"]
    W1 --- DB[(SQLite)]
    W2 --- DB
```

```bash
npm install && node index.js
```

---

### TP-k8s - Kubernetes orchestration (Minikube)

Deploy an nginx service, scale it, configure load balancing and pod self-healing.

```mermaid
flowchart TD
    User -->|NodePort| SVC[Kubernetes Service]
    SVC --> P1[nginx Pod 1]
    SVC --> P2[nginx Pod 2]
    SVC --> P3[nginx Pod 3]
    P2 -.->|crash| R["Automatically recreated"]
```

```bash
kubectl apply -f deployment.yaml
kubectl apply -f service.yaml
kubectl scale deployment nginx-deployment --replicas=3
```

---

## Specificities

### TP-Analyse-Wireshark-1 and 3 - Network analysis

pcapng capture analysis exercises: TCP sliding window, throughput calculation, TCP flags, TLS handshake in real Firefox traffic.

---

### TP-algorithme - Distributed algorithms (theory)

| Algorithm | Concept |
|---|---|
| Lamport clocks | Causal ordering of events |
| Vector clocks | Distributed causality |
| Bully algorithm | Leader election |
| Consensus | Agreement despite failures |
| Mutual exclusion | Shared resource access |
| Quorum | Replication and consistency |
| Heartbeat | Failure detection |

---

### Technologies

| Language / Tool | Usage |
|---|---|
| C | Network protocol simulation (TCP, TLS) |
| Python | Port scanner, unit tests (pytest) |
| Node.js / Socket.IO | Real-time WebSocket chat |
| Kubernetes / Minikube | Container orchestration |
| Wireshark | Network capture analysis (pcapng) |
| SQLite | Message persistence |
| Docker | nginx containers for k8s |
| Make | C project compilation |

---

## License

MIT
