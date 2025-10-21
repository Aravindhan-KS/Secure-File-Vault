# CS3224 XV6 Secure File Vault

A **Secure File Vault** implemented as a user-level program in **XV6 (CS3224)**.  
It provides a password-protected, encrypted storage area with logging and hidden directory support.

---

## Features

- **Encrypt / Decrypt files**: XOR-based encryption ensures files stored in the vault are secure.
- **Password protection**: Access to vault commands requires a password.
- **Hidden vault directory**: Files are stored in a `.vault` directory, hidden from normal `ls`.
- **Logging**: Every operation (login, encryption, decryption, chmod) is logged in `vault.log`.
- **Simulated chmod**: Change permissions on files (simulation; logged for tracking).

---

## File Structure

- `vault.c` — main vault program (user-level)  
- `.vault/` — hidden directory storing encrypted files (auto-created)  
- `vault.log` — logs all vault operations  

---

## Compilation / Integration

1. Place `vault.c` in the XV6 source directory (`xv6-public/`).
2. Add `_vault` to the `UPROGS` in the `Makefile`:

```makefile
UPROGS=\
    _cat\
    _echo\
    _ls\
    _vault\
````

3. Build XV6:

```bash
make clean
make qemu
```

---

## Password

Default password: `root123` (can be changed in `vault.c`):

```c
#define PASSWORD "root123"
```

---

## Commands

```bash
vault <command> [args]
```

**Available commands:**

* `put <file>` — Encrypt and move a file into the hidden vault.
* `get <vaultfile> <destination>` — Decrypt a file from the vault.
* `chmod <file> <mode>` — Simulate permission change (octal).
* `log` — Show vault access log.

---

## Implementation Notes

1. **Directory creation**: xv6 cannot create nested directories automatically. The vault directory is created at runtime if it does not exist:

```c
void ensure_vault_dir() {
    int fd = open(VAULT_DIR, 0);
    if(fd < 0) mkdir(VAULT_DIR);
    else close(fd);
}
```

2. **Path building**: xv6 does not include full `strcat`, so manual concatenation is used:

```c
void build_vault_path(char *filename, char *out) {
    int len=0, i;
    for(len=0; VAULT_DIR[len]; len++) out[len] = VAULT_DIR[len];
    out[len++] = '/';
    for(i=0; filename[i]; i++) out[len++] = filename[i];
    out[len] = '\0';
}
```

3. **File operations**:

* Do **not use `O_APPEND`** (not supported).
* Use `O_CREATE | O_RDWR` for logs and manually move cursor to the end if appending:

```c
int fd = open(LOGFILE, O_CREATE | O_RDWR);
if(fd >= 0){
    char buf[512]; int n;
    while((n = read(fd, buf, sizeof(buf))) > 0); // move cursor to end
    write(fd, msg, strlen(msg));
    write(fd, "\n", 1);
    close(fd);
}
```

4. **Hidden directory**: Rename `VAULT_DIR` to `.vault` to hide it from `ls`. Patch `ls.c` to skip entries starting with `.`:

```c
if(de.name[0] == '.') continue;
```

5. **Logging**: Each operation writes a separate line in `vault.log`.

6. **Encryption**: XOR-based per-byte encryption; simple but sufficient for lab purposes.

7. **Password protection**: Verified with `strcmp(input, PASSWORD)` at runtime.

---

## Example Workflow

### 1. Create a test file

```bash
$ echo hello > test.txt
$ cat test.txt
hello
```

### 2. Encrypt into vault

```bash
$ vault put test.txt
Enter vault password: root123
Creating hidden vault directory...
vault: file encrypted and moved to /.vault/
```

### 3. Check hidden directory

```bash
$ ls
cat echo ls vault vault.log   # .vault is hidden
$ ls .vault
test.txt
```

### 4. Decrypt from vault

```bash
$ vault get test.txt output.txt
Enter vault password: root123
vault: file decrypted to output.txt
$ cat output.txt
hello
```

### 5. Simulate chmod

```bash
$ vault chmod output.txt 777
Enter vault password: root123
vault: simulated chmod output.txt 777
```

### 6. View logs

```bash
$ vault log
Enter vault password: root123
Login: SUCCESS
Encrypt: File stored in vault
Decrypt: File retrieved from vault
Chmod: simulated on output.txt
```

### 7. Wrong password test

```bash
$ vault log
Enter vault password: wrongpassword
Access denied.
```

---

## Limitations

* XOR encryption is **not secure for real-world use**; educational only.
* Logging overwrites file if manual cursor not moved; no `O_APPEND`.
* Simulated `chmod` only logs the permission change; xv6 does not support real permission bits at user-level.

---

## Quick Verification Script

Inside xv6:

```bash
$ echo hello > test.txt
$ vault put test.txt
$ vault get test.txt output.txt
$ cat output.txt
$ vault chmod output.txt 777
$ vault log
$ ls
$ ls .vault
```

✅ This confirms all features: encryption, decryption, logging, hidden vault, and password protection.


