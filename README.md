# PSST

COSC 423 Project #1

## Project Goals

- User Psst! Client:
  - Provides a user interface that implements secure communication with other Psst! users.
  - Performs Psst! message encryption and decryption using RSA PKE.
- User SYH Client:
  - Implements the client part of two-factor authentication with the SYH server.
- SYH Server: 
  - Communicates with the Mailbox Server and Psst! clients to implement two-factor authentication.
- Public Key Server:
  - Manages users' public-keys.
- Mailbox Server:
  - Performs two-step user authentication.
  - Maintains the mailboxes of encrypted "Psst!" messages between users.

## Run Instructions

Run the following in separate bash instances:

- Run `./bin/auth 127.0.0.1`
- Run `./bin/auth_mgmt 127.0.0.1`
- Run `./bin/psst 127.0.0.1`
- Run `./bin/mailbox 127.0.0.1`

1. Psst will prompt for a public key. A hardcoded n value of 221 is used, so 23 is recommended.
2. Psst will then generate the private key. For 23, this should be 167 (d = 167).
3. Head over to the `auth` shell. It's currently waiting for a private key. Enter in the private key generated.
4. Head back to Psst and enter in the user ID. It will now attempt to login.
5. Head back to `auth`, it's waiting for the user to confirm if the login attempt is allowed. Type `y`.
6. Login attempt is successful.

### Valid client commands

- `view`
- `send`
- `fasdf`

## Build Instructions

Project uses a standard `Makefile` for building.

```bash
make clean
make all
```

### Targets

- `make auth` - Builds the 2FA authentication server
- `make auth_mgmt` - Builds the client for registering users on the 2FA server
- `make mailbox` - Builds the mailbox server for storing messages
- `make psst` - Builds the client
- `make all` - Builds all of the above
- `make clean` - Cleans the `bin` and `obj` directories

## Directory Structure

The project is composed of four directories under `src`. 

- `auth` - code specific to the auth server
- `auth_mgmt` code specific to auth management client
- `mailbox` - code specific to the mailbox server
- `psst` - code specific to the client program
- `shared` - shared code that should be duplicated among targets

## Binaries

Binaries are located in `bin` with their respective target names.