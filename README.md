# Nanopass -  Minimalistic encrypted password storage

## About

Minimalistic password manager with GUI and hierarchical search function.
Data is encrypted with AES256 and signed with a SHA3-HMAC.
Windows builds need Visual C build tools by Microsoft.
GTK+3.0 used, tested with version 3.18 and 3.14.

Install development headers with: `apt install build-essential libgtk-3-dev`

Icons by Yannick Lung (http://yannicklung.com/)

https://www.iconfinder.com/icons/314756/key_icon
https://www.iconfinder.com/icons/315201/document_locked_icon
https://www.iconfinder.com/icons/314700/combination_lock_icon
Icon C++ files generated using: xxd -i image.png > image.cpp

For more information, have a look at the homepage:

https://evelance.de/nanopass_password_manager/index.htm

## File format

```
File structure: .npw (Passwords database file) Version 1.0
 | "NANOPASSDB"
 | Major Version           - 1 Byte
 | "."
 | Minor Version           - 1 Byte
 | Payload

Payload Version 1.0
 | Key Derivation Salt     - 32 Byte
 | AES CTR IV              - 12 Byte
 | HMAC of encrypted file  - 32 Byte
 | Encrypted data length   -  4 Byte
 | Encrypted data          - 0..2^32 Byte
```
