#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define DEFAULT_PORT 4444
#define BUFFER_SIZE  1024

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if (argc == 2)
        port = atoi(argv[1]);

    /* Création du socket serveur */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    /* Réutilisation de l'adresse pour éviter "Address already in use" */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Liaison à l'adresse IP / port */
    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    /* Mise en écoute */
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("[*] Serveur en écoute sur le port %d...\n", port);

    /* Accepter une connexion entrante */
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("[+] Connexion reçue de %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    /* Lecture et affichage des ordres reçus */
    char buffer[BUFFER_SIZE];
    ssize_t n;
    while ((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        /* Supprimer le retour à la ligne éventuel */
        buffer[strcspn(buffer, "\r\n")] = '\0';
        printf("[ordre] %s\n", buffer);
    }

    if (n == 0)
        printf("[-] Client déconnecté.\n");
    else
        perror("recv");

    close(client_fd);
    close(server_fd);
    return 0;
}
