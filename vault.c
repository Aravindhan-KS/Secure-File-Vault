#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define KEY 0xB
#define PASSWORD "root123"
#define LOGFILE "vault.log"
#define VAULT_DIR ".vault"


void xor_encrypt(char *buf, int len) {
  int i;
  for (i = 0; i < len; i++)
    buf[i] ^= KEY;
}


void vault_log(char *msg) {
  int fd = open(LOGFILE, O_CREATE | O_RDWR);
  if(fd >= 0){
    char buf[512];
    int n;
    while((n = read(fd, buf, sizeof(buf))) > 0);
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
   close(fd);
  }
}


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

    for(len=0; VAULT_DIR[len]; len++)
        out[len] = VAULT_DIR[len];

    out[len++] = '/';

    int i=0;
    while(filename[i]) {
        out[len++] = filename[i++];
    }

    out[len] = '\0';
}

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

unsigned char gen_key(int pos) {
  const char *pwd = PASSWORD;
  int plen = strlen((char*)pwd);
  return (pwd[pos % plen] * 13 + pos * 7) & 0xFF;
}

#define ENC_KEY 3  

void encrypt_file(char *src) {
  ensure_vault_dir();

  const char *base = src;
int i;
  for (i = 0; src[i]; i++)
    if (src[i] == '/') base = src + i + 1;

  char dst[128];
  build_vault_path((char*)base, dst);

  int infd = open((char*)src, O_RDONLY);
  if (infd < 0) {
    printf(1, "vault: cannot open %s\n", src);
    vault_log("Encrypt: failed to open source");
    return;
  }


  unlink((char*)dst);
int outfd = open(dst, O_CREATE | O_WRONLY);
  if (outfd < 0) {
    printf(1, "vault: cannot create %s\n", dst);
    close(infd);
    vault_log("Encrypt: failed to create destination");
    return;
  }

  char buf[512];
  int n;
  while ((n = read(infd, buf, sizeof(buf))) > 0) {
	int i;
    for (i = 0; i < n; i++)
      buf[i] = buf[i] + ENC_KEY;  
    write(outfd, buf, n);
  }

  close(infd);
  close(outfd);

printf(1, "vault: file encrypted to %s\n", dst);
  vault_log("Encrypt: file stored in vault");
}

void decrypt_file(char *vaultfile, char *dst) {
  char src[128];
  build_vault_path(vaultfile, src);

  int infd = open(src, O_RDONLY);
  if (infd < 0) {
    printf(1, "vault: cannot open vault file %s\n", src);
    vault_log("Decrypt: failed to open vault file");
    return;
  }

  unlink((char *)dst);

  int outfd = open((char*)dst, O_CREATE | O_WRONLY);

if (outfd < 0) {
    printf(1, "vault: cannot create %s\n", dst);
    close(infd);
    vault_log("Decrypt: failed to create destination");
    return;
  }

  char buf[512];
  int n;
  while ((n = read(infd, buf, sizeof(buf))) > 0) {
int i;
    for (i = 0; i < n; i++) {
      buf[i] = buf[i] - ENC_KEY;
    write(outfd, buf, n);
  }

  close(infd);
  close(outfd);
  printf(1, "vault: file decrypted to %s\n", dst);
  vault_log("Decrypt: file retrieved from vault");
}
}

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