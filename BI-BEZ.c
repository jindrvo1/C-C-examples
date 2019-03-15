// Vojtech Jindra
#include <stdlib.h>
#include <openssl/evp.h>
#include <string.h>
 
int main(void) {
  unsigned char ot[8];  // open text
  unsigned char st[8];  // sifrovany text
  unsigned char key[EVP_MAX_KEY_LENGTH] = "Muj klic";  // klic pro sifrovani
  unsigned char iv[EVP_MAX_IV_LENGTH] = "inicial. vektor";  // inicializacni vektor

  int otLength = sizeof(ot);
  int stLength = 0;

  char buffer[55];

  FILE *file = fopen("Mad_scientist.bmp", "r");
  
  // ECB 
  FILE *new_file = fopen("Mad_scientist_ecb.bmp", "wb");
  FILE *new_file_dec = fopen("Mad_scientist_dec.bmp", "wb");

  fread(buffer, sizeof(char), 54, file);
  fwrite(buffer, sizeof(char), 54, new_file);

  EVP_CIPHER_CTX ctx; 
  EVP_EncryptInit(&ctx, EVP_des_ecb(), key, iv);  

  while ( fread(ot, sizeof(char), otLength, file) != 0 ) {
    EVP_EncryptUpdate(&ctx, st, &stLength, ot, otLength);  
    fwrite(st, sizeof(char), stLength, new_file);
  }

  EVP_EncryptFinal(&ctx, st, &stLength); 
 

  fclose(new_file);

  // ECB decypher
  new_file = fopen("Mad_scientist_ecb.bmp", "r");

  otLength = 0;
  stLength = sizeof(st);

  fread(buffer, sizeof(char), 54, new_file);
  fwrite(buffer, sizeof(char), 54, new_file_dec);
     
  EVP_DecryptInit(&ctx, EVP_des_ecb(), key, iv);

  while ( fread(st, sizeof(char), stLength, new_file) != 0 ) {
    EVP_DecryptUpdate(&ctx, ot, &otLength,  st, stLength);  
    fwrite(ot, sizeof(char), otLength, new_file_dec);
  }
  
  EVP_DecryptFinal(&ctx, ot, &otLength); 
  fwrite(ot, sizeof(char), otLength, new_file_dec);

  fclose(file);

  // CBC
  file = fopen("Mad_scientist.bmp", "r");
  new_file = fopen("Mad_scientist_cbc.bmp", "wa");
  new_file_dec = fopen("Mad_scientist_cbc_dec.bmp", "wa");

  otLength = sizeof(ot);
  stLength = 0;

  fread(buffer, sizeof(char), 54, file);
  fwrite(buffer, sizeof(char), 54, new_file);

  EVP_EncryptInit(&ctx, EVP_des_cbc(), key, iv);  

  while ( fread(ot, sizeof(char), otLength, file) != 0 ) {
    EVP_EncryptUpdate(&ctx, st, &stLength, ot, otLength);  
    fwrite(st, sizeof(char), stLength, new_file);
  }

  EVP_EncryptFinal(&ctx, st, &stLength); 
  fwrite(st, sizeof(char), stLength, new_file);

  fclose(new_file);

  // CBC decypher
  new_file = fopen("Mad_scientist_cbc.bmp", "r");

  otLength = 0;
  stLength = sizeof(st);

  fread(buffer, sizeof(char), 54, new_file);
  fwrite(buffer, sizeof(char), 54, new_file_dec);
     
  EVP_DecryptInit(&ctx, EVP_des_cbc(), key, iv);

  while ( fread(st, sizeof(char), stLength, new_file) != 0 ) {
    EVP_DecryptUpdate(&ctx, ot, &otLength,  st, stLength);  
    fwrite(ot, sizeof(char), otLength, new_file_dec);
  }
  
  EVP_DecryptFinal(&ctx, ot, &otLength); 
  fwrite(ot, sizeof(char), otLength, new_file_dec);
 
  exit(0);
}