# WebSocket Chat

Application de chat en temps réel construite avec **Socket.IO**, **Express** et **SQLite**, démontrant plusieurs fonctionnalités avancées : persistance des messages, récupération d'état après déconnexion, et mise à l'échelle par clustering Node.js.

## Fonctionnalités

- **Chat en temps réel** via WebSocket (Socket.IO)
- **Pseudonymes** : chaque utilisateur choisit un pseudo à la connexion
- **Persistance des messages** dans une base SQLite
- **Récupération d'état** : les messages manqués sont renvoyés automatiquement à la reconnexion
- **Déduplication** : les messages en double sont ignorés grâce à un `client_offset` unique
- **Clustering** : 2 workers Node.js sur les ports 3000 et 3001

## Prérequis

- [Node.js](https://nodejs.org/) v18+

## Installation

```bash
npm install
```

## Lancement

```bash
npm start
```

L'application démarre deux workers accessibles sur :
- http://localhost:3000
- http://localhost:3001

## Architecture

```
index.js      Serveur Express + Socket.IO avec clustering
index.html    Interface client (HTML/CSS/JS vanilla)
chat.db       Base de données SQLite (créée automatiquement)
```

### Serveur (`index.js`)

Le processus principal fork 2 workers via `node:cluster`. Chaque worker :

1. Ouvre la base SQLite et crée la table `messages` si elle n'existe pas (`id`, `content`, `username`, `client_offset`)
2. Démarre un serveur Express qui sert `index.html`
3. Écoute l'événement `chat message` :
   - Insère le message en base avec son `client_offset` (contrainte `UNIQUE` pour éviter les doublons) et le pseudo de l'auteur
   - Broadcast le message et le pseudo à tous les clients connectés via `io.emit()`
4. À la connexion d'un client non récupéré (`socket.recovered === false`), renvoie tous les messages (avec pseudos) depuis son dernier `serverOffset` connu

Le [cluster adapter](https://socket.io/docs/v4/cluster-adapter/) synchronise les événements entre les deux workers.

### Client (`index.html`)

- Demande un pseudo via `prompt()` au chargement, transmis dans `socket.auth.username`
- Se connecte avec `ackTimeout: 10000` et `retries: 3` pour garantir la livraison des messages
- Génère un `clientOffset` unique par message (`socketId-counter`) pour la déduplication côté serveur
- Affiche chaque message sous la forme `Pseudo : message`
- Met à jour son `serverOffset` à chaque message reçu pour permettre la resynchronisation après reconnexion

## Dépendances

| Package | Rôle |
|---|---|
| `express` | Serveur HTTP |
| `socket.io` | Communication WebSocket temps réel |
| `@socket.io/cluster-adapter` | Partage d'état Socket.IO entre workers |
| `sqlite3` + `sqlite` | Base de données SQLite (interface Promise) |


### Infos :
- Le style CSS a été fait avec IA