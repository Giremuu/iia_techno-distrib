# Rendu - TP Algorithmes fondamentaux des systèmes distribués

## Exercice 1 : Horloges logiques de Lamport

**Règles de Lamport :**
- Événement local : `H = H + 1`
- Envoi de message : `H = H + 1`, le message porte la valeur `H`
- Réception de message : `H = max(H_local, H_reçu) + 1`

| Événement | Processus | Action | Horloge |
|-----------|-----------|--------|---------|
| 1 | P1 | Événement local | H(P1) = 0 + 1 = **1** |
| 2 | P1 | Envoie message à P2 (porte H=2) | H(P1) = 1 + 1 = **2** |
| 3 | P2 | Reçoit le message (H_reçu=2, H_local=0) | H(P2) = max(0, 2) + 1 = **3** |
| 4 | P2 | Envoie message à P3 (porte H=4) | H(P2) = 3 + 1 = **4** |
| 5 | P3 | Reçoit le message (H_reçu=4, H_local=0) | H(P3) = max(0, 4) + 1 = **5** |

---

## Exercice 2 : Horloges vectorielles

**Règles :**
- Événement local sur Pi : incrémenter V[i]
- Envoi sur Pi : incrémenter V[i], envoyer le vecteur
- Réception sur Pi depuis Pj : V[k] = max(V[k], V_reçu[k]) pour tout k, puis V[i]++

Vecteur initial : **(0, 0, 0)** — indices [P1, P2, P3]

| Événement | Processus | Action | Vecteur résultant |
|-----------|-----------|--------|-------------------|
| 1 | P1 | Événement local | **(1, 0, 0)** |
| 2 | P1 | Envoie message à P2 | **(2, 0, 0)** → message porte (2, 0, 0) |
| 3 | P2 | Reçoit le message de P1 | max((0,0,0),(2,0,0)) + incrément P2 → **(2, 1, 0)** |
| 4 | P2 | Événement local | **(2, 2, 0)** |

---

## Exercice 3 : Algorithme d'élection du leader (Bully)

**Situation :** P4 (leader, ID=4) tombe en panne. P2 (ID=2) détecte la panne.

**1. À quels processus P2 envoie-t-il un message ?**

P2 envoie un message `ELECTION` à tous les processus avec un **identifiant supérieur** au sien :
- **P3** (ID=3) et **P4** (ID=4)

**2. Quel processus devient le nouveau leader ?**

- P3 reçoit le message d'élection de P2 → répond `OK` à P2 et lance à son tour une élection vers P4
- P4 ne répond pas (en panne)
- P3, n'ayant reçu aucune réponse de P4, se déclare coordinateur et envoie un message `COORDINATOR` à P1 et P2

**Le nouveau leader est P3** (ID=3).

**3. Pourquoi cet algorithme s'appelle-t-il "Bully" ?**

Car le processus ayant le **plus grand identifiant** impose son autorité ("bully" = intimidateur) sur tous les autres. Dès qu'un processus plus prioritaire répond à une élection, il prend le dessus et force les autres à s'incliner. Le processus dominant finit toujours par "écraser" les candidats inférieurs.

---

## Exercice 4 : Consensus distribué

| Serveur | Valeur proposée |
|---------|-----------------|
| S1 | 10 |
| S2 | 10 |
| S3 | 20 |

**1. Quelle valeur est choisie ?**

La valeur **10** est choisie : elle obtient 2 votes sur 3 (S1 et S2), ce qui constitue la **majorité** (> N/2 = 1,5).

**2. Que se passe-t-il si S2 tombe en panne ?**

Il ne reste que S1 (valeur 10) et S3 (valeur 20). Chaque valeur n'obtient qu'**1 vote**, sans majorité possible. Le système **ne peut pas atteindre le consensus** et reste bloqué jusqu'à la reprise de S2 ou l'intervention d'un mécanisme de timeout/reprise.

**3. Combien de serveurs minimum sont nécessaires pour tolérer une panne ?**

Pour tolérer **f pannes**, il faut au minimum **2f + 1** serveurs (afin de toujours disposer d'une majorité fonctionnelle).

Pour tolérer **1 panne** → minimum **3 serveurs** (2×1+1 = 3).

---

## Exercice 5 : Exclusion mutuelle distribuée

**Règle :** le processus ayant le **timestamp le plus faible** accède en premier à la ressource (priorité inverse du timestamp).

| Processus | Timestamp |
|-----------|-----------|
| P2 | 2 |
| P1 | 5 |
| P3 | 8 |

**Ordre d'accès à la ressource :**

1. **P2** (timestamp = 2)
2. **P1** (timestamp = 5)
3. **P3** (timestamp = 8)

---

## Exercice 6 : Réplication et quorum

**Paramètres :** 5 serveurs, quorum lecture = 2, quorum écriture = 3

**1. Peut-on lire si 3 serveurs sont en panne ?**

Serveurs disponibles : 5 − 3 = **2**. Le quorum de lecture exige 2 réponses. **Oui**, on peut tout juste lire (avec exactement les 2 serveurs restants).

**2. Peut-on écrire si 2 serveurs sont en panne ?**

Serveurs disponibles : 5 − 2 = **3**. Le quorum d'écriture exige 3 confirmations. **Oui**, on peut écrire (avec exactement les 3 serveurs restants).

**3. Pourquoi l'écriture nécessite-t-elle plus de confirmations que la lecture ?**

Pour garantir la **cohérence des données** : en exigeant davantage de serveurs pour valider une écriture, on s'assure qu'une majorité de nœuds possède la version la plus récente. Ainsi, toute opération de lecture ultérieure a de fortes chances de contacter au moins un serveur à jour. L'écriture est l'opération qui modifie l'état du système ; multiplier les confirmations réduit le risque de perte de données et de lecture de données obsolètes.

---

## Exercice 7 : Détection de panne par heartbeat

| Temps | Message |
|-------|---------|
| 0 s | alive |
| 2 s | alive |
| 4 s | alive |
| 6 s | — |
| 8 s | — |

**Analyse :**

- Le dernier message `alive` est reçu à **4 s**.
- Le prochain heartbeat attendu arrive à **6 s** → rien reçu : **premier heartbeat manqué**.
- À **8 s** → rien reçu : **deuxième heartbeat manqué consécutif**.

**Réponse :** On peut suspecter une panne dès 6s (après le premier heartbeat manqué, soit 2s après le dernier reçu), j'aurais tendance à attendre un second heartbeat manqué consécutivement pour déclencher une panne pour réduire les faux positifs.
