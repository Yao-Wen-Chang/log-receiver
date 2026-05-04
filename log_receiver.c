#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define DEFAULT_PORT  8080
#define DEFAULT_FILE  "fan_control.log"
#define MAX_BODY      4096

static volatile int g_running = 1;
static void sig_handler(int sig) { (void)sig; g_running = 0; }

static void handle_conn(int fd, FILE *log) {
    char line[512];
    int  li = 0, content_length = 0;
    char c;

    /* read HTTP headers until blank line, pick up Content-Length */
    while (1) {
        if (read(fd, &c, 1) != 1) return;
        if (c == '\n') {
            if (li > 0 && line[li - 1] == '\r') li--;
            line[li] = '\0';
            if (li == 0) break;   /* blank line = end of headers */
            if (strncasecmp(line, "content-length:", 15) == 0)
                content_length = atoi(line + 15);
            li = 0;
        } else {
            if (li < (int)sizeof(line) - 1) line[li++] = c;
        }
    }

    if (content_length <= 0 || content_length > MAX_BODY) goto respond;

    /* read body */
    char body[MAX_BODY + 1];
    int got = 0;
    while (got < content_length) {
        int n = read(fd, body + got, content_length - got);
        if (n <= 0) break;
        got += n;
    }
    /* strip trailing whitespace */
    while (got > 0 && (body[got - 1] == '\n' || body[got - 1] == '\r')) got--;
    body[got] = '\0';

    if (got > 0) {
        fprintf(log, "%s\n", body);
        fflush(log);
        printf("%s\n", body);
        fflush(stdout);
    }

respond:
    write(fd, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n", 46);
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [--port <port>] [--file <path>]\n", prog);
    fprintf(stderr, "  --port  TCP port to listen on (default: %d)\n", DEFAULT_PORT);
    fprintf(stderr, "  --file  log file to append to  (default: %s)\n", DEFAULT_FILE);
}

int main(int argc, char *argv[]) {
    time_t cur_time;
    struct tm *info;
    char log_path_buffer[64];
    
    /* Setup current time */
    time(&cur_time);
    setenv("TZ", "Asia/Taipei", 1);
    tzset();
    info = localtime(&cur_time);
    strftime(log_path_buffer, sizeof(log_path_buffer), "%Y-%m-%d_%H:%M:%S_", info);
    int buffer_len = strlen(log_path_buffer);
    snprintf(log_path_buffer + buffer_len, sizeof(log_path_buffer) - buffer_len, "%s", DEFAULT_FILE);


    int port = DEFAULT_PORT;
    const char *log_path = log_path_buffer;
    printf("Log will be saved to: %s\n", log_path);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--file") == 0 && i + 1 < argc)
            log_path = argv[++i];
        else if (strcmp(argv[i], "--help") == 0)
            { usage(argv[0]); return 0; }
    }

    FILE *log = fopen(log_path, "a");
    if (!log) { perror(log_path); return 1; }

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(port)
    };
    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(srv, 8) < 0) { perror("listen"); return 1; }

    signal(SIGTERM, sig_handler);
    signal(SIGINT,  sig_handler);

    printf("Listening on  :%d\n", port);
    printf("Writing to    %s\n", log_path);
    printf("Board config  remote_host <laptop-ip>, remote_port %d\n\n", port);
    fflush(stdout);

    while (g_running) {
        int fd = accept(srv, NULL, NULL);
        if (fd < 0) { if (g_running) perror("accept"); break; }
        handle_conn(fd, log);
        close(fd);
    }

    close(srv);
    fclose(log);
    return 0;
}
