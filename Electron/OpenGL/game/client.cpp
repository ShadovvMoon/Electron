//
//  client.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "client.h"
#include <thread>

void show_dump(unsigned char *data, long len, FILE *stream) {
    const static char       hex[] = "0123456789abcdef";
    static unsigned char    buff[67];   /* HEX  CHAR\n */
    unsigned char           chr,
    *bytes,
    *p,
    *limit,
    *glimit = data + len;
    
    memset(buff + 2, ' ', 48);
    
    while(data < glimit) {
        limit = data + 16;
        if(limit > glimit) {
            limit = glimit;
            memset(buff, ' ', 48);
        }
        
        p     = buff;
        bytes = p + 50;
        while(data < limit) {
            chr = *data;
            *p++ = hex[chr >> 4];
            *p++ = hex[chr & 15];
            p++;
            *bytes++ = ((chr < ' ') || (chr >= 0x7f)) ? '.' : chr;
            data++;
        }
        *bytes++ = '\n';
        
        fwrite(buff, bytes - buff, 1, stream);
    }
}

// Helper functions
uint32_t resolv(const char *host) {
    struct  hostent *hp;
    uint32_t     host_ip;
    
    host_ip = inet_addr(host);
    if(host_ip == INADDR_NONE) {
        hp = gethostbyname(host);
        if(!hp) {
            printf("\nError: Unable to resolv hostname (%s)\n", host);
            exit(1);
        } else host_ip = *(uint32_t *)(hp->h_addr);
    }
    return(host_ip);
}

unsigned char *gssdkcr(
                       unsigned char *dst,
                       unsigned char *src,
                       unsigned char *key) {
    
    unsigned long    oz,
    i,
    keysz,
    count,
    old,
    tmp,
    randnum;
    unsigned char   *ptr;
    const static char
    key_default[] =
    "3b8dd8995f7c40a9a5c5b7dd5b481341";
    
    randnum = (unsigned int)time(NULL);   // something random
    if(!key) key = (unsigned char *)key_default;
    keysz = strlen((char*)key);
    
    ptr = src;
    old = *ptr;
    tmp = old < 0x4f;
    count = 0;
    for(oz = i = 1; i < 32; i++) {
        count ^= ((((*ptr < old) ^ ((old ^ i) & 1)) ^ (*ptr & 1)) ^ tmp);
        ptr++;
        if(count) {
            if(!(*ptr & 1)) { oz = 0; break; }
        } else {
            if(*ptr & 1) { oz = 0; break; }
        }
    }
    
    ptr = dst;
    for(i = 0; i < 32; i++, ptr++) {
        if(!oz || !i || (i == 13)) {
            randnum = (randnum * 0x343FD) + 0x269EC3;
            *ptr = (((randnum >> 16) & 0x7fff) % 93) + 33;
            continue;
        } else if((i == 1) || (i == 14)) {
            old = src[i];
        } else {
            old = src[i - 1];
        }
        tmp = (old * i) * 17991;
        old = src[(key[(src[i] + i) % keysz] + (src[i] * i)) & 31];
        *ptr = ((old ^ key[tmp % keysz]) % 93) + 33;
    }
    *ptr = 0;
    
    return(dst);
}

int putcc(uint8_t *buff, int chr, int len) {
    memset(buff, chr, len);
    return(len);
}

int putmm(uint8_t *buff, uint8_t *data, int len) {
    memcpy(buff, data, len);
    return(len);
}

int putxx(uint8_t *data, uint32_t num, int bits) {
    int i,
    bytes;
    
    bytes = bits >> 3;
    for(i = 0; i < bytes; i++) {
        data[i] = (num >> (i << 3)) & 0xff;
    }
    return(bytes);
}

int timeout(int sock, int secs) {
    struct  timeval tout;
    fd_set  fd_read;
    
    tout.tv_sec  = secs;
    tout.tv_usec = 0;
    FD_ZERO(&fd_read);
    FD_SET(sock, &fd_read);
    if(select(sock + 1, &fd_read, NULL, NULL, &tout)
       <= 0) return(-1);
    return(0);
}

long send_recv(int sd, uint8_t *in, int insz, uint8_t *out, int outsz, struct sockaddr_in *peer, int err) {
    long retry = 2, len;
    
    if(in) {
        while(retry--) {
            //fputc('.', stdout);
            if(sendto(sd, in, insz, 0, (struct sockaddr *)peer, sizeof(struct sockaddr_in))
               < 0) goto quit;
            if(!out) return(0);
            if(!timeout(sd, 2)) break;
        }
    } else {
        if(timeout(sd, 3) < 0) retry = -1;
    }
    
    if(retry < 0) {
        if(!err) return(-1);
        printf("\nError: socket timeout, no reply received\n\n");
        return -1;
    }
    
    //fputc('.', stdout);
    len = recvfrom(sd, out, outsz, 0, NULL, NULL);
    if(len < 0) goto quit;
    return(len);
quit:
    if(err) {
        fprintf(stderr, "An error occured\n");
        exit(2);
    }
    return(-1);
}

void halo_create_randhash(uint8_t *out) {
    uint32_t            randnum;
    int                 i;
    const static char   *hex = (char*)"0123456789ABCDEF";
    
    
    randnum = (uint32_t)time(0);
    for(i = 0; i < 16; i++) {
        randnum = (randnum * 0x343FD) + 0x269EC3;
        *out++ = hex[(randnum >> 16) & 15];
    }
    *out = 0;
}

void halo_byte2hex(uint8_t *in, uint8_t *out) {
    int                 i;
    const static char   *hex = (char*)"0123456789ABCDEF";
    
    for(i = 16; i; i--) {
        if(*in) break;
        in++;
    }
    while(i--) {
        *out++ = hex[*in >> 4];
        *out++ = hex[*in & 15];
        in++;
    }
    *out = 0;
}

void halo_hex2byte(uint8_t *in, uint8_t *out) {
    int     i,
    j,
    t;
    
    memset(out, 0, 16);
    while(*in) {
        for(j = 0; j < 4; j++) {
            t = 0;
            for(i = 15; i >= 0; i--) {
                t += (out[i] << 1);
                out[i] = t;
                t >>= 8;
            }
        }
        t = *in |= 0x20;
        out[15] |= ((t - (0x27 * (t > 0x60))) - 0x30);
        in++;
    }
}

void halo_fix_check(uint8_t *key1, uint8_t *key2) {
    int     i,
    j;
    
    for(i = 0; i < 16; i++) {
        if(key1[i] != key2[i]) break;
    }
    if((i < 16) && (key1[i] > key2[i])) {
        for(j = 0, i = 16; i--; j >>= 8) {
            j += (key1[i] - key2[i]);
            key1[i] = j;
        }
    }
}

void halo_key_scramble(uint8_t *key1, uint8_t *key2, uint8_t *fixnumb) {
    int     i,
    j,
    cnt;
    uint8_t tk1[16],
    tk2[16];
    
    memcpy(tk1,  key1, 16);
    memcpy(tk2,  key2, 16);
    memset(key1, 0,    16);
    
    cnt = 16 << 3;
    while(cnt--) {
        if(tk1[15] & 1) {
            for(j = 0, i = 16; i--; j >>= 8) {
                j += key1[i] + tk2[i];
                key1[i] = j;
            }
            halo_fix_check(key1, fixnumb);
        }
        
        for(j = i = 0; i < 16; i++, j <<= 8) {
            j |= tk1[i];
            tk1[i] = j >> 1;
            j &= 1;
        }
        
        for(j = 0, i = 16; i--; j >>= 8) {
            j += (tk2[i] << 1);
            tk2[i] = j;
        }
        halo_fix_check(tk2, fixnumb);
    }
}

void halo_create_key(uint8_t *keystr, uint8_t *randhash, uint8_t *fixnum, uint8_t *dest) {
    int     i,
    j,
    cnt;
    uint8_t keystrb[16],
    randhashb[16],
    fixnumb[16];
    
    halo_hex2byte(keystr,   keystrb);
    halo_hex2byte(randhash, randhashb);
    halo_hex2byte(fixnum,   fixnumb);
    
    memset(dest, 0, 16);
    dest[15] = 0x01;
    
    cnt = 16 << 3;
    while(cnt--) {
        if(randhashb[15] & 1) {
            halo_key_scramble(dest, keystrb, fixnumb);
        }
        halo_key_scramble(keystrb, keystrb, fixnumb);
        
        for(j = i = 0; i < 16; i++, j <<= 8) {
            j |= randhashb[i];
            randhashb[i] = j >> 1;
            j &= 1;
        }
    }
}

void halo_generate_keys(uint8_t *hash, uint8_t *source_key, uint8_t *dest_key) {
    uint8_t tmp_key[33],
    fixed_key[33];
    
    strcpy((char*)fixed_key, "10001"); // key 1
    
    if(!source_key)
    {           // encryption
        strcpy((char*)tmp_key, "3");   // key 2
        halo_create_randhash(hash);
    } else {
        halo_byte2hex(source_key, tmp_key);
    }
    
    source_key = tmp_key;
    halo_create_key(source_key, hash, fixed_key, dest_key);
}

int hex_to_int(char c){
    if(c >=97)
        c=c-32;
    int first = c / 16 - 3;
    int second = c % 16;
    int result = first*10 + second;
    if(result > 9) result--;
    return result;
}

int hex_to_ascii(char c, char d){
    int high = hex_to_int(c) * 16;
    int low = hex_to_int(d);
    return high+low;
}


static unsigned char gethex(const char *s, char **endptr) {
    while (isspace(*s)) s++;
    return strtoul(s, endptr, 16);
}

unsigned char *convert(const char *s, long *length) {
    unsigned char *answer = (unsigned char *)malloc((strlen(s) + 1) / 3);
    unsigned char *p;
    for (p = answer; *s; p++)
        *p = gethex(s, (char **)&s);
    *length = p - answer;
    return answer;
}

char *gskeychall(char *cdkey, char *stoken, int ctoken) {
    static unsigned char     chall[73];
    char            *tmp;
    unsigned char   md5h[16],
    *ptr;
    const char      *hex = "0123456789abcdef";
    md5_context     md5t;
    long             i,
    tmplen;
    
#define DOMD5(x,y) \
md5_starts(&md5t); \
md5_update(&md5t, x, y); \
md5_finish(&md5t, md5h);
    
    
    if(!ctoken)
    {
        //srand(time(NULL));
        ctoken = (arc4random() << 16) ^ arc4random();
    }
    ctoken = abs(ctoken);   // needed, positive integer
    
    /* 1) CDKEY HASH */
    DOMD5((unsigned char*)cdkey, (size_t)strlen(cdkey));
    for(ptr = chall, i = 0; i < 16; i++) {
        *ptr++ = hex[md5h[i] >> 4];
        *ptr++ = hex[md5h[i] & 0xf];
    }
    
    /* 2) CLIENT TOKEN */
    sprintf(
            (char*)ptr,
            (char*)"%.8x",
            ctoken);
    
    printf("\nCLIENT TOKEN %s\n", ptr);
    
    /* 3) THIRD STRING */
    tmplen = strlen(cdkey) + 5 + strlen(stoken);
    tmp = (char*)alloca(tmplen + 1);   // auto-free
    if(!tmp) return((char*)"");
    
    i = sprintf(
                tmp,
                "%s%d%s",
                cdkey,
                ctoken % 0xffff,
                stoken);
    DOMD5((unsigned char*)tmp, (size_t)i);
    for(ptr += 8, i = 0; i < 16; i++) {
        *ptr++ = hex[md5h[i] >> 4];
        *ptr++ = hex[md5h[i] & 0xf];
    }
    
    *ptr = 0;
    return (char*)(chall);
}

unsigned int read_bits(    // number read
                       unsigned int bits,       // how much bits to read
                       unsigned char *in,       // buffer from which to read the number
                       unsigned int in_bits     // position of the buffer in bits
) {
    unsigned int    seek_bits,
    rem,
    seek = 0,
    ret  = 0,
    mask = 0xffffffff;
    
    if(bits > 32) return(0);
    if(bits < 32) mask = (1 << bits) - 1;
    for(;;) {
        seek_bits = in_bits & 7;
        ret |= ((in[in_bits >> 3] >> seek_bits) & mask) << seek;
        rem = 8 - seek_bits;
        if(rem >= bits) break;
        bits    -= rem;
        in_bits += rem;
        seek    += rem;
        mask     = (1 << bits) - 1;
    }
    return(ret);
}



unsigned int write_bits(   // position where the stored number finishs
                        unsigned int data,       // number to store
                        unsigned int bits,       // how much bits to occupy
                        unsigned char *out,      // buffer on which to store the number
                        unsigned int out_bits    // position of the buffer in bits
) {
    unsigned int    seek_bits,
    rem,
    mask;
    
    if(bits > 32) return(out_bits);
    if(bits < 32) data &= (1 << bits) - 1;
    for(;;) {
        seek_bits = out_bits & 7;
        mask = (1 << seek_bits) - 1;
        if((bits + seek_bits) < 8) mask |= ~(((1 << bits) << seek_bits) - 1);
        out[out_bits >> 3] &= mask; // zero
        out[out_bits >> 3] |= (data << seek_bits);
        rem = 8 - seek_bits;
        if(rem >= bits) break;
        out_bits += rem;
        bits     -= rem;
        data    >>= rem;
    }
    return(out_bits + bits);
}

int write_bstr(uint8_t *data, uint32_t len, uint8_t *buff, uint32_t bitslen) {
    int     i;
    
    for(i = 0; i < len; i++)
    {
        bitslen = write_bits(data[i], 8,  buff, bitslen);
    }
    return(bitslen);
}

int read_bstr(uint8_t *data, uint32_t len, uint8_t *buff, uint32_t bitslen) {
    int     i;
    
    for(i = 0; i < len; i++) {
        data[i] = read_bits(8, buff, bitslen);
        bitslen += 8;
    }
    return(bitslen);
}

uint32_t halo_crc32(uint8_t *data, int size) {
    const static uint32_t   crctable[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
        0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
        0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
        0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
        0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
        0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
        0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
        0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
        0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
        0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
        0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
        0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
        0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
        0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
        0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
        0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };
    uint32_t    crc = 0xffffffff;
    
    while(size--) {
        crc = crctable[(*data ^ crc) & 0xff] ^ (crc >> 8);
        data++;
    }
    return(crc);
}

void tea_decrypt(uint32_t *p, uint32_t *keyl) {
    uint32_t    y,
    z,
    sum,
    a = keyl[0],
    b = keyl[1],
    c = keyl[2],
    d = keyl[3];
    int         i;
    
    y = p[0];
    z = p[1];
    sum = 0xc6ef3720;
    for(i = 0; i < 32; i++) {
        z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
        y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        sum -= 0x9e3779b9;
    }
    p[0] = y;
    p[1] = z;
}



void halo_tea_decrypt(uint8_t *data, int size, uint8_t *key) {
    uint32_t    *p    = (uint32_t *)data,
    *keyl = (uint32_t *)key;
    
    if(size & 7) {
        tea_decrypt((uint32_t *)(data + size - 8), keyl);
    }
    
    size >>= 3;
    while(size--) {
        tea_decrypt(p, keyl);
        p += 2;
    }
}

void tea_encrypt(uint32_t *p, uint32_t *keyl) {
    uint32_t    y,
    z,
    sum,
    a = keyl[0],
    b = keyl[1],
    c = keyl[2],
    d = keyl[3];
    int         i;
    
    y = p[0];
    z = p[1];
    sum = 0;
    for(i = 0; i < 32; i++) {
        sum += 0x9e3779b9;
        y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
    }
    p[0] = y;
    p[1] = z;
}

void halo_tea_encrypt(uint8_t *data, int size, uint8_t *key) {
    uint32_t    *p    = (uint32_t *)data,
    *keyl = (uint32_t *)key;
    int         rest  = size & 7;
    
    size >>= 3;
    while(size--) {
        tea_encrypt(p, keyl);
        p += 2;
    }
    
    if(rest) {
        tea_encrypt((uint32_t *)((uint8_t *)p - (8 - rest)), keyl);
    }
}






bool Client::handshake() {
    fprintf(stdout, "Starting handshake...\n");
    
    // Assemble the server challenge
    uint8_t *psdk = buffer+7;
    uint8_t *p = buffer;
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 1,        8);
    p += putxx(p, htons(0), 16);
    p += putxx(p, htons(0), 16);
    memset(psdk, '1',       32);
    gssdkcr(psdk, psdk,     0);
    p += 32;
    
    // Send the challenge to the server
    fprintf(stdout, "Sending client keys...\n");

    long len = send_recv(sock, buffer, (int)(p - buffer), buffer, BUFFER_SIZE, &host, 1);
    while(buffer[2] != 2)
    {
        len = send_recv(sock, NULL, 0, buffer, BUFFER_SIZE, &host, 1);
    }
    fprintf(stdout, "Received server keys...\n");

    
    // Reply to the challenge
    p = buffer;
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 3,        8);
    p += putxx(p, htons(1), 16);
    p += putxx(p, htons(1), 16);
    
    // Generate my client keys
    gssdkcr(psdk, buffer + 39, 0); p += 32;
    uint8_t hash[17];
    halo_generate_keys(hash, NULL, enckey1);
    p += putmm(p, enckey1,   16);    // Halo handshake
    p += putxx(p, (int)HALO_VER,   32);
    
    // Send the reply to the server
    fprintf(stdout, "Sending secondary client keys...\n");
    len = send_recv(sock, buffer, (int)(p - buffer), buffer, BUFFER_SIZE, &host, 1);
    while((buffer[2] != 4) && (buffer[2] != 5) && (buffer[2] != 0x68))
    {
        len = send_recv(sock, NULL, 0, buffer, BUFFER_SIZE, &host, 1);
    }
    fprintf(stdout, "Received secondary server keys...\n");

    // Handle the reply
    if((buffer[2] == 5) && (buffer[7] == 6))
    {
        fprintf(stderr, "server full\n");
        return false;
    }
    else if((buffer[2] == 5) && (buffer[7] == 4))
    {
        fprintf(stderr, "unknown server version\n");
        return false;
    }
    else if((buffer[2] == 5) && (buffer[7] == 5))
    {
        fprintf(stderr, "unknown server version\n");
        return false;
    }
    else if(buffer[2] == 0x68)
    {
        fprintf(stderr, "disconnected\n");
        return false;
    }
    else if(buffer[2] != 4)
    {
        fprintf(stderr, "disconnected (unknown why)\n");
        return false;
    }
    
    // Generate the encryption keys
    halo_generate_keys(hash, psdk, deckey1);
    halo_generate_keys(hash, psdk, enckey1);
    
    // We are connected to the server
    fprintf(stdout, "connected\n");
    return true;
}

void Client::connect_packet() {
    
    // Generate a key
    char *name = (char*)this->name;
    char *pass = (char*)"lel";
    
    // Generate a random cd key
    char str[128];
    sprintf(str, "%d", arc4random()%1000000000);
    char *keyChallenge = gskeychall((char*)str, (char*)challenge, 0);
    long cdkeyLength = strlen(keyChallenge);
    
    // Assemble the packet
    uint8_t *string_buffer = (uint8_t*)malloc(BUFFER_SIZE);
    sprintf((char*)string_buffer, (char*)"0c 09 01 6d 65 73 73 61 67 65 20 69 6e 20 61 20 62 6f 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 32 30 39 37 30 36 61 64 32 62 39 38 63 30 37 38 64 66 61 39 34 64 66 33 63 33 38 65 35 31 64 39 61 66 66 38 35 34 31 30 62 33 31 32 36 37 64 31 30 30 63 62 61 30 35 30 61 34 62 30 64 64 38 35 33 35 39 37 66 38 34 00 04 00 52 00 4f 00 46 00 4c 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff ff ff %02x 00 ff ff 0e 98 20 02 c4 d8 de de c8 ce ea d8 c6 d0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2a 00 00", p_no);
    
    // Inject the custom information
    long length = 0;
    uint8_t *bb = convert((char*)string_buffer, &length);
    memcpy(bb+37, keyChallenge, cdkeyLength);
    
    uint8_t *k = bb+37+72+3;
    int ms;
    for (ms=0; ms < strlen(name); ms++)
    {
        memcpy(k, name+ms, 1);
        k+=2;
    }
    
    k = bb+20;
    for (ms=0; ms < strlen(pass); ms++)
    {
        memcpy(k, pass+ms, 1);
        k+=2;
    }
    
    // Convert the bits into the halo format
    uint8_t b = 0;
    uint8_t *p = buffer;
    uint8_t *psdk = buffer+7;
    b = write_bits((unsigned int)length, 11, (unsigned char*)psdk, b);
    b = write_bits(1, 1, (unsigned char*)psdk, b); // this part is not important, it's only needed to send a total of 4 bytes
    b = write_bstr(bb, ((unsigned int)length)*4, (unsigned char*)psdk, b); // this part is not important, it's only needed to send a total of 4 bytes
    p+=(length)+7;
    //free(string_buffer);
    
    // Fix up the header
    string_buffer = (uint8_t*)"fe fe 00 00 02 00 03 96 c1 90";
    bb = convert((char*)string_buffer, &length);
    memcpy(buffer, bb, length);
    
    // Generate checksum
    uint32_t crc = halo_crc32(psdk, (int)(p - psdk));
    p += putxx(p, crc, 32);
    
    // Encrypt the data
    halo_tea_encrypt(psdk, (int)(p - psdk), enckey1);
    
    // Send the data
    sendto(sock, buffer, p - buffer, 0, (struct sockaddr *)&host, sizeof(host));
}


#pragma pack(1)
typedef struct {
    uint16_t     sign;
    uint8_t      type;
    uint16_t     gs1;
    uint16_t     gs2;
} gh_t;
#pragma pack()

int haloreturnbits(uint8_t *buff,uint8_t *retbuff, int buffsz, bool output) {
    int     b,
    n,
    o;
    uint8_t      str[1 << 11];
    
    int offset = 0;
    
    buffsz -= 4;    // crc;
    if(buffsz <= 0) return -1;
    buffsz <<= 3;
    
    for(b = 0;;) {
        if((b + 11) > buffsz) break;
        n = read_bits(11, buff, b);     b += 11;
        
        if((b + 1) > buffsz) break;
        o = read_bits(1,  buff, b);     b += 1;
        
        if((b + n) > buffsz) break;
        b = read_bstr(str, n, buff, b);
        
        memcpy(retbuff+offset, str, n);
        offset+=n;
    }
    return offset;
}

int decreypt(uint8_t *buff, int len, uint8_t *deckey, uint8_t *enckey, bool output) {
    gh_t    *gh;
    int     head;
    
    head = 0;
    gh   = (gh_t *)buff;
    
    if(ntohs(gh->sign) == 0xfefd) { /* info */
        if (output)
            //show_dump(buff, len, stdout);
        return 0;
    }
    if(ntohs(gh->sign) == 0xfefe)
    {
        if(len <= 7)
        {
            if (output)
                //show_dump(buff, len, stdout);
            
            return 0;
        }
        head = 7;
    }
    
    halo_tea_decrypt(buff + head, len - head, deckey);
    
    //if(head) show_dump(buff, head, stdout);
    return haloreturnbits(buff + head, buff + head,len - head, output);
}

void Client::send_acknowledgement() {
    //fprintf(stdout, "sending acknowledgement... %d\n", client_position);
    
    // Attach the header
    uint8_t *p = buffer;
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 0x64,     8);
    p += putxx(p, htons(server_position), 16);
    
    // Debugging
    //show_dump(buffer, 5, stdout);
    
    // Send the packet
    sendto(sock, buffer, p - buffer, 0, (struct sockaddr *)&host, sizeof(host));
}

#include "math.h"
void Client::send_bit_packet(uint8_t *data, int b) {
    //printf("sending bits %d of data\n", b);
    memset(send_buffer, 0, BUFFER_SIZE);
    
    uint8_t *p = send_buffer;
    uint8_t *psdk = send_buffer + 7;
    
    // Apply header
    client_position++;
    
    // Add the header
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 0,         8);
    p += putxx(p, htons(client_position), 16);
    p += putxx(p, htons(server_position), 16);

    long bitlen = b + 12;
    long bytelen = (long)ceil((float)bitlen/8.0);
    int cb = 0;
    cb = write_bits((unsigned int)bytelen, 11, (unsigned char*)psdk, cb);
    cb = write_bits(1, 1, (unsigned char*)psdk, cb);
    int i;
    for (i=0; i < bytelen*8; i++) {
        cb = write_bits(read_bits(1, data, i), 1, (unsigned char*)psdk, cb);
    }
    
    //cb = write_bstr(data, (unsigned int), (unsigned char*)psdk, cb); // this part is not important, it's only needed to send a total of 4 bytes
    p += bytelen;
    
    // Generate checksum
    uint32_t crc = halo_crc32(psdk, (int)(p - psdk));
    p += putxx(p, crc, 32);
    
    // Debugging
    //show_dump(send_buffer, 7, stdout);
    //show_dump(psdk, p - psdk, stdout);
    
    // Encrypt the data
    halo_tea_encrypt(psdk, (int)(p - psdk), enckey1);
    
    // Send the data
    sendto(sock, send_buffer, p - send_buffer, 0, (struct sockaddr *)&host, sizeof(host));
}


void Client::send_packet(uint8_t *data, uint8_t length) {
    
    uint8_t *p = send_buffer;
    uint8_t *psdk = send_buffer + 7;
    
    // Apply header
    client_position++;
    
    // Add the header
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 0,     8);
    p += putxx(p, htons(client_position), 16);
    p += putxx(p, htons(server_position), 16);
    //p += putxx(p, 0, 8);
    //p += putxx(p, 4, 8);
    
    p += putxx(p, length+1, 8);         // size
    
    int i;
    for (i = 0; i < length; i++) {
        p += putxx(p, data[i], 8);
    }
    
    // Generate checksum
    uint32_t crc = halo_crc32(psdk, (int)(p - psdk));
    p += putxx(p, crc, 32);
    
    // Debugging
    //show_dump(send_buffer, 7, stdout);
    //show_dump(psdk, p - psdk, stdout);
    
    // Encrypt the data
    halo_tea_encrypt(psdk, (int)(p - psdk), enckey1);
    
    // Send the data
    sendto(sock, send_buffer, p - send_buffer, 0, (struct sockaddr *)&host, sizeof(host));
}

void Client::spawn() {
    fprintf(stdout, "spawning...\n");
    
    
    uint8_t *p = buffer;
    p += putxx(p, 0xc0, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x08, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x10, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0, 32);      // packet identifier (which function to send it to)
    p += putxx(p, 0xa0, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x01, 8);      // packet identifier (which function to send it to)
    send_packet(buffer, p - buffer);
    
    
    p = buffer;
    p += putxx(p, 0xb8, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x09, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x10, 8);      // packet identifier (which function to send it to)
    p += putxx(p, 0x04, 8);      // packet identifier (which function to send it to)
    send_packet(buffer, p - buffer);
}

void Client::join() {
    // Receive the challenge key and map name
    fprintf(stdout, "joining game...\n");
    long len = recv_bits();
    long n = decreypt(buffer, len, deckey1, enckey1, true);
    
    // Extract the positioning
    gh_t *gh = (gh_t *)buffer;
    if(ntohs(gh->sign) == 0xfefe) {
        server_position = ntohs(gh->gs1);
        client_position = ntohs(gh->gs2)-1;
    }
    
    // Extract the challenge
    challenge = (char*)malloc(8);
    memset(challenge, 0, 8);
    challenge[0] = buffer[10];
    challenge[1] = buffer[11];
    challenge[2] = buffer[12];
    challenge[3] = buffer[13];
    challenge[4] = buffer[14];
    challenge[5] = buffer[15];
    challenge[6] = buffer[16];
    p_no =   (int)(buffer[22]);
    
    // Send the machine connection
    connect_packet();
    
    // Receive the game state
    recv_bits();
    send_acknowledgement();
    
    // Join the game
    spawn();
}

void halobits(uint8_t *buff,int buffsz, bool output) {
    int     b,
    n,
    o;
    uint8_t      str[1 << 11];
    
    buffsz -= 4;    // crc;
    
    if(buffsz <= 0) return;
    buffsz <<= 3;
    
    
    for(b = 0;;)
    {
        if((b + 11) > buffsz) break;
        n = read_bits(11, buff, b);     b += 11;
        
        if((b + 1) > buffsz) break;
        o = read_bits(1,  buff, b);     b += 1;
        
        if((b + n) > buffsz) break;
        b = read_bstr(str, n, buff, b);
        
        //show_dump(str, n, stdout);
        
        
        if (n > 5 && str[0] == 0x1e)
        {
            //printf("%d\n", n);
            
            //return;
            
            
            //show_dump(str, n, stdout);
            
            
            
        }
    }
}

long Client::recv_bits() {
    //printf("receiving data from the server\n");
    long len = recvfrom(sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (len == -1) {
        printf("read failed\n");
        return len;
    }
    
    gh_t    *gh;
    int     head;
    head = 0;
    gh   = (gh_t *)buffer;
    
    if(ntohs(gh->sign) == 0xfefd) { /* info */
        //show_dump(buffer, len, stdout);
        return len;
    }
    if(ntohs(gh->sign) == 0xfefe) {
        if(len <= 7) {
            if (gh->type == 0x64 || gh->type == 0x65) {
                client_position = ntohs(gh->gs1);
                client_position--;
            }
            
            //show_dump(buffer, len, stdout);
            return len;
        }
        head = 7;
        server_position = ntohs(gh->gs1)+1;
        client_position = ntohs(gh->gs2)-1;
    }

    //show_dump(buffer, head, stdout);
    //halo_tea_decrypt(buffer + head, len - head, deckey1);
    //halobits(buffer + head, len - head, true);
    //halo_tea_encrypt(buffer + head, len - head, enckey1);
    return len;
}

#include <math.h>
int sread_bit(uint8_t *p, int *o) {
    int result = (p[(*o)/8] & (1 << (7-((*o)%8)))) >> (7-((*o)%8));
    (*o)++;
    return result;
}
int sread_bits(uint8_t *p, int *o, int n) {
    int num = 0;
    for (int i=n-1; i >= 0; i--) {
        int bit = read_bits(1, p, *o);
        (*o)++;
        num |= bit << ((n-1)-i); //pow(2,(n-1)-i) * bit;
    }
    return num;
    
    /*
    int num = 0;
    for (int i=0; i < n; i++) {
        int bit = sread_bit(p, o);
        printf("%d", bit);
        num += pow(2,(n-1)-i) * bit;
    }
    printf(" ");
    return num;
    */
}

enum network_messsage_type
{
    _network_messsage_type_message, // 'old' packets
    _network_messsage_type_message_delta,
    
    _network_messsage_type,
};

enum message_delta
{
    _message_delta_object_deletion,
    _message_delta_projectile_update,
    _message_delta_equipment_update,
    _message_delta_weapon_update,
    _message_delta_biped_update,
    _message_delta_vehicle_update,
    _message_delta_hud_add_item,
    _message_delta_player_create, //111000 =
    _message_delta_player_spawn, // probably contains team change? (000100)
    /*
     
     
     
     */
    _message_delta_player_exit_vehicle,
    _message_delta_player_set_action_result,
    _message_delta_player_effect_start,
    _message_delta_unit_kill,
    _message_delta_client_game_update,                      // client
    /*
     [  1]
     [  1] set jump bit
     [  1] set x-turn bit
     [  1] set y-turn bit
     [  1] set movement bit
     [  1] set shooting bit
     [  1] set weapon switching bit
     [  1]
     [  1]
     [  6] tick (0-63)

     // JUMP FIELD (10 bits)
     [  1] crouching
     [  1] jumping
     [  1] flashlight
     [  1] action
     [  1] melee
     [  1]
     [  1] shooting
     [  1]
     [  1]
     [  1] action long
     
     // X-TURN FIELD (32 bits)
     [ 32] x-turn
     
     // Y-TURN FIELD (32 bits)
     [ 32] y-turn
     
     // MOVEMENT FIELD (4 bits)
     [  1] right
     [  1] left
     [  1] backwards
     [  1] forwards
     */
    _message_delta_player_handle_powerup,
    _message_delta_hud_chat, // complete
    _message_delta_slayer_update,
    _message_delta_ctf_update,
    _message_delta_oddball_update,
    _message_delta_king_update,
    _message_delta_race_update,
    _message_delta_player_score_update,
    _message_delta_game_engine_change_mode,
    _message_delta_game_engine_map_reset,
    _message_delta_multiplayer_hud_message,
    _message_delta_multiplayer_sound,
    _message_delta_team_change, //client (0b011010 == 010110)
    /*
     [  8] player id
     [  8] team id
     */
    _message_delta_unit_drop_current_weapon,
    _message_delta_vehicle_new,
    _message_delta_biped_new, // server
    /*
     [ 16] biped tag_index
     [ 16] biped table_id
     ...
     */
    _message_delta_projectile_new,
    _message_delta_equipment_new, //server
    /*
     [  7]
     [  2]
     [ 16] eqip tag_index
     [ 16] eqip table_id
     [ 50]
     [ 32] x (float)
     [ 32] y (float)
     [ 32] z (float)
     */
    _message_delta_weapon_new, //server
    /*
     [ 16] weap tag_index
     [ 16] weap table_id
     [278] ?
     */
    _message_delta_game_settings_update,
    _message_delta_parameters_protocol,
    _message_delta_local_player_update, //server
    /*
     [ 11]
     [ 32] x (float)
     [ 32] y (float)
     [ 32] z (float)
     [  8]
     [ 11] bitmask
     ...
     */
    _message_delta_local_player_vehicle_update,
    _message_delta_remote_player_action_update,
    _message_delta_super_remote_players_action_update, //0110010
    
    _message_delta_remote_player_position_update,
    _message_delta_remote_player_vehicle_update,
    _message_delta_remote_player_total_update_biped, //server [no team]
    /*
     [  5] player number
     [ 10] 
     [  1] 
     [ 14] control mask?
     [ 15] rotation?
     [  4] weapon index
     [  1] nade index
     [ 25] X (int) * 6710.87548805
     [ 25] Y (int) * 6710.87548805
     [ 25] Z (int) * 6710.87548805
     */
    _message_delta_remote_player_total_update_vehicle,
    _message_delta_weapon_start_reload,
    _message_delta_weapon_ammo_pickup_mid_reload,
    _message_delta_weapon_finish_reload,
    _message_delta_weapon_cancel_reload,
    _message_delta_netgame_equipment_new,
    _message_delta_projectile_detonate,
    _message_delta_item_accelerate,
    _message_delta_damage_dealt,
    _message_delta_projectile_attach,
    _message_delta_client_to_server_pong,
    /*
     [  8] player number
     */
    _message_delta_super_ping_update,
    _message_delta_sv_motd,
    _message_delta_rcon_request,
    _message_delta_rcon_response,
    k_message_deltas_count
};

enum message {
    _message_client_broadcast_game_search,
    _message_client_ping,
    _message_server_game_advertise,
    _message_server_pong,
    _message_server_new_client_challenge,
    _message_server_machine_accepted,
    _message_server_machine_rejected,
    _message_server_game_is_ending_holdup,
    _message_server_game_settings_update,
    _message_server_pregame_countdown,
    _message_server_begin_game,
    _message_server_graceful_game_exit_pregame,
    _message_server_pregame_keep_alive,
    _message_server_postgame_keep_alive,
    _message_client_join_game_request,
    _message_client_add_player_request_pregame,
    _message_client_remove_player_request_pregame,
    _message_client_settings_request,
    _message_client_player_settings_request,
    _message_client_game_start_request,
    _message_client_graceful_game_exit_pregame,
    _message_client_map_is_precached_pregame,
    _message_server_game_update,
    _message_server_add_player_ingame,
    _message_server_remove_player_ingame,
    _message_server_game_over,
    _message_client_loaded,
    _message_client_game_update,
    _message_client_add_player_request_ingame,
    _message_client_remove_player_request_ingame,
    _message_client_graceful_game_exit_ingame,
    _message_client_host_crashed_cry_for_help,
    _message_client_join_new_host,
    _message_server_reconnect,
    _message_server_graceful_game_exit,
    _message_client_remove_player_request_postgame,
    _message_client_switch_to_pregame,
    _message_client_graceful_game_exit_postgame,
    k_message_type_count
};

enum field_type_bit_size
{
    _field_type_bit_size_real = 32,
    _field_type_bit_size_boolean = 1,
    _field_type_bit_size_ascii_character = 8,
    _field_type_bit_size_wide_character = 16,
    _field_type_bit_size_angle = 32,
    _field_type_bit_size_time = 32,
    _field_type_bit_size_grenade_counts = 6,
    _field_type_bit_size_digital_throttle = 4,
    _field_type_bit_size_fixed_width_weapon_index = 3,
    _field_type_bit_size_fixed_width_grenade_index = 2,
    _field_type_bit_size_item_placement_position = 20 /*X*/ + 20 /*Y*/ + 30 /*Z*/,
};

char *message_packet_to_string_table[] = {
    "client-broadcast_game_search",
    "client-ping",
    "server-game_advertise",
    "server-pong",
    "server-new_client_challenge",
    "server-machine_accepted",
    "server-machine_rejected",
    "server-game_is_ending_holdup",
    "server-game_settings_update",
    "server-pregame_countdown",
    "server-begin_game",
    "server-graceful_game_exit_pregame",
    "server-pregame_keep_alive",
    "server-postgame_keep_alive",
    "client-join_game_request",
    "client-add_player_request_pregame",
    "client-remove_player_request_pregame",
    "client-settings_request",
    "client-player_settings_request",
    "client-game_start_request",
    "client-graceful_game_exit_pregame",
    "client-map_is_precached_pregame",
    "server-game_update",
    "server-add_player_ingame",
    "server-remove_player_ingame",
    "server-game_over",
    "client-loaded",
    "client-game_update",
    "client-add_player_request_ingame",
    "client-remove_player_request_ingame",
    "client-graceful_game_exit_ingame",
    "client-host_crashed_cry_for_help",
    "client-join_new_host",
    "server-reconnect",
    "server-graceful_game_exit",
    "client-remove_player_request_postgame",
    "client-switch_to_pregame",
    "client-graceful_game_exit_postgame",
};

char *message_delta_packet_to_string_table[] = {
    "_message_delta_object_deletion",
    "_message_delta_projectile_update",
    "_message_delta_equipment_update",
    "_message_delta_weapon_update",
    "_message_delta_biped_update",
    "_message_delta_vehicle_update",
    "_message_delta_hud_add_item",
    "_message_delta_player_create",
    "_message_delta_player_spawn",
    "_message_delta_player_exit_vehicle",
    "_message_delta_player_set_action_result",
    "_message_delta_player_effect_start",
    "_message_delta_unit_kill",
    "_message_delta_client_game_update",
    "_message_delta_player_handle_powerup",
    "_message_delta_hud_chat",
    "_message_delta_slayer_update",
    "_message_delta_ctf_update",
    "_message_delta_oddball_update",
    "_message_delta_king_update",
    "_message_delta_race_update",
    "_message_delta_player_score_update",
    "_message_delta_game_engine_change_mode",
    "_message_delta_game_engine_map_reset",
    "_message_delta_multiplayer_hud_message",
    "_message_delta_multiplayer_sound",
    "_message_delta_team_change",
    "_message_delta_unit_drop_current_weapon",
    "_message_delta_vehicle_new",
    "_message_delta_biped_new",
    "_message_delta_projectile_new",
    "_message_delta_equipment_new",
    "_message_delta_weapon_new",
    "_message_delta_game_settings_update",
    "_message_delta_parameters_protocol",
    "_message_delta_local_player_update",
    "_message_delta_local_player_vehicle_update",
    "_message_delta_remote_player_action_update",
    "_message_delta_super_remote_players_action_update",
    "_message_delta_remote_player_position_update",
    "_message_delta_remote_player_vehicle_update",
    "_message_delta_remote_player_total_update_biped",
    "_message_delta_remote_player_total_update_vehicle",
    "_message_delta_weapon_start_reload",
    "_message_delta_weapon_ammo_pickup_mid_reload",
    "_message_delta_weapon_finish_reload",
    "_message_delta_weapon_cancel_reload",
    "_message_delta_netgame_equipment_new",
    "_message_delta_projectile_detonate",
    "_message_delta_item_accelerate",
    "_message_delta_damage_dealt",
    "_message_delta_projectile_attach",
    "_message_delta_client_to_server_pong",
    "_message_delta_super_ping_update",
    "_message_delta_sv_motd",
    "_message_delta_rcon_request",
    "_message_delta_rcon_response",
};

//_message_delta_client_game_update
/*
 11000000 00011101 10000000 00001000 00000001 10000000 11110111 11000000
 11000000 00011101 10000000 00000100 00110100 10001100 01101100 10100100
 11000000 00011101 10000000 00001100 00010010 10000100 10000001 00011100
 

 CLIENT 1 13 _message_delta_client_game_update
 10100000000 1 1 101100 100000000100000100000 00000100001100001010000111010110
 CLIENT 1 52 _message_delta_client_to_server_pong
 11000000000 1 1 001011 1000000000000 11010011110100101011000110000101
 CLIENT 0 13 _message_delta_client_game_update
 11000000000 1 1 101100 0000000001000 00000001100000001111011111000000

 [ 11 ] bytes            0
 [ 1  ]                 11
 [ 1  ] message mode    12
 [ 6  ] message type    13
 [ 1  ]                 19
 [ 1  ] jump            20
 [ 1  ] x-turn          21
 [ 1  ] y-turn          22
 [ 1  ] move            23
 [ 1  ] shooting        24
 [ 1  ] tab             25
 [ 1  ]                 26
 [ 1  ]                 27
 [ 6  ] tick            28
 
 // if jump, add 10 bit mask
 [ 1  ] crouching
 [ 1  ] jumping
 [ 1  ] flashlight
 [ 1  ] action
 [ 1  ] melee
 [ 1  ]
 [ 1  ] shooting
 [ 1  ]
 [ 1  ]
 [ 1  ] action long
 
 // if x turning, add 32 bit float
 // if y turning, add 32 bit float
 
 // if forward, add 4 bit mask
 [ 1  ] right
 [ 1  ] left
 [ 1  ] backwards
 [ 1  ] forward
 
 byte padding
 [ 32 ] crc32           40 [5 bytes ( + crc )]
 */
void Client::send_game_update() {
    Player *me = game->getPlayer(p_no);
    tick_counter ++;
    if (tick_counter > 63) tick_counter = 0;

    // Create a client update packet
    unsigned char *bb = (unsigned char*)build_buffer;
    memset(build_buffer, 0, BUFFER_SIZE);
    
    // delta header
    uint8_t b = 0;
    b = write_bits(1, 1, bb, b);                                 // message mode
    b = write_bits(_message_delta_client_game_update, 6, bb, b); // message type
    
    // control bitmask (a.k.a what fields have changed since the last tick)
    b = write_bits(0, 1, bb, b); //
    b = write_bits(1, 1, bb, b); // jump
    b = write_bits(1, 1, bb, b); // x-turn
    b = write_bits(1, 1, bb, b); // y-turn
    b = write_bits(1, 1, bb, b); // movement
    b = write_bits(0, 1, bb, b); // shooting
    b = write_bits(0, 1, bb, b); // switch weapons
    b = write_bits(0, 1, bb, b); //
    b = write_bits(0, 1, bb, b); //
    b = write_bits(tick_counter, 6, bb, b); // tick

    // jumping bitmask
    if (true) {
        b = write_bits(me->controls.jumping.crouch      ?1:0, 1, bb, b); // crouching
        b = write_bits(me->controls.jumping.jump        ?1:0, 1, bb, b); // jumping
        b = write_bits(me->controls.jumping.flash       ?1:0, 1, bb, b); // flashlight
        b = write_bits(me->controls.jumping.action      ?1:0, 1, bb, b); // action
        b = write_bits(me->controls.jumping.melee       ?1:0, 1, bb, b); // melee
        b = write_bits(0, 1, bb, b); //
        b = write_bits(me->controls.jumping.shoot       ?1:0, 1, bb, b); // shooting
        b = write_bits(0, 1, bb, b); //
        b = write_bits(0, 1, bb, b); //
        b = write_bits(me->controls.jumping.longact     ?1:0, 1, bb, b); // action long
    }
    
    // x turning
    if (true) {
        uint32_t bits = 0;
        memcpy(&bits, &me->controls.look_x, sizeof(float));
        b = write_bits(bits, 32, bb, b);
    }
    
    // y turning
    if (true) {
        uint32_t bits = 0;
        memcpy(&bits, &me->controls.look_y, sizeof(float));
        b = write_bits(bits, 32, bb, b);
    }
    
    // movement bitmask
    if (true) {
        b = write_bits(me->controls.movement.right      ?1:0, 1, bb, b);  // right
        b = write_bits(me->controls.movement.left       ?1:0, 1, bb, b);  // left
        b = write_bits(me->controls.movement.backwards  ?1:0, 1, bb, b);  // back
        b = write_bits(me->controls.movement.forwards   ?1:0, 1, bb, b);  // foward
    }
    
    // send the packet
    send_bit_packet(bb, b);
}

void Client::send_pong() {
 
    // Create a client pong packet
    unsigned char *bb = (unsigned char*)build_buffer;
    memset(build_buffer, 0, BUFFER_SIZE);
    
    // delta header
    uint8_t b = 0;
    int i;

    b = write_bits(1, 1, bb, b);                                    // message mode
    b = write_bits(_message_delta_client_to_server_pong, 6, bb, b); // message type
    
    // player id
    b = write_bits(1, 8, bb, b);

    
    // send the packet
    send_bit_packet(build_buffer, b);
}



//_message_delta_team_change
/*
 [ 11 ] bytes            0
 [ 1  ]
 [ 1  ] message mode
 [ 6  ] message type
 [ 8  ] player id (maybe only the first 4 bits?)
 [ 8  ] team id
 byte padding / unknown
 [ 32 ] crc32
 */










// New SkyBridge code
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdlib>

void Client::main() {
    Player *me = game->getPlayer(p_no);
    while (true) {
        long len = recv_bits();
       
        gh_t    *gh;
        int     head;
        head = 0;
        gh   = (gh_t *)buffer;
        
        if(ntohs(gh->sign) == 0xfefd) { /* info */
            //show_dump(buffer, len, stdout);
            continue;
        }
        if(ntohs(gh->sign) == 0xfefe) {
            if(len <= 7) {
                //show_dump(buffer, len, stdout);
                continue;
            }
            head = 7;
        }
        
        // Decrypt the data
        halo_tea_decrypt(buffer + head, (int)(len - head), deckey1);

        int packet_len = (int)(len - head);
        packet_len -= 4; //crc
        packet_len *= 8; // bit length
        
        uint8_t *buff = buffer+head;
        uint8_t str[1 << 11];
        int b, n, o;
        for(b = 0;;) {
            if((b + 11) > packet_len) break;
            n = read_bits(11, buff, b);     b += 11;
            if((b + 1) > packet_len) break;
            o = read_bits(1, buff, b);     b += 1;
            if((b + n) > packet_len) break;
            b = read_bstr(str, n, buff, b);
            
            // Handle this message
            // What type of message is this?
            int bit = 0;
            while (bit < n) {
                int message_mode = sread_bits(str, &bit, 1);
                int message_type = sread_bits(str, &bit, 6);
                
           
                //printf("%s\n", message_delta_packet_to_string_table[message_type]);
                int i;
                for (i=0; i < (len-head)*8; i++) {
                    printf("%d", read_bits(1, buffer+head, i));
                }
                printf("\n");
                
                //if (message_mode == 1) {
                    //printf("%s\n", message_delta_packet_to_string_table[message_type]);
                    // else {
                    //    printf("%s\n", message_delta_packet_to_string_table[message_type]);
                    //    int i;
                    //    for (i=0; i < (len-head)*8; i++) {
                    //        printf("%d", read_bits(1, buffer+head, i));
                    //    }
                    //    printf("\n");
                    //
                    //}
                    //break;
                //} else if (message_mode == 0) { // delta
                
                    
                    
                    
                    
                    if (message_type == _message_delta_super_remote_players_action_update) {
                        /*
                         int i;
                         for (i=0; i < (len-head)*8; i++) {
                         printf("%d", read_bits(1, buffer+head, i));
                         }
                         printf("\n");
                         */
                        int unknown0  = sread_bits(str, &bit, 7);
                        int movement  = sread_bits(str, &bit, 1);
                        int unknown1  = sread_bits(str, &bit, 15);
                        if (movement) {
                            int right     = sread_bits(str, &bit, 1);
                            int left      = sread_bits(str, &bit, 1);
                            int backwards = sread_bits(str, &bit, 1);
                            int forwards  = sread_bits(str, &bit, 1);
                            
                            //if (right)      printf("right\n");
                            //if (left)       printf("left\n");
                            //if (forwards)   printf("forwards\n");
                            //if (backwards)  printf("backwards\n");
                        }
                    } else if (message_type == _message_delta_object_deletion) {
                        int unknown0 = sread_bits(str, &bit, 9);
                        continue;
                    } else if (message_type == _message_delta_super_ping_update) {
                        int count = sread_bits(str, &bit, 4);
                        //printf("=== PING ===\n");
                        int i;
                        for (i=0; i < count; i++) {
                            int player   = sread_bits(str, &bit, 8);
                            int ping     = sread_bits(str, &bit, 32);
                            //printf("Player %d: %dms\n", player, ping);
                        }
                        break;
                    } else if (message_type == _message_delta_weapon_start_reload) {
                        int weapon = sread_bits(str, &bit, 7);
                        int player = sread_bits(str, &bit, 4);
                        int junk   = sread_bits(str, &bit, 14);
                        //printf("player %d started reloading %d\n", player, weapon);
                        continue;
                    } else if (message_type == _message_delta_weapon_cancel_reload) {
                        int weapon = sread_bits(str, &bit, 7);
                        int player = sread_bits(str, &bit, 4);
                        //int junk   = sread_bits(str, &bit, 14);
                        printf("player %d cancelled reloading %d\n", player, weapon);
                        continue;
                    } else if (message_type == _message_delta_weapon_finish_reload) {
                        int weapon = sread_bits(str, &bit, 7);
                        int player = sread_bits(str, &bit, 4);
                        int junk   = sread_bits(str, &bit, 14);
                        //printf("player %d finished reloading %d\n", player, weapon);
                        continue;
                    } else if (message_type == _message_delta_hud_chat) {
                        int player = sread_bits(str, &bit, 4);
                        int team   = sread_bits(str, &bit, 12);
                        int size   = sread_bits(str, &bit, 8);
                        printf("player %d said: ", player);
                        
                        int i;
                        for (i=0; i < size; i++) {
                            int byte   = sread_bits(str, &bit, 16);
                            printf("%c", byte);
                        }
                        printf("\n");
                        continue;
                    } else if (message_type == _message_delta_object_deletion) {
                        int unknown0 = sread_bits(str, &bit, 9);
                        continue;
                    } else if (message_type == _message_delta_netgame_equipment_new) {
                        int unknown0 = sread_bits(str, &bit, 27);
                        int unknown1 = sread_bits(str, &bit, 32);
                        int unknown2 = sread_bits(str, &bit, 32);
                        int x        = sread_bits(str, &bit, 32);
                        int y        = sread_bits(str, &bit, 32);
                        int z        = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        printf("new equipment at %f %f %f\n", xf, yf, zf);
                        continue;
                    } else if (message_type == _message_delta_weapon_new) {
                        int unknown0 = sread_bits(str, &bit, 32);
                        int unknown1 = sread_bits(str, &bit, 32);
                        int unknown2 = sread_bits(str, &bit, 32);
                        int unknown3 = sread_bits(str, &bit, 32);
                        int unknown4 = sread_bits(str, &bit, 32);
                        int unknown5 = sread_bits(str, &bit, 32);
                        int unknown6 = sread_bits(str, &bit, 32);
                        int unknown7 = sread_bits(str, &bit, 32);
                        int unknown8 = sread_bits(str, &bit, 32);
                        int unknown9 = sread_bits(str, &bit, 11);
                        continue;
                    } else if (message_type == _message_delta_local_player_update) {
                        int unknown0 = sread_bits(str, &bit, 11); //11010011110
                        uint32_t x   = sread_bits(str, &bit, 32);
                        uint32_t y   = sread_bits(str, &bit, 32);
                        uint32_t z   = sread_bits(str, &bit, 32);
                        int bitmask0 = sread_bits(str, &bit, 22);
                        int bitmask1 = sread_bits(str, &bit, 12);

                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        me->position->x = xf;
                        me->position->y = yf;
                        me->position->z = zf;
                        break;
                        //printf("I'm at %f %f %f\n", xf, yf, zf);
                    } else if (message_type == _message_delta_remote_player_total_update_biped) {
                        
                        int player_number       = sread_bits(str, &bit, 5)-1;
                        int unknown0            = sread_bits(str, &bit, 10);
                        int unknown1            = sread_bits(str, &bit, 1);
                        int controls            = sread_bits(str, &bit, 14);
                        
                        /*
                         MD SERVER
                        int angley              = sread_bits(str, &bit, 20);
                        int anglex              = sread_bits(str, &bit, 20);
                        int unknown2            = sread_bits(str, &bit, 5);
                        int weapon_index        = sread_bits(str, &bit, 3);
                        int nade_index          = sread_bits(str, &bit, 2);
                        int unknown3            = sread_bits(str, &bit, 7);
                        int x_raw               = sread_bits(str, &bit, 23);
                        int x_raw_sign          = sread_bits(str, &bit, 1);
                        int unknown4            = sread_bits(str, &bit, 7);
                        int y_raw               = sread_bits(str, &bit, 23);
                        int y_raw_sign          = sread_bits(str, &bit, 1);
                        int unknown5            = sread_bits(str, &bit, 7);
                        int z_raw               = sread_bits(str, &bit, 23);
                        int z_raw_sign          = sread_bits(str, &bit, 1);
                        printf("\n");
                        
                        // Parse the packet parameters
                        x_raw = x_raw_sign ? x_raw : -((~x_raw) & 0b00000000011111111111111111111111);
                        y_raw = y_raw_sign ? y_raw : -((~y_raw) & 0b00000000011111111111111111111111);
                        z_raw = z_raw_sign ? z_raw : -((~z_raw) & 0b00000000011111111111111111111111);
                        
                        float magic_float = 1677.717834;
                        float x = x_raw / magic_float;
                        float y = y_raw / magic_float;
                        float z = z_raw / magic_float;
                        */
                        
                        // SAPP SERVER
                        int angley              = sread_bits(str, &bit, 8);
                        int anglex              = sread_bits(str, &bit, 8);
                        int bitmask             = sread_bits(str, &bit, 4);
                        int weapon_index        = sread_bits(str, &bit, 3);
                        int nade_index          = sread_bits(str, &bit, 2);
                        int x_raw               = sread_bits(str, &bit, 24);
                        int x_raw_sign          = sread_bits(str, &bit, 1);
                        int y_raw               = sread_bits(str, &bit, 24);
                        int y_raw_sign          = sread_bits(str, &bit, 1);
                        int z_raw               = sread_bits(str, &bit, 24);
                        int z_raw_sign          = sread_bits(str, &bit, 1);
                       
                        // Parse the packet parameters
                        x_raw = !x_raw_sign ? x_raw : -(~x_raw & 0b111111111111111111111111);
                        y_raw = !y_raw_sign ? y_raw : -(~y_raw & 0b111111111111111111111111);
                        z_raw = !z_raw_sign ? z_raw : -(~z_raw & 0b111111111111111111111111);
                        
                        float magic_float = 6710.87548805;
                        float x = x_raw / magic_float;
                        float y = y_raw / magic_float;
                        float z = z_raw / magic_float;
                        
                        Player *p = game->getPlayer(player_number);
                        if (p != nullptr) {
                            p->position->x = x;
                            p->position->y = y;
                            p->position->z = z;
                            p->alive = true;
                        }
                        //printf("Player %d is now at %f %f %f and using weapon %d and nade %d\n", player_number, x, y, z, weapon_index, nade_index);
                        break;
                    } else {
                        
                        
                        break;
                    }

                break;
            }
        }
        
        // AI Tick update
        // Find the closest player
        int i; float minDist = 500.0; Player *closest = nullptr;
        for (i=0; i < kMaxPlayers; i++) {
            if (i == p_no) continue;
            
            Player *p = game->getPlayer(i);
            if (p != nullptr && p->alive) {
                float distance = p->position->distance(me->position);
                if (distance < minDist) {
                    minDist = distance;
                    closest = p;
                }
            }
        }
        if (closest) {
            float dx = closest->position->x - me->position->x;
            float dy = closest->position->y - me->position->y;
            
            float rx = 0.0;
            if (dx > 0 && dy > 0) {
                rx = atan2f(dy, dx);
            } else if (dx < 0 && dy > 0) {
                rx = M_PI - atan2f(dy, -dx);
            } else if (dx < 0 && dy < 0) {
                rx = M_PI + atan2f(-dy, -dx);
            } else if (dx > 0 && dy < 0) {
                rx = -atan2f(-dy, dx);
            }
            
            me->controls.look_x = rx;
            me->controls.movement.forwards = true;
            me->controls.jumping.crouch    = false;
            me->controls.jumping.shoot     = false; //minDist < 10;
        }
        
        
        uint16_t info = ntohs(((uint16_t*)(buffer+head))[0]);
        if (tick_counter == 0) {
            send_pong();
        }
        
        send_game_update();
        if (t == 0) {
            //control->left = !control->left;
            send_acknowledgement();
            
            //char *temp = (char *)malloc(BUFFER_SIZE);
            //sprintf(temp, "%d %d!", p, control->forward);
            //send_chat(temp);
            //free(temp);
            
            t = 10;
        }
        t--;
        usleep(100000 / 60);
        
        // Output the whole packet
        //show_dump(buffer, head, stdout);
        //show_dump(buffer + head, (int)(len - head), stdout);
        
    }
}

void Client::start() {
    // Perform the handshake
    if (!handshake()) {
        close(sock);
        return;
    }
    
    // Perform the join game
    join();
    
    // Start the game
    main();
}

Client::Client(const char *name, const char *ip, short port) {
    
    // Setup general structures
    memcpy(this->name, name, kPlayerNameLength);
    
    // Setup controls
    printf("setting up controls\n");
    game = new Game();
    
    // Create the buffer
    buffer          = (uint8_t*)malloc(BUFFER_SIZE);
    send_buffer     = (uint8_t*)malloc(BUFFER_SIZE);
    build_buffer    = (uint8_t*)malloc(BUFFER_SIZE);
    
    // Create the address
    host.sin_addr.s_addr = resolv(ip);
    host.sin_port        = htons(port);
    host.sin_family      = AF_INET;
    
    // Create a socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0) {
        fprintf(stderr, "Error creating socket\n");
        exit(1);
    }
    
    // Connect
    std::thread(&Client::start, this).detach();
}


// Cleanup
Client::~Client() {
    printf("deleting client\n");
}