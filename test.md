## 1️⃣ Create a test file

```bash
$ echo hello > test.txt
$ cat test.txt
hello
```

✅ Confirms your source file exists.

---

## 2️⃣ Encrypt file into hidden vault

```bash
$ vault put test.txt
Enter vault password: root123
vault: file encrypted and moved to /.vault/
```

Check hidden folder:

```bash
$ ls
cat echo ls vault vault.log     # .vault is hidden
$ ls .vault
test.txt                        # Encrypted file inside
```

✅ Shows file moved into hidden vault.

---

## 3️⃣ Decrypt file from hidden vault

```bash
$ vault get test.txt output.txt
Enter vault password: root123
vault: file decrypted to output.txt
$ cat output.txt
hello
```

✅ Confirms decryption works.

---

## 4️⃣ Simulate chmod

```bash
$ vault chmod output.txt 777
Enter vault password: root123
vault: simulated chmod output.txt 777
```

✅ Confirms permission simulation and logs entry.

---

## 5️⃣ View access logs

```bash
$ vault log
Enter vault password: root123
Login: SUCCESS
Encrypt: File stored in hidden vault
Decrypt: File retrieved from vault
Chmod: simulated on output.txt
```

✅ Shows all vault operations logged correctly.

---

## 6️⃣ Test wrong password

```bash
$ vault log
Enter vault password: wrongpassword
Access denied.
```

✅ Confirms password protection works; failed login is denied.

---

### Optional

* Check hidden vault directory is invisible in normal `ls`:

```bash
$ ls
cat echo ls vault vault.log     # .vault not listed
```

* Directly list vault files:

```bash
$ ls .vault
test.txt
```

---

This sequence **fully verifies all features**: encryption, decryption, password protection, hidden directory, logging, and simulated chmod.