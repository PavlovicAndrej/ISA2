/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Base16 encoding and decoding.
 */

#define MASK 0b00001111

#include <stdlib.h>

void b16_encode(char *dst, char const *src, size_t n) {
    for (int i = 0; i < n*2; i++)
        *(dst+i) = (i%2 ? *(src+i/2)&MASK : (unsigned char)*(src+i/2)>>4)+'A';
}

void b16_decode(char *dst, char const *src, size_t n) {
    if (n % 2)
        n--;
    for (int i = 0; i < n/2; i++)
        *(dst+i) = (*(src+i*2)-'A')<<4 | *(src+i*2+1)-'A';
}