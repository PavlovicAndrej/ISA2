/**
 * @Author Andrej Pavlovič
 * @Email <xpavlo14@vutbr.cz>
 * @Project DNS Tunneling
 * @Program Base16 encoding and decoding.
 * @details header file
 */

// GUARD
#ifndef B16_H
#define B16_H

#include <stdlib.h>

/**
 * Function encodes 'n' bytes of character array pointed by 'src' into character array pointed by 'dst' with base16
 * (Hex) encoding. Used characters to encode are 'A'-'P'.
 *
 * Please note, that this encoding causes data size overhead of 100%, hence there will be 2*'n' bytes saved into 'dst'.
 * Therefor if used as pair function with 'b16_decode()' to same buffer length, this function should be passed two times
 * smaller 'n', as 'b16_decode()' was passed.
 *
 * @param dst Pointer pointing to character array into which encoded array will be saved.
 * @param src Pointer pointing to character array to be encoded.
 * @param n Number of 'src' bytes to be encoded (buffer size).
 */
void b16_encode(char *dst, char const *src, size_t n);

/**
 * Function decodes 'n' bytes of character array pointed by 'src' into character array pointed by 'dst' with base16
 * (Hex) decoding. 'src' array is expected to be encoded with 'A'-'P' character set.
 *
 * Please note, that this decoding causes data size shrink of 100%, hence there will be 'n'/2 bytes saved into 'dst'.
 * Therefor if used as pair function with 'b16_encode()' to same buffer length, this function should be passed two times
 * larger 'n', as 'b16_encode()' was passed.
 *
 * If 'n' is odd, last byte of 'src' (src[n-1]) is ignored and not processed.
 *
 * 'dst' and 'src' can overlap on physical memory IF 'src' >= 'dst' - 1
 *
 * @param dst Pointer pointing to character array into which decoded array will be saved.
 * @param src Pointer pointing to character array to be decoded.
 * @param n Number of 'src' bytes to be decoded (buffer size).
 */
void b16_decode(char *dst, char const *src, size_t n);

// END GUARD
#endif