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
#define BUFFER_SIZE  65535

/* En-tête TCP pour le calcul du checksum */
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

/* Checksum TCP avec en-tête */
static uint16_t tcp_checksum(const struct iphdr *iph, const struct tcphdr *tcph)
{
    int psize = (int)(sizeof(struct pseudo_header) + sizeof(struct tcphdr));
    char *pseudo = malloc(psize);
    if (!pseudo) return 0;

    struct pseudo_header psh = {
        .src_addr   = iph->saddr,
        .dst_addr   = iph->daddr,
        .placeholder = 0,
        .protocol   = IPPROTO_TCP,
        .tcp_length = htons(sizeof(struct tcphdr)),
    };
    memcpy(pseudo, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

    uint16_t result = checksum((uint16_t *)pseudo, psize);
    free(pseudo);
    return result;
}

/* Construit un paquet IP+TCP SYN-ACK dans `packet` */
static void build_syn_ack(char *packet,
                          const char *src_ip, uint32_t dst_addr,
                          uint16_t dst_port,
                          uint32_t server_seq, uint32_t ack_seq)
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
    iph->daddr    = dst_addr;
    iph->check    = checksum((uint16_t *)iph, sizeof(struct iphdr));

    /* --- TCP header --- */
    tcph->source  = htons(SERVER_PORT);
    tcph->dest    = dst_port;           /* déjà en network byte order */
    tcph->seq     = htonl(server_seq);
    tcph->ack_seq = htonl(ack_seq);
    tcph->doff    = 5;
    tcph->syn     = 1;
    tcph->ack     = 1;
    tcph->window  = htons(65535);
    tcph->check   = 0;
    tcph->check   = tcp_checksum(iph, tcph);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip_serveur>\n", argv[0]);
        return 1;
    }
    const char *server_ip = argv[1];

    srand((unsigned)time(NULL));

    /* Création du raw socket */
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) { perror("socket"); return 1; }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL");
        close(sock);
        return 1;
    }

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  TCP Three-Way Handshake  —  SERVEUR         ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("IP serveur : %s   Port : %d\n\n", server_ip, SERVER_PORT);
    printf("État initial : LISTEN\n");
    printf("En attente d'un SYN...\n\n");

    char buffer[BUFFER_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    /* ISN (Initial Sequence Number) du serveur */
    uint32_t server_seq = (uint32_t)rand();

    /* Variables pour mémoriser le client */
    uint32_t client_seq = 0;
    char client_ip_str[INET_ADDRSTRLEN] = {0};
    uint16_t client_port_net = 0;

    /* Machine à états */
    enum { LISTEN, SYN_RECEIVED, ESTABLISHED } state = LISTEN;

    while (state != ESTABLISHED) {
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr *)&src_addr, &addr_len);
        if (bytes < 0) { perror("recvfrom"); continue; }

        struct iphdr  *iph  = (struct iphdr *)buffer;
        struct tcphdr *tcph = (struct tcphdr *)(buffer + iph->ihl * 4);


        if (ntohs(tcph->dest) != SERVER_PORT) continue;

        if (ntohs(tcph->source) == SERVER_PORT) continue;

        /* ── Étape 1 : réception du SYN ────────────────────── */
        if (state == LISTEN && tcph->syn && !tcph->ack) {
            client_seq     = ntohl(tcph->seq);
            client_port_net = tcph->source;
            inet_ntop(AF_INET, &iph->saddr, client_ip_str, INET_ADDRSTRLEN);

            printf("┌─ Étape 1/3 : SYN reçu ──────────────────────┐\n");
            printf("│  De        : %s:%d\n", client_ip_str, ntohs(tcph->source));
            printf("│  Seq client: %u\n", client_seq);
            printf("│  Flags     : SYN=1 ACK=0\n");
            printf("└──────────────────────────────────────────────┘\n");
            printf("État : LISTEN → SYN_RECEIVED\n\n");
            state = SYN_RECEIVED;

            /* ── Étape 2 : envoi du SYN-ACK ───────────────────── */
            char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
            memset(packet, 0, sizeof(packet));
            build_syn_ack(packet, server_ip, iph->saddr,
                          client_port_net,
                          server_seq, client_seq + 1);

            struct sockaddr_in dst = {
                .sin_family      = AF_INET,
                .sin_port        = client_port_net,
                .sin_addr.s_addr = iph->saddr,
            };

            if (sendto(sock, packet, sizeof(packet), 0,
                       (struct sockaddr *)&dst, sizeof(dst)) < 0) {
                perror("sendto SYN-ACK");
            } else {
                printf("┌─ Étape 2/3 : SYN-ACK envoyé ───────────────┐\n");
                printf("│  Vers      : %s:%d\n", client_ip_str, ntohs(client_port_net));
                printf("│  Seq srv   : %u\n", server_seq);
                printf("│  Ack       : %u  (client_seq + 1)\n", client_seq + 1);
                printf("│  Flags     : SYN=1 ACK=1\n");
                printf("└──────────────────────────────────────────────┘\n\n");
            }

        /* ── Étape 3 : réception du ACK final ──────────────── */
        } else if (state == SYN_RECEIVED && tcph->ack && !tcph->syn) {
            uint32_t ack = ntohl(tcph->ack_seq);

            /* Vérifier que l'ACK cible bien notre SYN-ACK */
            if (ack != server_seq + 1) continue;

            printf("┌─ Étape 3/3 : ACK reçu ──────────────────────┐\n");
            printf("│  De        : %s:%d\n", client_ip_str, ntohs(client_port_net));
            printf("│  Ack       : %u  (server_seq + 1)\n", ack);
            printf("│  Flags     : SYN=0 ACK=1\n");
            printf("└──────────────────────────────────────────────┘\n");
            printf("État : SYN_RECEIVED → ESTABLISHED\n\n");
            printf("══════════════════════════════════════════════\n");
            printf("  CONNEXION ÉTABLIE — Handshake terminé\n");
            printf("══════════════════════════════════════════════\n");
            state = ESTABLISHED;
        }
    }

    close(sock);
    return 0;
}
