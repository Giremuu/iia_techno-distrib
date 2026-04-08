# TP - Analyse approfondie du protocole TCP

## Partie 1 - Capture d'un trafic TCP significatif

### Réponses

**Q1 - Interface réseau utilisée :**  
L'interface utilisée pour la capture est **`wlo1`** (interface Wi-Fi sous mon Linux), identifiée dans les métadonnées du fichier pcapng. L'encapsulation est de type Ethernet.

**Q2 - Application ayant généré le trafic TCP :**  
Le trafic a été généré principalement par Mozilla Firefox (version 140.0 sous Linux x86_64), identifiable dans le champ `User-Agent` des requêtes HTTP. Firefox effectue notamment des requêtes de détection de portail captif vers `detectportal.firefox.com`, ainsi que des connexions HTTPS vers des serveurs Microsoft, GitHub et Cloudflare.

**Q3 - Durée totale de la capture :**  
La capture s'étend sur 343,5 secondes, soit 5 minutes 43 secondes.

**Q4 - Nombre de paquets capturés :**  
**3 054 paquets** ont été capturés au total, pour un volume de données de **765 KB**

---

## Partie 2 - Identification d'une session TCP complète


### Session analysée

La session retenue pour l'analyse est la connexion **HTTP en clair** établie entre le client et le serveur `detectportal.firefox.com`. Elle est choisie car elle est non chiffrée (port 80), ce qui permet d'observer le contenu applicatif, et elle présente un cycle complet : handshake, transfert de données, keep-alive, fermeture propre.

**Q1 - Adresse IP du client :** `10.111.17.4` (IP de l'école en /22)

**Q2 - Adresse IP du serveur :** `34.107.221.82`

**Q3 - Port source (client) :** `51486`

**Q4 - Port de destination (serveur) :** `80`

**Q5 - Service applicatif correspondant :**  
Le port 80 correspond au protocole **HTTP**, protocole applicatif de transfert de pages web en clair. La requête capturée est effectivement une requête HTTP/1.1 à destination de `detectportal.firefox.com`.

---

## Partie 3 - Analyse de l'établissement de la connexion

### Réponses

**Q1 - Les trois paquets du three-way handshake :**

| Étape | Heure (relative) | Source | Destination | Description |
|-------|-----------------|--------|-------------|-------------|
| SYN | 120.689 s | `10.111.17.4:51486` | `34.107.221.82:80` | Le client initie la connexion |
| SYN-ACK | 120.705 s | `34.107.221.82:80` | `10.111.17.4:51486` | Le serveur accepte et synchronise |
| ACK | 120.705 s | `10.111.17.4:51486` | `34.107.221.82:80` | Le client confirme la connexion |

**Q2 - Drapeaux TCP activés dans chaque paquet :**

- **SYN** : flag `SYN` uniquement - demande d'ouverture de connexion, le client propose son ISN.
- **SYN-ACK** : flags `SYN` + `ACK` - le serveur accepte la connexion, propose son propre ISN et acquitte le SYN client.
- **ACK** : flag `ACK` uniquement - le client confirme la réception du SYN-ACK du serveur. La connexion est établie.

**Q3 - Numéro de séquence initial du client :**  
`3 449 354 677` (valeur brute). Wireshark affiche `Seq=0` en numérotation relative pour faciliter la lecture.

**Q4 - Numéro d'acquittement retourné par le serveur (dans le SYN-ACK) :**  
`Ack=1` (relatif), soit ISN_client + 1 = `3 449 354 678`. Le serveur acquitte le SYN du client (qui compte pour 1 octet logique), et propose son propre ISN brut : `2 201 517 685`.

**Q5 - Le handshake est-il complet et correct ?**  
Oui. Les trois paquets sont présents et cohérents : le serveur acquitte correctement l'ISN du client (ISN+1), le client acquitte ensuite l'ISN du serveur (ISN_serveur+1). Les options négociées (MSS=1460 côté client, MSS=1354 côté serveur, SACK, Timestamps, Window Scale) sont cohérentes avec un échange Ethernet standard. Le handshake est valide.

---

## Partie 4 - Étude des numéros de séquence et d'acquittement

### Réponses

**Q1 - Évolution des numéros de séquence :**
Les numéros de séquence progressent en fonction de la quantité de données envoyées. Lors du handshake, chaque SYN consomme 1 unité (logique). Ensuite, lorsque le client envoie sa requête HTTP GET (326 octets de données), son numéro de séquence passe de 1 à 327. Côté serveur, la réponse HTTP (298 octets de données) fait passer son Seq de 1 à 299.

**Q2 - Évolution des numéros d'acquittement :**  
Chaque ACK émis confirme la réception de tous les octets reçus jusqu'alors. Après réception de la requête GET du client (326 octets), le serveur envoie `Ack=327`. Après réception de la réponse HTTP (298 octets), le client envoie `Ack=299`.

**Q3 - Cohérence de la progression des acquittements :**  
Oui, la progression est parfaitement cohérente. Les acquittements correspondent exactement aux données reçues : chaque ACK vaut le Seq précédent + la longueur (Len) du segment reçu.

**Q4 - Trois paquets successifs avec Seq et Ack :**

| N° paquet | Source | Seq (relatif) | Ack (relatif) | Len | Description |
|-----------|--------|--------------|--------------|-----|-------------|
| Paquet 3 (ACK final handshake) | Client | 1 | 1 | 0 | Fin du handshake |
| Paquet 4 (GET HTTP) | Client | 1 | 1 | 326 | Requête GET /canonical.html |
| Paquet 5 (ACK serveur) | Serveur | 1 | 327 | 0 | Serveur acquitte la requête |

---

## Partie 5 - Analyse des drapeaux TCP

### Réponses

**Q1 - Drapeaux les plus fréquents :**  
Le drapeau **ACK** est de très loin le plus fréquent, présent dans quasi tous les paquets à partir du troisième échange. Il est souvent seul (simple acquittement), ou combiné avec PSH (données applicatives urgentes) ou FIN (fermeture).

**Q2 - Exemple de paquet contenant ACK :**  
Paquet à t=120.705 s : `10.111.17.4 → 34.107.221.82` - `[ACK] Seq=1 Ack=1 Win=64256 Len=0` - troisième étape du handshake, pure confirmation de connexion sans données.

**Q3 - Exemple de paquet contenant PSH :**  
Paquet à t=120.705 s : `10.111.17.4 → 34.107.221.82` - `[PSH, ACK] Seq=1 Ack=1 Len=326` - requête HTTP GET. Le flag `PSH` indique que les données doivent être transmises immédiatement à la couche applicative sans attendre de buffering supplémentaire.

**Q4 - Exemple de paquet FIN :**  
Paquet à t=237.621 s : `10.111.17.4 → 34.107.221.82` - `[FIN, ACK] Seq=327 Ack=299 Win=64128 Len=0` - le client initie la fermeture de la session HTTP après la phase de keep-alive.

**Q5 - Rôle de ces drapeaux dans la communication :**  
Ces drapeaux orchestrent l'intégralité du cycle de vie d'une connexion TCP. `SYN` et `SYN-ACK` établissent la connexion et synchronisent les ISN. `ACK` assure la fiabilité en confirmant la réception de chaque segment. `PSH` optimise la latence applicative en forçant la remontée immédiate des données. `FIN` permet une fermeture propre et bilatérale, garantissant qu'aucune donnée n'est perdue.

---

## Partie 6 - Étude de la fenêtre TCP

### Réponses

**Q1 - Valeur initiale de la fenêtre TCP :**  
Dans le SYN du client : **64 240 octets** (valeur brute, après scaling = 64 240 × 128 = ~8 MB de fenêtre maximale potentielle). Dans le SYN-ACK du serveur : **65 535 octets** (valeur brute).

**Q2 - La taille de fenêtre reste-t-elle constante ?**  
Non. La fenêtre évolue dynamiquement au fil de la communication, reflétant l'état du buffer de réception de chaque hôte.

**Q3 - Trois valeurs différentes observées :**

| Moment | Valeur de fenêtre | Côté |
|--------|------------------|------|
| SYN (début) | 64 240 octets | Client |
| Après réception de la réponse HTTP | 64 000 octets | Client |
| Pendant keep-alive | 64 128 octets | Client |
| Côté serveur (stable) | 268 800 octets | Serveur (34.107.221.82) |

**Q4 - Lien entre fenêtre TCP et réception des données :**  
La fenêtre TCP est l'outil principal du contrôle de flux : elle empêche l'émetteur d'inonder le récepteur. Après réception d'un segment de données, la fenêtre diminue du nombre d'octets reçus non encore traités par l'application. Elle se rétablit lorsque l'application lit les données dans son buffer. Dans cette session, la fenêtre reste élevée car le débit est faible, ce qui confirme l'absence de congestion.

---

## Partie 7 - Recherche de retransmissions et d'anomalies

### Réponses

**Q1 - Présence de retransmissions :**  
Oui, des retransmissions sont présentes dans la capture, mais sur des sessions HTTPS (chiffrées), pas sur la session HTTP analysée.

**Q2 - Nombre de retransmissions détectées :**  
Wireshark en détecte **6 au total** sur l'ensemble de la capture :

| N° | Heure | Session concernée | Type |
|----|-------|------------------|------|
| 1 | 120.656 s | `195.154.179.210 → 10.111.17.4` (TLS) | TCP Spurious Retransmission |
| 2 | 120.980 s | `104.18.22.112 → 10.111.17.4` (HTTPS) | TCP Retransmission |
| 3 | 134.635 s | `140.82.112.26 → 10.111.17.4` (GitHub HTTPS) | TCP Retransmission |
| 4 | 134.952 s | `10.111.17.4 → 140.82.112.26` (GitHub HTTPS) | TCP Retransmission |
| 5 | 135.472 s | `140.82.112.26 → 10.111.17.4` (GitHub HTTPS) | TCP Retransmission |
| 6 | 207.968 s | `10.111.17.4 → 13.107.226.42` (Microsoft HTTPS) | TCP Retransmission |

**Q3 - ACK dupliqués présents :**  
Oui, **9 ACK dupliqués** ont été détectés sur différentes sessions, notamment vers GitHub (140.82.112.26), Cloudflare (104.18.22.112), et d'autres serveurs HTTPS.

**Q4 - Exemple précis d'anomalie :**  
À t=120.956 s, sur la session `10.111.17.4:46750 → 104.18.22.112:443` (HTTPS Cloudflare) : Wireshark signale un **TCP Dup ACK** avec `Ack=2687` et une option SACK indiquant `SLE=3095, SRE=3126`. Cela signifie que le client a reçu des données après le byte 3095 mais pas le segment entre 2687 et 3095, révélant un paquet manquant en transit. Une retransmission suit à t=120.980 s.

**Q5 - Conséquences sur les performances :**  
Une retransmission implique un délai supplémentaire égal au RTT (Round-Trip Time), et une réduction de la fenêtre de congestion (mécanisme slow start ou fast recovery). Dans le cas d'une *Spurious Retransmission* (retransmission inutile d'un segment déjà reçu), elle consomme inutilement de la bande passante. Ces anomalies restent ici ponctuelles et n'affectent pas significativement la qualité globale de la communication.

---

## Partie 8 - Analyse graphique du flux TCP

### Réponses

**Q1 - Régularité du flux TCP :**  
Sur la session HTTP analysée (port 80, 34.107.221.82), le flux est **simple et très régulier** : un unique aller-retour applicatif (requête GET + réponse), suivi d'une longue phase de keep-alive avec des échanges périodiques toutes les 10 secondes environ, puis une fermeture propre. Le graphe Time-Sequence serait quasi plat après le premier échange.

**Q2 - Ruptures, ralentissements ou paliers :**  
Le flux HTTP ne présente aucune rupture. La phase de keep-alive représente un long plateau dans le graphe (aucune nouvelle donnée échangée pendant ~117 secondes), ce qui est attendu et non problématique. Sur les sessions HTTPS les plus actives (Microsoft, GitHub), des micro-paliers sont visibles lors des retransmissions identifiées en Partie 7.

**Q3 - Stabilité du RTT :**  
Le RTT mesuré sur la session HTTP est stable et faible : environ **15 ms** entre l'envoi du SYN (t=120.689 s) et la réception du SYN-ACK (t=120.705 s), et environ **14 ms** entre l'envoi du GET et la réception de l'ACK serveur. Ce RTT constant confirme une connexion réseau stable vers ce serveur.

**Q4 - Conclusions de l'analyse graphique :**  
La session HTTP présente un comportement idéal : flux simple, RTT constant, aucune anomalie. Les sessions HTTPS présentent davantage d'activité et quelques irrégularités (retransmissions, duplicate ACKs), mais celles-ci restent marginales et témoignent de conditions réseau globalement saines avec quelques pertes ponctuelles typiques d'un réseau Wi-Fi en environnement dense.

---

## Partie 9 - Reconstruction du flux applicatif

### Réponses

**Q1 - Protocole applicatif transporté :**  
La session TCP analysée transporte du **HTTP/1.1** (*HyperText Transfer Protocol version 1.1*), protocole applicatif non chiffré de transfert de contenu web.

**Q2 - Requête applicative identifiable :**  
Oui. Le client envoie :
```
GET /canonical.html HTTP/1.1
Host: detectportal.firefox.com
User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:140.0) Gecko/20100101 Firefox/140.0
```
Il s'agit d'une requête de détection de portail captif effectuée automatiquement par Firefox pour vérifier la connectivité internet.

**Q3 - Réponse du serveur identifiable :**  
Oui. Le serveur répond :
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 90
```
Suivi du contenu HTML. Le code `200 OK` confirme que la ressource existe et est accessible.

**Q4 - Lisibilité du contenu applicatif :**  
Le contenu est **intégralement lisible**. Étant en HTTP clair (non chiffré), la totalité des échanges - en-têtes et corps de la réponse - est visible en clair dans Wireshark via la fonction "Follow TCP Stream".

**Q5 - Conclusion sur la visibilité des données applicatives :**  
Cette session illustre parfaitement le défaut majeur du protocole HTTP : l'absence de chiffrement expose l'intégralité des données échangées (URL consultée, contenu de la réponse, User-Agent, et donc système d'exploitation et navigateur de l'utilisateur) à toute personne réalisant une capture réseau sur le chemin. C'est pourquoi HTTPS (HTTP over TLS) est aujourd'hui la norme. À titre de comparaison, toutes les autres sessions de la capture (TLS/HTTPS) affichent un payload binaire chiffré, illisible sans la clé de session.

---

## Partie 10 - Analyse de la fermeture de connexion

### Réponses

**Q1 - Drapeaux de fermeture utilisés :**  
La fermeture s'effectue avec des drapeaux **FIN-ACK**, caractéristiques d'une fermeture propre et gracieuse. Aucun RST n'est émis pour cette session.

**Q2 - Nombre de paquets participant à la fermeture :**  
**3 paquets** participent à la fermeture observée (fermeture simultanée) :

| Étape | Heure | Source | Drapeaux | Description |
|-------|-------|--------|----------|-------------|
| 1 | 237.621 s | Client (`10.111.17.4`) | `[FIN, ACK]` | Le client initie la fermeture |
| 2 | 237.634 s | Serveur (`34.107.221.82`) | `[FIN, ACK]` | Le serveur acquitte et ferme à son tour |
| 3 | 237.634 s | Client (`10.111.17.4`) | `[ACK]` | Le client acquitte le FIN du serveur |

On observe ici une fermeture en 3 temps (le serveur combine son ACK et son FIN dans le même segment), ce qui est courant et parfaitement valide dans TCP.

**Q3 - Hôte initiateur de la fermeture :**  
C'est le **client** (`10.111.17.4`) qui initie la fermeture en envoyant le premier `FIN-ACK`.

**Q4 - La fermeture est-elle propre et complète ?**  
Oui, la fermeture est **propre et complète**. Les deux hôtes ont échangé des FIN et reçu les ACK correspondants. Aucune donnée n'est perdue. L'état `TIME_WAIT` côté client est implicite (non visible dans la capture mais attendu). La session se termine sans anomalie.

---

## Analyse finale - Synthèse technique

### Q1 - Scénario général de la communication analysée

La machine cliente (Linux, IP `10.111.17.4`, Wi-Fi `wlo1`) exécute Firefox, qui génère automatiquement un trafic de détection de portail captif vers `detectportal.firefox.com` (IP `34.107.221.82`). Firefox ouvre deux connexions HTTP parallèles (ports 51486 et 51494) vers ce serveur, effectue une requête GET sur chacune, reçoit des réponses HTTP 200 OK, maintient les connexions actives par des Keep-Alive pendant environ 2 minutes, puis les ferme proprement. En parallèle, d'autres sessions HTTPS sont actives vers GitHub (140.82.112.26), Cloudflare (162.159.135.234, 104.18.22.112), et Microsoft (13.107.226.42), générant un trafic chiffré non analysable en clair.

### Q2 - Principales étapes de la session TCP

1. **Établissement** (t=120.689 s) : three-way handshake complet en ~15 ms, négociation des options TCP (MSS, SACK, Window Scale, Timestamps).
2. **Transfert de données** (t=120.705 s) : envoi de la requête GET (326 octets), réception de la réponse HTTP 200 OK avec contenu HTML (298 octets de payload TCP).
3. **Maintien de connexion** (t=130 s à t=237 s) : échanges de Keep-Alive toutes les ~10 secondes pour maintenir la session ouverte sans données applicatives.
4. **Fermeture** (t=237.621 s) : fermeture gracieuse en 3 échanges FIN/ACK initiée par le client.

### Q3 - La communication est-elle saine ou dégradée ?

La communication analysée est **saine**. La session HTTP est exemplaire : handshake rapide, échange de données sans retransmission, keep-alive réguliers, fermeture propre. L'ensemble de la capture présente quelques anomalies mineures (6 retransmissions, 9 duplicate ACKs) sur des sessions HTTPS distinctes, mais elles restent ponctuelles et attendues dans un contexte Wi-Fi.

### Q4 - Justification par les observations Wireshark

- **Handshake complet** : les 3 paquets SYN / SYN-ACK / ACK sont présents et cohérents (ISN corrects, options négociées).
- **Aucune retransmission sur la session HTTP** : Wireshark ne signale aucune couleur rouge/jaune sur cette session.
- **RTT stable à ~15 ms** : mesurable entre le SYN et le SYN-ACK, confirmant une connexion réseau stable.
- **Fenêtre TCP élevée et stable** : le client maintient une fenêtre de réception autour de 64 128 octets, signe d'un buffer non saturé et d'un flux bien maîtrisé.
- **Fermeture propre par FIN** : aucun RST, fermeture bilatérale complète, pas de paquets orphelins.
- **Contenu applicatif intègre** : la reconstruction du flux HTTP donne un contenu cohérent et complet (`HTTP 200 OK`, body lisible), sans fragmentation anormale.