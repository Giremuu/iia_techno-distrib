
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SERVER_PORT  4444
#define CLIENT_PORT  5555
#define BUFFER_SIZE  65535

/* En-tête pseudo TCP pour le calcul du checksum */
struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t  placeholder;
    uint8_t  protocol;
    uint16_t tcp_length;
};

/* Calcul du checksum Internet */
static uint16_t checksum(const uint16_t *ptr, int nbytes)
{
    unsigned long sum = 0;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        uint16_t odd = 0;
        *(uint8_t *)&odd = *(const uint8_t *)ptr;
        sum += odd;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

/* Checksum TCP avec pseudo en-tête */
static uint16_t tcp_checksum(const struct iphdr *iph, const struct tcphdr *tcph)
{
    int psize = (int)(sizeof(struct pseudo_header) + sizeof(struct tcphdr));
    char *pseudo = malloc(psize);
    if (!pseudo) return 0;

    struct pseudo_header psh = {
        .src_addr    = iph->saddr,
        .dst_addr    = iph->daddr,
        .placeholder = 0,
        .protocol    = IPPROTO_TCP,
        .tcp_length  = htons(sizeof(struct tcphdr)),
    };
    memcpy(pseudo, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

    uint16_t result = checksum((uint16_t *)pseudo, psize);
    free(pseudo);
    return result;
}

/* Construit un paquet IP+TCP SYN dans `packet` */
static void build_syn(char *packet,
                      const char *src_ip, const char *dst_ip,
                      uint32_t client_seq)
{
    struct iphdr  *iph  = (struct iphdr *)packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

    /* --- IP header --- */
    iph->ihl      = 5;
    iph->version  = 4;
    iph->tos      = 0;
    iph->tot_len  = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
    iph->id       = htons((uint16_t)(rand() & 0xFFFF));
    iph->frag_off = 0;
    iph->ttl      = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check    = 0;
    inet_pton(AF_INET, src_ip, &iph->saddr);
    inet_pton(AF_INET, dst_ip, &iph->daddr);
    iph->check    = checksum((uint16_t *)iph, sizeof(struct iphdr));

    /* --- TCP header --- */
    tcph->source  = htons(CLIENT_PORT);
    tcph->dest    = htons(SERVER_PORT);
    tcph->seq     = htonl(client_seq);
    tcph->ack_seq = 0;
    tcph->doff    = 5;
    tcph->syn     = 1;
    tcph->ack     = 0;
    tcph->window  = htons(65535);
    tcph->check   = 0;
    tcph->check   = tcp_checksum(iph, tcph);
}

/* Construit un paquet IP+TCP ACK dans `packet` */
static void build_ack(char *packet,
                      const char *src_ip, const char *dst_ip,
                      uint32_t client_seq, uint32_t ack_seq)
{
    struct iphdr  *iph  = (struct iphdr *)packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

    /* --- IP header --- */
    iph->ihl      = 5;
    iph->version  = 4;
    iph->tos      = 0;
    iph->tot_len  = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
    iph->id       = htons((uint16_t)(rand() & 0xFFFF));
    iph->frag_off = 0;
    iph->ttl      = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check    = 0;
    inet_pton(AF_INET, src_ip, &iph->saddr);
    inet_pton(AF_INET, dst_ip, &iph->daddr);
    iph->check    = checksum((uint16_t *)iph, sizeof(struct iphdr));

    /* --- TCP header --- */
    tcph->source  = htons(CLIENT_PORT);
    tcph->dest    = htons(SERVER_PORT);
    tcph->seq     = htonl(client_seq);
    tcph->ack_seq = htonl(ack_seq);
    tcph->doff    = 5;
    tcph->syn     = 0;
    tcph->ack     = 1;
    tcph->window  = htons(65535);
    tcph->check   = 0;
    tcph->check   = tcp_checksum(iph, tcph);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_client> <ip_serveur>\n", argv[0]);
        return 1;
    }
    const char *client_ip = argv[1];
    const char *server_ip = argv[2];

    srand((unsigned)time(NULL));

    /* Création du raw socket */
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) { perror("socket"); return 1; }

    /* On fournit nous-mêmes l'en-tête IP */
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL");
        close(sock);
        return 1;
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  TCP Three-Way Handshake  —  CLIENT          ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("IP client  : %s   Port : %d\n", client_ip, CLIENT_PORT);
    printf("IP serveur : %s   Port : %d\n\n", server_ip, SERVER_PORT);
    printf("État initial : CLOSED\n\n");

    /* ISN (Initial Sequence Number) du client */
    uint32_t client_seq = (uint32_t)rand();
    uint32_t server_seq = 0;

    struct sockaddr_in dst_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(SERVER_PORT),
    };
    inet_pton(AF_INET, server_ip, &dst_addr.sin_addr);

    /* ── Étape 1 : envoi du SYN ────────────────────────── */
    char syn_packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(syn_packet, 0, sizeof(syn_packet));
    build_syn(syn_packet, client_ip, server_ip, client_seq);

    if (sendto(sock, syn_packet, sizeof(syn_packet), 0,
               (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
        perror("sendto SYN");
        close(sock);
        return 1;
    }

    printf("┌─ Étape 1/3 : SYN envoyé ────────────────────┐\n");
    printf("│  Vers      : %s:%d\n", server_ip, SERVER_PORT);
    printf("│  Seq client: %u\n", client_seq);
    printf("│  Flags     : SYN=1 ACK=0\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : CLOSED → SYN_SENT\n\n");

    /* ── Étape 2 : attente du SYN-ACK ──────────────────── */
    char buffer[BUFFER_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    int got_syn_ack = 0;
    while (!got_syn_ack) {
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr *)&src_addr, &addr_len);
        if (bytes < 0) { perror("recvfrom"); continue; }

        struct iphdr  *iph  = (struct iphdr *)buffer;
        struct tcphdr *tcph = (struct tcphdr *)(buffer + iph->ihl * 4);

        /* Filtrer : SYN-ACK venant du serveur vers notre port */
        if (ntohs(tcph->source) != SERVER_PORT) continue;
        if (ntohs(tcph->dest)   != CLIENT_PORT) continue;
        if (!tcph->syn || !tcph->ack)           continue;

        /* Vérifier que l'ACK cible bien notre SYN */
        if (ntohl(tcph->ack_seq) != client_seq + 1) continue;

        server_seq = ntohl(tcph->seq);

        printf("┌─ Étape 2/3 : SYN-ACK reçu ─────────────────┐\n");
        printf("│  De        : %s:%d\n", server_ip, SERVER_PORT);
        printf("│  Seq srv   : %u\n", server_seq);
        printf("│  Ack       : %u  (client_seq + 1)\n", ntohl(tcph->ack_seq));
        printf("│  Flags     : SYN=1 ACK=1\n");
        printf("└──────────────────────────────────────────────┘\n\n");
        got_syn_ack = 1;
    }

    /* ── Étape 3 : envoi du ACK final ──────────────────── */
    char ack_packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(ack_packet, 0, sizeof(ack_packet));
    build_ack(ack_packet, client_ip, server_ip,
              client_seq + 1, server_seq + 1);

    if (sendto(sock, ack_packet, sizeof(ack_packet), 0,
               (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0) {
        perror("sendto ACK");
        close(sock);
        return 1;
    }

    printf("┌─ Étape 3/3 : ACK envoyé ────────────────────┐\n");
    printf("│  Vers      : %s:%d\n", server_ip, SERVER_PORT);
    printf("│  Ack       : %u  (server_seq + 1)\n", server_seq + 1);
    printf("│  Flags     : SYN=0 ACK=1\n");
    printf("└──────────────────────────────────────────────┘\n");
    printf("État : SYN_SENT → ESTABLISHED\n\n");
    printf("══════════════════════════════════════════════\n");
    printf("  CONNEXION ÉTABLIE — Handshake terminé\n");
    printf("══════════════════════════════════════════════\n");

    close(sock);
    return 0;
}
