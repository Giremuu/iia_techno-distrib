# Rendu TP Analyse Wireshark 1

## Partie 1 — Observation du sliding window dans Wireshark

**Données :**
- MSS - Taille max d'un segment = 1460 octets
- Fenêtre = 8760 octets
- RTT = 40 ms

### Q1. Combien de segments TCP peuvent être envoyés sans attendre d'acquittement ?

$$\frac{8760}{1460} = \boxed{6 \text{ segments}}$$

### Q2. Combien d'octets peuvent être envoyés sans attendre d'ACK ?

La fenêtre TCP définit directement le nombre d'octets pouvant être envoyé sans attendre un ACK :

$$\boxed{8760 \text{ octets}}$$

### Q3. Combien d'acquittements sont nécessaires pour transmettre 43 800 octets ?

Nombre total de segments :

$$\frac{43800}{1460} = 30 \text{ segments}$$

Nombre de fenêtres nécessaires :

$$\frac{30}{6} = \boxed{5 \text{ ACK}}$$

Soit 5 ACK seront nécessaires pour transmettre les 43 800 octets répartis dans 30 segments.

### Q4. Quelle est la relation entre la taille de la fenêtre et le nombre de segments envoyés ?

La fenêtre TCP fixe le volume max de données en transit non acquittées. Plus elle est grande relativement au MSS, plus l'émetteur peut envoyer de segments consécutifs.

Soit :
$$\text{Segments} = \left\lfloor \frac{\text{Window Size}}{\text{MSS}} \right\rfloor$$


## Partie 2 — Numéros de séquence

**Séquences observées :**
- 1000
- 2460
- 3920
- 5380
- 6840

### Q1. Quelle est la taille d'un segment TCP dans cet exemple ?

$$2460 - 1000 = \boxed{1460 \text{ octets}}$$

### Q2. Combien d'octets sont transmis entre le premier et le dernier segment ?

$$6840 - 1000 = \boxed{5840 \text{ octets}}$$

### Q3. Quel sera le prochain numéro de séquence attendu ?

$$6840 + 1460 = \boxed{8300}$$

### Q4. Que représente le numéro de séquence dans TCP ?

Le numéro de séquence identifie le premier octet du segment dans le flux de données. Il permet au récepteur de réordonner les segments arrivés dans le désordre et de détecter les doublons ou les pertes.


## Partie 3 — Calcul du débit théorique

**Données :**
- fenêtre = 64 KB
- RTT = 50 ms

### Q1. Convertir la taille de fenêtre en octets

$$64 \text{ KB} = 64 \times 1024 = \boxed{65536 \text{ octets}}$$

### Q2. Convertir le RTT en secondes

$$50 \text{ ms} = \frac{50}{1000} = \boxed{0{,}05 \text{ s}}$$

### Q3. Débit maximal théorique en octets/s

$$Débit = \frac{65536}{0{,}05} = \boxed{1\,310\,720 \text{ o/s}}$$

### Q4. Convertir en Ko/s

$$\frac{1\,310\,720}{1024} = \boxed{1280 \text{ Ko/s}}$$

### Q5. Convertir en Mb/s

$$\frac{1\,310\,720 \times 8}{1\,000\,000} = \boxed{\approx 10{,}49 \text{ Mb/s}}$$



## Partie 4 — Influence du RTT

**Paramètre fixe :**
- Fenêtre = 65 535 octets

$$Débit = \frac{65535}{RTT}$$

### Q1. RTT = 20 ms

$$\frac{65535}{0{,}02} = \boxed{3\,276\,750 \text{ o/s} \approx 26{,}2 \text{ Mb/s}}$$

### Q2. RTT = 100 ms

$$\frac{65535}{0{,}1} = \boxed{655\,350 \text{ o/s} \approx 5{,}24 \text{ Mb/s}}$$

### Q3. RTT = 200 ms

$$\frac{65535}{0{,}2} = \boxed{327\,675 \text{ o/s} \approx 2{,}62 \text{ Mb/s}}$$

### Q4. Conclusion sur l'influence du RTT

| RTT | Débit théorique |
|-----|----------------|
| 20 ms | ~26,2 Mb/s |
| 100 ms | ~5,24 Mb/s |
| 200 ms | ~2,62 Mb/s |

Un RTT élevé dégrade fortement les performances TCP et témoigne d'une congestion réseau.


## Partie 5 — Fenêtre glissante

**Données :**
- MSS = 1000 octets
- Fenêtre = 5000 octets

### Q1. Combien de segments peuvent être envoyés avant réception d'un ACK ?

$$\frac{5000}{1000} = \boxed{5 \text{ segments}}$$

### Q2. Numéros de séquence envoyés (premier = 0)

```
Seq = 0
Seq = 1000
Seq = 2000
Seq = 3000
Seq = 4000
```

### Q3. Numéro d'acquittement après réception complète

L'ACK indique le prochain octet attendu :

$$0 + 5000 = \boxed{ACK = 5000}$$

### Q4. Que devient la fenêtre après réception de l'ACK ?

La fenêtre glisse : elle avance du nombre d'octets acquittés et permet d'envoyer les 5 segments suivants (Seq 5000 à 9000).



## Partie 6 — Analyse d'une capture Wireshark

**Capture :**
```
Seq = 0
Seq = 1460
Seq = 2920
Seq = 4380

Ack = 5840
Window Size = 5840
```

### Q1. Combien de segments ont été envoyés ?

4 segments (Seq 0, 1460, 2920, 4380).

### Q2. Combien d'octets ont été transmis ?

$$4 \times 1460 = \boxed{5840 \text{ octets}}$$

### Q3. Pourquoi l'ACK indique 5840 ?

L'ACK indique le prochain octet attendu. Après réception des 4 segments (0 à 4379 + le dernier segment de 1460 o), le récepteur attend l'octet numéro 5840.

$$ACK = 4380 + 1460 = \boxed{5840}$$

### Q4. Quelle est la taille de la fenêtre annoncée par le récepteur ?

$$\boxed{5840 \text{ octets}}$$

C'est exactement la taille des données reçues, le récepteur indique qu'il est prêt à en recevoir autant de plus.

### Q5. Que signifie une diminution de la taille de la fenêtre ?

Le récepteur manque de mémoire tampon > Il signale à l'émetteur de ralentir l'envoi > Si la fenêtre atteint 0, l'émetteur doit s'arrêter et attendre un Window Update du récepteur.

Cela peut indiquer une saturation applicative ou système côté récepteur.


## Partie 7 — Débit réel

**Données :**
- Fenêtre = 12 000 octets
- RTT = 60 ms

### Q1. Débit maximal théorique

$$\frac{12000}{0{,}06} = \boxed{200\,000 \text{ o/s} = 200 \text{ Ko/s} \approx 1{,}6 \text{ Mb/s}}$$

### Q2. Débit réel (8000 o transmis par RTT)

$$\frac{8000}{0{,}06} \approx \boxed{133\,333 \text{ o/s} \approx 130 \text{ Ko/s} \approx 1{,}07 \text{ Mb/s}}$$

### Q3. Comparaison

| | Débit |
|---|---|
| Théorique | 200 Ko/s |
| Réel | ~133 Ko/s |
| Écart | ~33% de perte |

### Q4. Explication possible à la différence

Plusieurs causes peuvent expliquer cet écart :
- Congestion réseau : TCP réduit sa fenêtre d'envoi via le contrôle de congestion
- Pertes de paquets et retransmissions qui occupent de la bande passante
- Limitation applicative : l'application ne fournit pas les données assez vite
- Acquittements retardés qui ralentissent la progression de la fenêtre
- Limitations matérielles du récepteur

---

## Partie 8 — Synthèse

### Q1. Rôle principal du sliding window

Permettre l'envoi de plusieurs segments sans attendre un ACK par segment. Sans ce mécanisme, TCP serait en attente d'ACK après chaque segment.

### Q2. Pourquoi TCP n'envoie-t-il pas les données une par une ?

Envoyer un segment et attendre son ACK avant d'envoyer le suivant implique un temps d'attente d'un RTT complet entre chaque segment. Avec un RTT de 50 ms et des segments de 1460 o, le débit serait limité à `1460 / 0,05 = 29,2 Ko/s` au lieu de plusieurs Mo/s.

### Q3. Impact d'une petite fenêtre TCP

- Moins de segments en transit en même temps
- Le débit est réduit
- La latence applicative augmente

### Q4. Impact d'un RTT élevé

- Le débit maximal (`fenêtre / RTT`) diminue
- L'émetteur doit attendre plus longtemps avant de recevoir les ACK et de faire avancer la fenêtre
- Les performances se dégradent même si la fenêtre est grande
- Les liens longue distance sont affectés

### Q5. Pourquoi le sliding window améliore-t-il les performances ?

Le sliding window permet de maintenir plusieurs segments en transit simultanément. L'émetteur n'attend pas l'ACK de chaque segment pour envoyer le suivant : dès qu'un ACK est reçu, la fenêtre glisse et de nouveaux segments peuvent partir.