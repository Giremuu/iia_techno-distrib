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

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip_serveur>\n", argv[0]);
        return 1;
    }
    const char *server_ip = argv[1];

    srand((unsigned)time(NULL));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(SERVER_PORT),
    };
    if (inet_pton(AF_INET, server_ip, &srv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide : %s\n", server_ip);
        close(sock); return 1;
    }

    if (connect(sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  TLS 1.2 Handshake  —  CLIENT                ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("Serveur : %s:%d\n\n", server_ip, SERVER_PORT);
    printf("Connexion TCP établie.\n");
    printf("État initial : IDLE\n\n");

    char msg[MSG_SIZE];
    char hex[256];

    /* ─────────────────────────────────────────────── */
    /* Étape 1 : envoi du ClientHello                 */
    /* ─────────────────────────────────────────────── */
    if (tls_send(sock, "CLIENT_HELLO") < 0) {
        perror("send CLIENT_HELLO"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 1 : ClientHello envoyé ──────────────┐\n");
    printf("│  Vers           : %s:%d\n", server_ip, SERVER_PORT);
    printf("│  Version max    : TLS 1.2 (0x0303)\n");
    printf("│  Client Random  : %s\n", hex);
    printf("│  Cipher Suites  : TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384\n");
    printf("│                   TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256\n");
    printf("│  Extensions     : SNI, ALPN, session_ticket\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : IDLE → WAIT_SERVER_HELLO\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 2a : réception du ServerHello            */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "SERVER_HELLO") != 0) {
        fprintf(stderr, "Attendu SERVER_HELLO\n"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 2a : ServerHello reçu ───────────────┐\n");
    printf("│  De             : %s:%d\n", server_ip, SERVER_PORT);
    printf("│  Version        : TLS 1.2 (0x0303)\n");
    printf("│  Server Random  : %s\n", hex);
    printf("│  Session ID     : %08x\n", rand());
    printf("│  Cipher Suite   : TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_SERVER_HELLO → WAIT_CERTIFICATE\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 2b : réception du Certificate            */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "CERTIFICATE") != 0) {
        fprintf(stderr, "Attendu CERTIFICATE\n"); goto end;
    }
    fake_hex(hex, 8);
    printf("┌─ Étape 2b : Certificate reçu ───────────────┐\n");
    printf("│  Sujet          : CN=serveur.local, O=IIA\n");
    printf("│  Émetteur       : CN=IIA-CA\n");
    printf("│  Validité       : 2024-01-01 → 2026-01-01\n");
    printf("│  Clé publique   : RSA 2048 bits\n");
    printf("│  Empreinte SHA1 : %s\n", hex);
    printf("│  Vérification   : OK (chaîne de confiance valide)\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_CERTIFICATE → WAIT_SERVER_KEY_EXCHANGE\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 2c : réception du ServerKeyExchange      */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "SERVER_KEY_EXCHANGE") != 0) {
        fprintf(stderr, "Attendu SERVER_KEY_EXCHANGE\n"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 2c : ServerKeyExchange reçu ─────────┐\n");
    printf("│  Courbe         : secp256r1 (NIST P-256)\n");
    printf("│  Clé publique   : %s\n", hex);
    printf("│  Signature      : RSA-SHA256 (vérifiée avec le cert)\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_SERVER_KEY_EXCHANGE → WAIT_SERVER_HELLO_DONE\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 2d : réception du ServerHelloDone        */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "SERVER_HELLO_DONE") != 0) {
        fprintf(stderr, "Attendu SERVER_HELLO_DONE\n"); goto end;
    }
    printf("┌─ Étape 2d : ServerHelloDone reçu ───────────┐\n");
    printf("│  Le serveur a fini d'envoyer ses paramètres. │\n");
    printf("│  → Génération de la clé ECDH cliente...      │\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_SERVER_HELLO_DONE → ENVOI_CKE\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 3a : envoi du ClientKeyExchange          */
    /* ─────────────────────────────────────────────── */
    if (tls_send(sock, "CLIENT_KEY_EXCHANGE") < 0) {
        perror("send CLIENT_KEY_EXCHANGE"); goto end;
    }
    fake_hex(hex, 16);
    printf("┌─ Étape 3a : ClientKeyExchange envoyé ───────┐\n");
    printf("│  Vers           : %s:%d\n", server_ip, SERVER_PORT);
    printf("│  Clé publique   : %s\n", hex);
    printf("│  → Calcul du Pre-Master Secret (ECDH)\n");
    printf("│  → Dérivation Master Secret + clés de session\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 3b : envoi du ChangeCipherSpec           */
    /* ─────────────────────────────────────────────── */
    if (tls_send(sock, "CHANGE_CIPHER_SPEC") < 0) {
        perror("send CHANGE_CIPHER_SPEC"); goto end;
    }
    printf("┌─ Étape 3b : ChangeCipherSpec envoyé ────────┐\n");
    printf("│  Le client bascule sur le chiffrement        │\n");
    printf("│  négocié (AES-256-GCM).                      │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 3c : envoi du Finished                   */
    /* ─────────────────────────────────────────────── */
    if (tls_send(sock, "FINISHED") < 0) {
        perror("send FINISHED"); goto end;
    }
    fake_hex(hex, 12);
    printf("┌─ Étape 3c : Finished envoyé ────────────────┐\n");
    printf("│  PRF verify_data : %s\n", hex);
    printf("│  (Hash PRF de tous les messages du handshake)│\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : ENVOI_CKE → WAIT_SERVER_FINISHED\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 4a : réception du ChangeCipherSpec srv   */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "CHANGE_CIPHER_SPEC") != 0) {
        fprintf(stderr, "Attendu CHANGE_CIPHER_SPEC (serveur)\n"); goto end;
    }
    printf("┌─ Étape 4a : ChangeCipherSpec reçu (srv) ────┐\n");
    printf("│  Le serveur bascule sur le chiffrement       │\n");
    printf("│  négocié (AES-256-GCM).                      │\n");
    printf("└──────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────── */
    /* Étape 4b : réception du Finished serveur       */
    /* ─────────────────────────────────────────────── */
    if (tls_recv(sock, msg) < 0 || strcmp(msg, "FINISHED") != 0) {
        fprintf(stderr, "Attendu FINISHED (serveur)\n"); goto end;
    }
    fake_hex(hex, 12);
    printf("┌─ Étape 4b : Finished reçu (serveur) ────────┐\n");
    printf("│  PRF verify_data : %s\n", hex);
    printf("│  → Vérification du hash du handshake : OK    │\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : WAIT_SERVER_FINISHED → ESTABLISHED\n\n");

    printf("══════════════════════════════════════════════\n");
    printf("  SESSION TLS ÉTABLIE — Handshake terminé\n");
    printf("  Chiffrement : AES-256-GCM\n");
    printf("  Intégrité   : SHA-384\n");
    printf("══════════════════════════════════════════════\n");

end:
    close(sock);
    return 0;
}
