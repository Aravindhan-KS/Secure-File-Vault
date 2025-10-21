#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define KEY 0xAA
#define PASSWORD "root123"
#define LOGFILE "vault.log"
#define VAULT_DIR ".vault"

// Encrypt/decrypt buffer
void xor_encrypt(char *buf, int len) {
  int i;
  for (i = 0; i < len; i++)
    buf[i] ^= KEY;
}

// Log messages
void vault_log(char *msg) {
  int fd = open(LOGFILE, O_CREATE | O_RDWR);
  if(fd >= 0){
    char buf[512];
    int n;
    while((n = read(fd, buf, sizeof(buf))) > 0); // move cursor to end
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
   close(fd);
  }
}

// Password check
int check_password() {
  char input[32];
  printf(1, "Enter vault password: ");
  gets(input, sizeof(input));
  int n = strlen(input);
  if (n > 0 && input[n - 1] == '\n')
    input[n - 1] = '\0';
  if (strcmp(input, PASSWORD) == 0) {
    vault_log("Login: SUCCESS");
    return 1;
  } else {
    vault_log("Login: FAILED");
    printf(1, "Access denied.\n");
    return 0;
  }
}

// Ensure /vault directory exists
void ensure_vault_dir() {
  int fd = open(VAULT_DIR, 0);
  if (fd < 0) {
    printf(1, "Creating hidden vault directory...\n");
    mkdir(VAULT_DIR);
  } else {
    close(fd);
  }
}

char* strcat(char* dest, const char* src) {
  char* d = dest + strlen(dest);
  while((*d++ = *src++) != 0);
  return dest;
}

void build_vault_path(char *filename, char *out) {
    int len = 0;

    // Copy VAULT_DIR
    for(len=0; VAULT_DIR[len]; len++)
        out[len] = VAULT_DIR[len];

    // Add '/'
    out[len++] = '/';

    // Copy filename
    int i=0;
    while(filename[i]) {
        out[len++] = filename[i++];
    }

    out[len] = '\0';
}

// Encrypt + move file to /vault/
void encrypt_file(char *src) {
  ensure_vault_dir();
  int fd1, fd2, n;
  char buf[512];
  char dst[128];

  // extract base filename
  char *base = src;
  int i;
  for (i = 0; src[i]; i++)
    if (src[i] == '/')
      base = src + i + 1;

  build_vault_path(base, dst);

  fd1 = open(src, O_RDONLY);
  if (fd1 < 0) {
    printf(1, "vault: cannot open %s\n", src);
    vault_log("Encrypt: Failed to open file");
    return;
  }

  fd2 = open(dst, O_CREATE | O_WRONLY);
  if (fd2 < 0) {
    printf(1, "vault: cannot create %s\n", dst);
    close(fd1);
    vault_log("Encrypt: Failed to create encrypted file");
    return;
  }

  while ((n = read(fd1, buf, sizeof(buf))) > 0) {
    xor_encrypt(buf, n);
    write(fd2, buf, n);
  }

  close(fd1);
  close(fd2);
  vault_log("Encrypt: File stored in hidden vault");
  printf(1, "vault: file encrypted and moved to /vault/\n");
}

// Decrypt file from /vault/
void decrypt_file(char *vaultfile, char *dst) {
  char src[128];
  build_vault_path(vaultfile, src);

  int fd1, fd2, n;
  char buf[512];
  fd1 = open(src, O_RDONLY);
  if (fd1 < 0) {
    printf(1, "vault: cannot open vault file %s\n", src);
    vault_log("Decrypt: Failed to open vault file");
    return;
  }
  fd2 = open(dst, O_CREATE | O_WRONLY);
  if (fd2 < 0) {
    printf(1, "vault: cannot create %s\n", dst);
    close(fd1);
    vault_log("Decrypt: Failed to create output file");
    return;
  }
  while ((n = read(fd1, buf, sizeof(buf))) > 0) {
    xor_encrypt(buf, n);
    write(fd2, buf, n);
  }
  close(fd1);
  close(fd2);
  vault_log("Decrypt: File retrieved from vault");
  printf(1, "vault: file decrypted to %s\n", dst);
}

// Simulate chmod
void fake_chmod(char *path, char *perm) {
  int mode = 0, i;
  for (i = 0; perm[i]; i++)
    mode = mode * 8 + (perm[i] - '0');
  char msg[128];
  strcpy(msg, "Chmod: simulated on ");
  strcat(msg, path);
  vault_log(msg);
  printf(1, "vault: simulated chmod %s %s\n", path, perm);
}

// Show log contents
void show_log() {
  int fd, n;
  char buf[512];
  fd = open(LOGFILE, O_RDONLY);
  if (fd < 0) {
    printf(1, "vault: no log file found\n");
    return;
  }
  while ((n = read(fd, buf, sizeof(buf))) > 0)
    write(1, buf, n);
  close(fd);
}

// main
int main(int argc, char *argv[]) {
  if (!check_password())
    exit();

  if (argc < 2) {
    printf(1, "Usage: vault <command> [args]\n");
    printf(1, "Commands:\n");
    printf(1, "  put <src>          - encrypt and move file into /vault/\n");
    printf(1, "  get <vaultfile> <dst> - decrypt file from /vault/\n");
    printf(1, "  chmod <file> <perm>   - simulate permission change\n");
    printf(1, "  log                - show access log\n");
    exit();
  }

  if (strcmp(argv[1], "put") == 0 && argc == 3)
    encrypt_file(argv[2]);
  else if (strcmp(argv[1], "get") == 0 && argc == 4)
    decrypt_file(argv[2], argv[3]);
  else if (strcmp(argv[1], "chmod") == 0 && argc == 4)
    fake_chmod(argv[2], argv[3]);
  else if (strcmp(argv[1], "log") == 0)
    show_log();
  else
    printf(1, "vault: invalid usage\n");

  exit();
}
