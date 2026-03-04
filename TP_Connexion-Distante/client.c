#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 4444
#define BUFFER_SIZE  1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <adresse_ip> [port]\n", argv[0]);
        fprintf(stderr, "Exemple: %s 192.168.1.1 4444\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;

    /* Création du socket client */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    /* Adresse du serveur distant */
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
    };

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide : %s\n", server_ip);
        close(sock_fd);
        return 1;
    }

    /* Tentative de connexion */
    printf("[*] Connexion vers %s:%d...\n", server_ip, port);
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }
    printf("[+] Connecté à %s:%d\n", server_ip, port);

    /* Envoi d'ordres saisis au clavier */
    char buffer[BUFFER_SIZE];
    printf("Entrez les ordres à envoyer (Ctrl+D pour quitter) :\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        ssize_t sent = send(sock_fd, buffer, strlen(buffer), 0);
        if (sent < 0) {
            perror("send");
            break;
        }
    }

    printf("[-] Déconnexion.\n");
    close(sock_fd);
    return 0;
}
