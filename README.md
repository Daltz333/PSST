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