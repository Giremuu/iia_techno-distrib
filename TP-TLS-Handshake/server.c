#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT  4433
#define MSG_SIZE     64

/* Génère une chaîne hexadécimale simulant des octets aléatoires */
static void fake_hex(char *buf, int nbytes)
{
    int pos = 0;
    for (int i = 0; i < nbytes; i++) {
        pos += sprintf(buf + pos, "%02x", rand() & 0xFF);
        if (i % 16 == 15 && i != nbytes - 1)
            pos += sprintf(buf + pos, "\n│             ");
        else if (i != nbytes - 1)
            pos += sprintf(buf + pos, " ");
    }
}

/* Envoie un message TLS simulé */
static int tls_send(int fd, const char *msg)
{
    char buf[MSG_SIZE];
    memset(buf, 0, MSG_SIZE);
    strncpy(buf, msg, MSG_SIZE - 1);
    return send(fd, buf, MSG_SIZE, 0) == MSG_SIZE ? 0 : -1;
}

/* Reçoit un message TLS simulé */
static int tls_recv(int fd, char *msg)
{
    char buf[MSG_SIZE];
    int n = recv(fd, buf, MSG_SIZE, MSG_WAITALL);
    if (n != MSG_SIZE) return -1;
    strncpy(msg, buf, MSG_SIZE - 1);
    msg[MSG_SIZE - 1] = '\0';
    return 0;
}

int main(void)
{
    srand((unsigned)time(NULL));

    int srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    if (bind(srv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(srv_fd); return 1;
    }
    if (listen(srv_fd, 1) < 0) {
        perror("listen"); close(srv_fd); return 1;
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  TLS Handshake  —  SERVEUR               ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("Port : %d\n\n", SERVER_PORT);
    printf("État initial : LISTEN\n");
    printf("En attente d'une connexion...\n\n");

    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
    if (cli_fd < 0) { perror("accept"); close(srv_fd); return 1; }

    char cli_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, sizeof(cli_ip));
    printf("Connexion TCP établie depuis %s:%d\n\n", cli_ip, ntohs(cli_addr.sin_port));

    char msg[MSG_SIZE];
    char hex[256];

    /* ─────────────────────────────────────────────── */
    /* Étape 1 : réception du ClientHello             */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(cli_fd, msg) < 0 || strcmp(msg, "CLIENT_HELLO") != 0) {
        fprintf(stderr, "Attendu CLIENT_HELLO\n"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 1 : ClientHello reçu ────────────────┐\n");
    printf("│  De             : %s:%d\n", cli_ip, ntohs(cli_addr.sin_port));
    printf("│  Version        : TLS (0x0303)\n");
    printf("│  Client Random  : %s\n", hex);
    printf("│  Cipher Suites  : TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384\n");
    printf("│                   TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256\n");
    printf("│  Extensions     : SNI, ALPN, session_ticket\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : LISTEN → WAIT_CLIENT_KEY_EXCHANGE\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 2 : envoi du ServerHello                 */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "SERVER_HELLO") < 0) goto end;
    fake_hex(hex, 16);
    printf("┌─ Étape 2a : ServerHello envoyé ─────────────┐\n");
    printf("│  Vers           : %s:%d\n", cli_ip, ntohs(cli_addr.sin_port));
    printf("│  Version        : TLS (0x0303)\n");
    printf("│  Server Random  : %s\n", hex);
    printf("│  Session ID     : %08x\n", rand());
    printf("│  Cipher Suite   : TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384\n");
    printf("│  Compression    : null\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 3 : envoi du Certificate                 */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "CERTIFICATE") < 0) goto end;
    fake_hex(hex, 8);
    printf("┌─ Étape 2b : Certificate envoyé ─────────────┐\n");
    printf("│  Sujet          : CN=serveur.local, O=IIA\n");
    printf("│  Émetteur       : CN=IIA-CA\n");
    printf("│  Validité       : 2024-01-01 → 2026-01-01\n");
    printf("│  Clé publique   : RSA 2048 bits\n");
    printf("│  Empreinte SHA1 : %s\n", hex);
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 4 : envoi du ServerKeyExchange            */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "SERVER_KEY_EXCHANGE") < 0) goto end;
    fake_hex(hex, 16);
    printf("┌─ Étape 2c : ServerKeyExchange envoyé ───────┐\n");
    printf("│  Courbe         : secp256r1 (NIST P-256)\n");
    printf("│  Clé publique   : %s\n", hex);
    printf("│  Signature      : RSA-SHA256 (vérifie le certificat)\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 5 : envoi du ServerHelloDone             */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "SERVER_HELLO_DONE") < 0) goto end;
    printf("┌─ Étape 2d : ServerHelloDone envoyé ─────────┐\n");
    printf("│  Le serveur a fini d'envoyer ses messages.   │\n");
    printf("│  En attente de la réponse du client...       │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 6 : réception du ClientKeyExchange       */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(cli_fd, msg) < 0 || strcmp(msg, "CLIENT_KEY_EXCHANGE") != 0) {
        fprintf(stderr, "Attendu CLIENT_KEY_EXCHANGE\n"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 3a : ClientKeyExchange reçu ─────────┐\n");
    printf("│  Clé publique   : %s\n", hex);
    printf("│  → Calcul du pré Secret (ECDH)\n");
    printf("│  → Dérivation pré Secret + clés de session\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 7 : réception du ChangeCipherSpec client */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(cli_fd, msg) < 0 || strcmp(msg, "CHANGE_CIPHER_SPEC") != 0) {
        fprintf(stderr, "Attendu CHANGE_CIPHER_SPEC (client)\n"); goto end;
    }
    printf("┌─ Étape 3b : ChangeCipherSpec reçu (client) ─┐\n");
    printf("│  Le client bascule sur le chiffrement        │\n");
    printf("│  négocié (AES-256-GCM).                      │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 8 : réception du Finished client         */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(cli_fd, msg) < 0 || strcmp(msg, "FINISHED") != 0) {
        fprintf(stderr, "Attendu FINISHED (client)\n"); goto end;
    }
    fake_hex(hex, 12);
    printf("┌─ Étape 3c : Finished reçu (client) ─────────┐\n");
    printf("│  PRF verify_data : %s\n", hex);
    printf("│  → Vérification du hash du handshake : OK    │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 9 : envoi du ChangeCipherSpec serveur    */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "CHANGE_CIPHER_SPEC") < 0) goto end;
    printf("┌─ Étape 4a : ChangeCipherSpec envoyé (srv) ──┐\n");
    printf("│  Le serveur bascule sur le chiffrement       │\n");
    printf("│  négocié (AES-256-GCM).                      │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 10 : envoi du Finished serveur           */
    /* ─────────────────────────────────────────────── */
    if (tls_send(cli_fd, "FINISHED") < 0) goto end;
    fake_hex(hex, 12);
    printf("┌─ Étape 4b : Finished envoyé (serveur) ──────┐\n");
    printf("│  PRF verify_data : %s\n", hex);
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_CLIENT_KEY_EXCHANGE → ESTABLISHED\n\n");

    printf("══════════════════════════════════════════════\n");
    printf("  SESSION TLS ÉTABLIE — Handshake terminé\n");
    printf("  Chiffrement : AES-256-GCM\n");
    printf("  Intégrité   : SHA-384\n");
    printf("══════════════════════════════════════════════\n");

end:
    close(cli_fd);
    close(srv_fd);
    return 0;
}
