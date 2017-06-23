//
//  client.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "client.h"
#include <thread>
#include <cstdlib>
//#define VERBOSE 1

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


//39 59



bool Client::handshake() {
    fprintf(stdout, "Starting handshake...\n");
    
    // Assemble the server challenge
    uint8_t *psdk = buffer+7;
    uint8_t *p = buffer;
    p += putxx(p, 0xfefe,   16);    // gssdk_header
    p += putxx(p, 1,        8);
    p += putxx(p, htons(0), 16);
    p += putxx(p, htons(0), 16);
    
    int i;
    for (i=0; i < 32; i++) {
        psdk[i] = arc4random() % 128;
    }
    
    //memset(psdk, '1',       32);
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
    char *pass = (char*)"lelwut";
    
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
    //recv_bits();
    main();
    
    // handle dem bits
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
    _message_delta_object_deletion,     //0 [DONE]
    _message_delta_projectile_update,   //1
    _message_delta_equipment_update,    //2
    _message_delta_weapon_update,       //3
    _message_delta_biped_update,        //4 => 4?
    _message_delta_vehicle_update,      //5
    _message_delta_hud_add_item,        //6 [DONE]
    _message_delta_player_create,       //7 [DONE]
    _message_delta_player_spawn,        //8 [DONE] probably contains team change? (000100)
    /*
     
     
     
     */
    _message_delta_player_exit_vehicle, // 9
    _message_delta_player_set_action_result, //10
    _message_delta_player_effect_start, // 11
    _message_delta_unit_kill,           // 12
    _message_delta_client_game_update,  // 13
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
    _message_delta_player_handle_powerup,   //14
    _message_delta_hud_chat,                //15
    _message_delta_slayer_update,           //16
    _message_delta_ctf_update,              //17
    _message_delta_oddball_update,          //18
    _message_delta_king_update,             //19
    _message_delta_race_update,             //20
    _message_delta_player_score_update,     //21
    _message_delta_game_engine_change_mode, //22
    _message_delta_game_engine_map_reset,   //23
    _message_delta_multiplayer_hud_message, //24
    _message_delta_multiplayer_sound,       //25
    _message_delta_team_change,             //26 [DONE]
    /*
     [  8] player id
     [  8] team id
     */
    _message_delta_unit_drop_current_weapon,//27
    _message_delta_vehicle_new,             //28
    _message_delta_biped_new,               //29
    /*
     [ 16] biped tag_index
     [ 16] biped table_id
     ...
     */
    _message_delta_projectile_new,          //30
    _message_delta_equipment_new, //server  //31
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
    _message_delta_weapon_new, //server     //32
    /*
     [ 16] weap tag_index
     [ 16] weap table_id
     [278] ?
     */
    _message_delta_game_settings_update,    //33
    _message_delta_parameters_protocol,     //34
    _message_delta_local_player_update,     //35
    /*
     [ 11]
     [ 32] x (float)
     [ 32] y (float)
     [ 32] z (float)
     [  8]
     [ 11] bitmask
     ...
     */
    _message_delta_local_player_vehicle_update, //36
    _message_delta_remote_player_action_update, //37
    _message_delta_super_remote_players_action_update, //38 //0110010
    
    _message_delta_remote_player_position_update, //39
    _message_delta_remote_player_vehicle_update,  //40
    _message_delta_remote_player_total_update_biped, //41       //server [no team]
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
    _message_delta_remote_player_total_update_vehicle,  //42
    _message_delta_weapon_start_reload,                 //43
    _message_delta_weapon_ammo_pickup_mid_reload,       //44
    _message_delta_weapon_finish_reload,                //45
    _message_delta_weapon_cancel_reload,                //46
    _message_delta_netgame_equipment_new,               //47
    _message_delta_projectile_detonate,                 //48
    _message_delta_item_accelerate,                     //49
    _message_delta_damage_dealt,                        //50
    _message_delta_projectile_attach,                   //51
    _message_delta_client_to_server_pong,               //52
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

char *message_packet_to_string_table[] = { // 38 [loc_1bbcd2]
    (char *)"client-broadcast_game_search",
    (char *)"client-ping",
    (char *)"server-game_advertise",
    (char *)"server-pong",
    (char *)"server-new_client_challenge",
    (char *)"server-machine_accepted",
    (char *)"server-machine_rejected",
    (char *)"server-game_is_ending_holdup",
    (char *)"server-game_settings_update",
    (char *)"server-pregame_countdown",
    (char *)"server-begin_game",
    (char *)"server-graceful_game_exit_pregame",
    (char *)"server-pregame_keep_alive",
    (char *)"server-postgame_keep_alive",
    (char *)"client-join_game_request",             //<-- join [14]
    (char *)"client-add_player_request_pregame",
    (char *)"client-remove_player_request_pregame",
    (char *)"client-settings_request",
    (char *)"client-player_settings_request",
    (char *)"client-game_start_request",
    (char *)"client-graceful_game_exit_pregame",
    (char *)"client-map_is_precached_pregame",      //<--- join [21]
    (char *)"server-game_update",
    (char *)"server-add_player_ingame",
    (char *)"server-remove_player_ingame",
    (char *)"server-game_over",
    (char *)"client-loaded",                        //<--- join [26]
    (char *)"client-game_update",
    (char *)"client-add_player_request_ingame",
    (char *)"client-remove_player_request_ingame",
    (char *)"client-graceful_game_exit_ingame",
    (char *)"client-host_crashed_cry_for_help",
    (char *)"client-join_new_host",
    (char *)"server-reconnect",
    (char *)"server-graceful_game_exit",
    (char *)"client-remove_player_request_postgame",
    (char *)"client-switch_to_pregame",
    (char *)"client-graceful_game_exit_postgame",
};

//00060844         jmp        dword [ds:0x343648+eax*4]                                     ; switch statement with 6 cases

char *message_delta_packet_to_string_table[] = { //loc_1bcc21 [56]
    (char *)"_message_delta_object_deletion",       //000000
    (char *)"_message_delta_projectile_update",     //100000
    (char *)"_message_delta_equipment_update",      //010000
    (char *)"_message_delta_weapon_update",         //110000
    (char *)"_message_delta_biped_update",          //001000
    (char *)"_message_delta_vehicle_update",        //101000
    (char *)"_message_delta_hud_add_item",          //111000
    (char *)"_message_delta_player_create",         //000100
    (char *)"_message_delta_player_spawn",          //100100
    (char *)"_message_delta_player_exit_vehicle",   //010100
    (char *)"_message_delta_player_set_action_result",  //110100
    (char *)"_message_delta_player_effect_start",   //001100
    (char *)"_message_delta_unit_kill",             //101100
    (char *)"_message_delta_client_game_update",    //011100
    (char *)"_message_delta_player_handle_powerup", //111100
    (char *)"_message_delta_hud_chat",              //000010
    (char *)"_message_delta_slayer_update",
    (char *)"_message_delta_ctf_update",
    (char *)"_message_delta_oddball_update",
    (char *)"_message_delta_king_update",
    (char *)"_message_delta_race_update",
    (char *)"_message_delta_player_score_update",
    (char *)"_message_delta_game_engine_change_mode",
    (char *)"_message_delta_game_engine_map_reset",
    (char *)"_message_delta_multiplayer_hud_message",
    (char *)"_message_delta_multiplayer_sound",
    (char *)"_message_delta_team_change",           //001011
    (char *)"_message_delta_unit_drop_current_weapon",
    (char *)"_message_delta_vehicle_new",
    (char *)"_message_delta_biped_new",
    (char *)"_message_delta_projectile_new",
    (char *)"_message_delta_equipment_new",
    (char *)"_message_delta_weapon_new",
    (char *)"_message_delta_game_settings_update",
    (char *)"_message_delta_parameters_protocol",
    (char *)"_message_delta_local_player_update",
    (char *)"_message_delta_local_player_vehicle_update",
    (char *)"_message_delta_remote_player_action_update",
    (char *)"_message_delta_super_remote_players_action_update",
    (char *)"_message_delta_remote_player_position_update",
    (char *)"_message_delta_remote_player_vehicle_update",
    (char *)"_message_delta_remote_player_total_update_biped",
    (char *)"_message_delta_remote_player_total_update_vehicle",
    (char *)"_message_delta_weapon_start_reload",
    (char *)"_message_delta_weapon_ammo_pickup_mid_reload",
    (char *)"_message_delta_weapon_finish_reload",
    (char *)"_message_delta_weapon_cancel_reload",
    (char *)"_message_delta_netgame_equipment_new",
    (char *)"_message_delta_projectile_detonate",
    (char *)"_message_delta_item_accelerate",
    (char *)"_message_delta_damage_dealt",
    (char *)"_message_delta_projectile_attach",
    (char *)"_message_delta_client_to_server_pong",
    (char *)"_message_delta_super_ping_update",
    (char *)"_message_delta_sv_motd",
    (char *)"_message_delta_rcon_request",
    (char *)"_message_delta_rcon_response",
};


//11101
//10111

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
    Biped *mep = game->getBiped(0);

    if (mep == nullptr) {
        mep = game->getBiped(0);
    }
    
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
        b = write_bits(mep->controls.jumping.crouch      ?1:0, 1, bb, b); // crouching
        b = write_bits(mep->controls.jumping.jump        ?1:0, 1, bb, b); // jumping
        b = write_bits(mep->controls.jumping.flash       ?1:0, 1, bb, b); // flashlight
        b = write_bits(mep->controls.jumping.action      ?1:0, 1, bb, b); // action
        b = write_bits(mep->controls.jumping.melee       ?1:0, 1, bb, b); // melee
        b = write_bits(0, 1, bb, b); //
        b = write_bits(mep->controls.jumping.shoot       ?1:0, 1, bb, b); // shooting
        b = write_bits(0, 1, bb, b); //
        b = write_bits(0, 1, bb, b); //
        b = write_bits(mep->controls.jumping.longact     ?1:0, 1, bb, b); // action long
    }
    
    // x turning
    if (true) {
        uint32_t bits = 0;
        memcpy(&bits, &mep->controls.look_x, sizeof(float));
        b = write_bits(bits, 32, bb, b);
    }
    
    // y turning
    if (true) {
        uint32_t bits = 0;
        memcpy(&bits, &mep->controls.look_y, sizeof(float));
        b = write_bits(bits, 32, bb, b);
    }
    
    // movement bitmask
    if (true) {
        b = write_bits(mep->controls.movement.right      ?1:0, 1, bb, b);  // right
        b = write_bits(mep->controls.movement.left       ?1:0, 1, bb, b);  // left
        b = write_bits(mep->controls.movement.backwards  ?1:0, 1, bb, b);  // back
        b = write_bits(mep->controls.movement.forwards   ?1:0, 1, bb, b);  // foward
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



char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
        
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}






// New SkyBridge code
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdlib>

int err_count = 0;
void Client::main() {
    Player *me = game->getPlayer(p_no);
    //while (true) {
        long len = recv_bits();
       
        gh_t    *gh;
        int     head;
        head = 0;
        gh   = (gh_t *)buffer;
        
        if(ntohs(gh->sign) == 0xfefd) { /* info */
            //show_dump(buffer, len, stdout);
            return;
        }
        if(ntohs(gh->sign) == 0xfefe) {
            if(len <= 7) {
                //show_dump(buffer, len, stdout);
                return;
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
            while (bit < n*8 && (n*8 - bit - 12) > 8) {
                int message_mode = sread_bits(str, &bit, 1);
                int message_type = sread_bits(str, &bit, 6);
                if (message_mode == 1) { // delta
                    
                    if (message_type == _message_delta_super_remote_players_action_update) {
                        
                        
                        /*
                         update_tick_count == 5
                         control_flags
                         fixed_width_normal_8bit
                         digital_throttle
                         fixed_width_1bit
                         fixed_width_weapon_index
                         fixed_width_grenade_index
                         */
                        
                        
                        mtx.lock();
                        for (int i = 1; i < kMaxBipeds; i++) {
                            Biped *bipd = game->getBiped(i);
                            bipd->dead_tick++;
                            if (bipd->dead_tick > 3) {
                                game->getBiped(i)->alive = false;
                            }
                        }
                        
                        int update_count = sread_bits(str, &bit, 4);
                        for (int i = 0; i <= update_count; i++) {
                            int unknown         = sread_bits(str, &bit, 1);
                            int jumping         = sread_bits(str, &bit, 1);
                            int turning         = sread_bits(str, &bit, 1);
                            int movement        = sread_bits(str, &bit, 1);
                            int shooting        = sread_bits(str, &bit, 1);
                            int u1              = sread_bits(str, &bit, 1);
                            int u2              = sread_bits(str, &bit, 1);
                            
                            //int player_number   = sread_bits(str, &bit, 5);
                            
                            
                            int player_number                           = sread_bits(str, &bit, 5);
                            int update_id                               = sread_bits(str, &bit, 6);
                            int remote_player_action_update_baseline_id = sread_bits(str, &bit, 1);
                            
                            //update_id 6
                            //remote_player_action_update_baseline_id 1
                            
                            
                            
                            //int tickid          = sread_bits(str, &bit, 7);
                            
                            bool apply = false;
                            Biped *p = nullptr;
                            if (player_number < kMaxPlayers) {
                                p = game->getBiped(player_number);
                                p->alive = true;
                                p->dead_tick = 0;
                                
                                //if (tickid > p->last_update_tick || (p->last_update_tick - tickid) > 10) {
                                    p->last_update_tick = update_id;
                                    apply = true;
                                //}
                            }
                            
                            if (unknown) { // confirmed
                                sread_bits(str, &bit, 5);
                            }
                            if (jumping) {
                                int crouch   = sread_bits(str, &bit, 1);
                                int jump     = sread_bits(str, &bit, 1);
                                int flash    = sread_bits(str, &bit, 1);
                                int action   = sread_bits(str, &bit, 1);
                                int melee    = sread_bits(str, &bit, 1);
                                int unk      = sread_bits(str, &bit, 1);
                                int shoot    = sread_bits(str, &bit, 1);
                                int unk1     = sread_bits(str, &bit, 1);
                                int unk2     = sread_bits(str, &bit, 1);
                                int longact  = sread_bits(str, &bit, 1);
                                
                                
                                if (apply) {
                                    p->controls.jumping.crouch  = crouch;
                                    p->controls.jumping.jump    =  jump;
                                    p->controls.jumping.flash   = flash;
                                    p->controls.jumping.action  = action;
                                    p->controls.jumping.melee   = melee;
                                    p->controls.jumping.shoot   = shoot;
                                    p->controls.jumping.longact = longact;
                                }
                            }
                            if (turning) {
                                int y   = sread_bits(str, &bit, 8);
                                int x   = sread_bits(str, &bit, 8);
                                float rx = ((x / 255.0) * M_PI * 2) + M_PI / 2;
                                float ry = (y / 255.0) * M_PI + M_PI/2;
                                
                                if (apply) {
                                    p->controls.look_x = rx;
                                    p->controls.look_y = ry;
                                }
                            }
                            if (movement) {
                                int right     = sread_bits(str, &bit, 1);
                                int left      = sread_bits(str, &bit, 1);
                                int backwards = sread_bits(str, &bit, 1);
                                int forwards  = sread_bits(str, &bit, 1);
                                
                                if (apply) {
                                    p->controls.movement.right      = right==1;
                                    p->controls.movement.left       = left==1;
                                    p->controls.movement.forwards   = forwards==1;
                                    p->controls.movement.backwards  = backwards==1;
                                }
                            }
                            if (shooting) { //confirmed
                                sread_bits(str, &bit, 1);
                            }
                            if (u1) { // confirmed
                                sread_bits(str, &bit, 3); //weapon index
                            }
                            if (u2) { // confirmed
                                sread_bits(str, &bit, 2); // grenade index
                            }
                        }
                        mtx.unlock();
                        continue;
                    } else {
                        printf("MISSING %s\n", message_delta_packet_to_string_table[message_type]);
                        //break;
                    }
                }
                
                
#ifdef VERBOSE
                if (bit == 7) {
                    printf("%d -----------\n", bit);
                    int i;
                    for (i=0; i < (len-head)*8; i++) {
                        printf("%d", read_bits(1, buffer+head, i));
                    }
                    printf("\n");
                }
                printf("K %d %s\n", bit, message_delta_packet_to_string_table[message_type]);
#endif
                
                if (true || message_mode == 0) { // delta
                    
                    
                    
                    
                    /*
                     
                     
                     3 -> 8
                     010101010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011111110
                     01010101001100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011111110
                     0101010101110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111001
                     [pid] 									   [bipd][team]
                     0 111000 11000 00011000000000000001110111000110111 00010 10000 00000000000000000000000000 <--- PLAYER CREATE
                     01010101000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011111110001100111011011100000111001100100
                     

                     
                     4->9
                     10010011000 1 0 101010 1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111001
                      0 101010 0110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111001
                     01010101110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111001
                     01010100001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111001
                     
                         01110000010000000100000000000000001111000110111100100000000000000000000000000000000
                     0101010101001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001001000110101010101001100101101101

                     
                     
                     
                     
                     
                     
                     
                     
                     
                     */
                    
                    
                    
                    
                    
                    printf("X %d %s\n", bit, message_delta_packet_to_string_table[message_type]);
                    if (message_type == _message_delta_unit_kill) {
                        printf("Unit killed [UNKNOWN]\n");
                        int i;
                        for (i=0; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
                        
                        break;
                    } else if (message_type == _message_delta_hud_add_item) {
                        int tag_index = sread_bits(str, &bit, 16);
                        int table_id  = sread_bits(str, &bit, 16);
                        uint16_t integer_small  = sread_bits(str, &bit, 8);
                        uint16_t integer_medium = sread_bits(str, &bit, 16);
                        continue;
                    } else if (message_type == _message_delta_player_create) {
                        uint16_t player_number = sread_bits(str, &bit, 8); //integer_small
                        uint16_t unk  = sread_bits(str, &bit, 32);         //integer_large
                        uint16_t biped_number = sread_bits(str, &bit, 5);  //player_index
                        uint32_t unk3 = sread_bits(str, &bit, 32);         //integer_large
                        
                        
                        //printf("Player %d assigned biped %d\n", player_number, biped_number);
                        
                        
                        /*
                         
                         biped
                         2 95.702271 -145.769958 0.026822 p1 blue
                         8 97.390572 -152.568771 0.079870 p2 red
                         11 79.456398 -140.814713 2.168123 p3 blue
                         
                         011100010000
                         
                         
                         */
                        
                        
                        
                        if (player_number > kMaxPlayers) {
                            //printf("Invalid player number");
                            continue;
                        }
                        Player *p = game->getPlayer(player_number);
                        if (p) {
                            p->team = 0;
                            p->biped_number = biped_number;
                            p->alive = true;
                            printf("%s joined the game as biped %d (player %d)\n", p->name, p->biped_number, player_number);
                        }
                        
                        continue;
                    } else if (message_type == _message_delta_player_score_update) {
                        // LENGTH IS 295
                        int u = 0;
                        for (u=0; u < 9; u++) {
                            sread_bits(str, &bit, 32);
                        }
                        sread_bits(str, &bit, 7);
                        
                        continue;
                    } else if (message_type == _message_delta_oddball_update) {
                        /*
                         
                         
                         0 010010 000001011000100000000000000000000000000000000000000001111111111111111
                         0 100110 001100110000000000000000000000000000000000000000000000000000000000000000000001 //_message_delta_multiplayer_sound

                         
                         */
                        
                        // LENGTH IS 154 //unknown
                        int u = 0;
                        for (u=0; u < 4; u++) {
                            sread_bits(str, &bit, 32);
                        }
                        sread_bits(str, &bit, 26);
                        
                        continue;
                    } else if (message_type == _message_delta_equipment_new) { //confirmed
                        int tag_index       = sread_bits(str, &bit, 16);
                        int table_id        = sread_bits(str, &bit, 16);
                        int object_id       = sread_bits(str, &bit, 9);  //32
                        
                        int integer_medium  = sread_bits(str, &bit, 16); //41
                        int player_index    = sread_bits(str, &bit, 5);  //57 == 0
                        int object_id2      = sread_bits(str, &bit, 9);  //62 == 0
                        int integer_large   = sread_bits(str, &bit, 32); //71 == 0
                        int fixed_width_normal_8bit1  = sread_bits(str, &bit, 16); //103
                        int fixed_width_normal_8bit2  = sread_bits(str, &bit, 16); //119
                        
                        if (map && table_id > 0 && table_id < map->tags.size()) {
                            ProtonTag *weaponTag = map->tags.at(table_id).get();
                            printf("Create equipment %s for player %d\n", weaponTag->Name(), player_index);
                        } else {
                            printf("Invalid equipment 0x%x 0x%x for player %d\n", tag_index, table_id, player_index);
                        }
                        
                        int integer_small   = sread_bits(str, &bit, 8); //135
                        
                        NetworkObject *object = game->getObject(object_id);
                        object->table_id = table_id;
                        object->tag_index = tag_index;
                        int x = sread_bits(str, &bit, 32); //143
                        int y = sread_bits(str, &bit, 32);
                        int z = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        object->position.x = xf;
                        object->position.y = yf;
                        object->position.z = zf;
                        int u = sread_bits(str, &bit, 3);
                        char *buffer = itoa (fixed_width_normal_8bit2,2);
                        printf ("binary: %s (%d)\n",buffer, fixed_width_normal_8bit2);
                        
                        //sread_bits(str, &bit, 0x5F);
                        //sread_bits(str, &bit, 0x5F);
                        
                        //translational_velocity + angular_velocity = 3?
                        
                        
                        
                        //242
                        
                        
                        /*
                        #ifdef VERBOSE
                        printf("New equipment (0x%x)\n", object_id);
                        int zx = 0;
                        int i;
                        for (i=bit; i < bit+204; i++) {
                            int x = sread_bits(str, &i, 32);
                            int y = sread_bits(str, &i, 32);
                            int z = sread_bits(str, &i, 32);
                            float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                            float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                            float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                            i -= 32 * 3;
                            
                            printf("%d %f %f %f\n", zx, xf, yf, zf);
                            zx++;
                        }
                        printf("\n");
                        
                        // LENGTH IS 200 unknown
                        int u = 0;
                        for (u=0; u < 20; u++) {
                            sread_bits(str, &bit, 10);
                        }
                         */
                        continue;
                    } else if (message_type == _message_delta_netgame_equipment_new) {
                        int tag_index = sread_bits(str, &bit, 16);
                        int table_id  = sread_bits(str, &bit, 16);
                        int object_id = sread_bits(str, &bit, 10);
                        
                        int unknown0 = sread_bits(str, &bit, 17);
                        int unknown2 = sread_bits(str, &bit, 32);
                        int x        = sread_bits(str, &bit, 32);
                        int y        = sread_bits(str, &bit, 32);
                        int z        = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        
                        NetworkObject *object = game->getObject(object_id);
                        object->table_id = table_id;
                        object->tag_index = tag_index;
                        object->position.x = xf;
                        object->position.y = yf;
                        object->position.z = zf;
                        continue;
                    } else if (message_type == _message_delta_weapon_new) { //455 -> 311
                        int tag_index       = sread_bits(str, &bit, 16); //0
                        int table_id        = sread_bits(str, &bit, 16); //16
                        int object_id       = sread_bits(str, &bit, 9);  //32
                        
                        int integer_medium  = sread_bits(str, &bit, 16); //41
                        int player_index    = sread_bits(str, &bit, 5);  //57
                        int object_id2      = sread_bits(str, &bit, 9);  //62
                        int integer_large   = sread_bits(str, &bit, 32); //71
                        int fixed_width_normal_8bit1  = sread_bits(str, &bit, 16); //103
                        int fixed_width_normal_8bit2  = sread_bits(str, &bit, 16); //119
                        
                        if (map && table_id > 0 && table_id < map->tags.size()) {
                            ProtonTag *weaponTag = map->tags.at(table_id).get();
                            printf("Create weapon %s for player %d\n", weaponTag->Name(), player_index);
                        } else {
                            printf("Invalid weapon 0x%x 0x%x for player %d\n", tag_index, table_id, player_index);
                            
                        }
                        
                        int integer_small   = sread_bits(str, &bit, 8); //135
                        
                        NetworkObject *object = game->getObject(object_id);
                        object->table_id = table_id;
                        object->tag_index = tag_index;
                        int x = sread_bits(str, &bit, 32); //143
                        int y = sread_bits(str, &bit, 32);
                        int z = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        object->position.x = xf;
                        object->position.y = yf;
                        object->position.z = zf;
                        
                        /*

                         72?
                         
                        
                        int v = sread_bits(str, &bit, 3);
                        
                        int vx = sread_bits(str, &bit, 32);
                        int vy = sread_bits(str, &bit, 32);
                        int vz = sread_bits(str, &bit, 31);
                         
                        int integer_medium2   = sread_bits(str, &bit, 16);
                        int integer_medium3   = sread_bits(str, &bit, 16);
                        int fixed_width_6bits = sread_bits(str, &bit, 6);
                        int integer_medium4   = sread_bits(str, &bit, 16);
                        int integer_medium5   = sread_bits(str, &bit, 16);
                        continue;
                        */
                        
              
  
                        //int u = 0;
                        //for (u=0; u < 10; u++) { //42
                        //    sread_bits(str, &bit, 10);
                        //}
                        //sread_bits(str, &bit, 2); //142
                        
                        int unknown2 = sread_bits(str, &bit, 32);
                        int unknown3 = sread_bits(str, &bit, 32);
                        int unknown4 = sread_bits(str, &bit, 8);
                        continue;
                    } else if (message_type == _message_delta_vehicle_new) { //confirmed
                        int tag_index = sread_bits(str, &bit, 16);
                        int table_id  = sread_bits(str, &bit, 16);
                        int object_id = sread_bits(str, &bit, 10);
                        
                        NetworkObject *object = game->getObject(object_id);
                        object->table_id = table_id;
                        object->tag_index = tag_index;
                        sread_bits(str, &bit, 32);
                        sread_bits(str, &bit, 32);
                        sread_bits(str, &bit, 9);
                        int x   = sread_bits(str, &bit, 32);
                        int y   = sread_bits(str, &bit, 32);
                        int z   = sread_bits(str, &bit, 32);
                        int r   = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        float rx = 0.0; memcpy(&rx, &r, sizeof(float));
                        object->position.x = xf;
                        object->position.y = yf;
                        object->position.z = zf;
                        object->rotation.z = rx;
                        sread_bits(str, &bit, 3);
                        continue;
                        
                        /*
                    
                        
                        //#ifdef VERBOSE
                        printf("New vehicle (0x%x)\n", object_id);
                        int zx = 0;
                        int i;
                        for (i=bit; i < bit+204; i++) {
                            int x = sread_bits(str, &i, 32);
                            int y = sread_bits(str, &i, 32);
                            int z = sread_bits(str, &i, 32);
                            float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                            float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                            float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                            i -= 32 * 3;
                            
                            printf("%d %f %f %f\n", zx, xf, yf, zf);
                            zx++;
                        }
                        printf("\n");
                        
                        //9141
                        //9214
                        //73
                        
                        // LENGTH IS 204 unknown
                        int u = 0;
                        for (u=0; u < 20; u++) {
                            sread_bits(str, &bit, 10);
                        }
                        sread_bits(str, &bit, 4);
                        continue;
                         */
                    }
                    
                    /*else if (message_type == _message_delta_unit_drop_current_weapon) { //unconfirmed
                        int tag_index = sread_bits(str, &bit, 16);
                        int table_id  = sread_bits(str, &bit, 16);
                        int object_id = sread_bits(str, &bit, 10);
                        printf("New vehicle (0x%x)\n", object_id);
                        
                     
                     000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011111110010000100010001001110001000000000000000000000000000000000000000011111111111111111110110000001110000000000000000000001000000000000000000000000000000000000000000000001
                     
                     
                        // LENGTH IS 456 unknown
                        int u = 0;
                        for (u=0; u < 45; u++) {
                            sread_bits(str, &bit, 10);
                        }
                        sread_bits(str, &bit, 7);
                        continue;
                    }*/
                    else if (message_type == _message_delta_biped_update) {
                        int player_id  = sread_bits(str, &bit, 4);
                        int unknown_id = sread_bits(str, &bit, 6);
                        int object_id  = sread_bits(str, &bit, 10);
                        
                        
                       
                        
                  
                        
                        
                        //000100000001000100000000101001000000000000001000001011111111101001110110001011110100001100110110
#ifdef VERBOSE
                        printf("Update Biped %d: 0x%x\n", player_id, object_id);
                        int i;
                        for (i=0; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
#endif
                        break;
                    }
                    else if (message_type == _message_delta_game_settings_update) {
                        printf("Game settings update\n");
                        int name_length = sread_bits(str, &bit, 7);
                        
                        // Skip the string
                        unsigned char game_name[name_length];
                        printf("Name: %d ", name_length);
                        int i;
                        for (i=0; i < name_length; i++) {
                            uint16_t byte = sread_bits(str, &bit, 16);
                            game_name[i] = byte;
                            
                            printf("%c", byte);
                        }
                        game_name[i] = '\0';
                        printf("\n");
                        
                        int version = sread_bits(str, &bit, 32);
                        printf("Version: 0x%x\n", version);
                        
                        int map_length = sread_bits(str, &bit, 8);
                        unsigned char map_name[map_length];
                        printf("Name: %d ", map_length);
                        for (i=0; i < map_length; i++) {
                            uint16_t byte = sread_bits(str, &bit, 8);
                            map_name[i] = byte;
                            printf("%c", byte);
                        }
                        map_name[i] = '\0';
                        printf("\n");

                        #ifdef VERBOSE
                        for (i=(bit+12); i < (bit+12) + (23*32+10); i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
#endif
                        
                        
                        int u = 0;
                        for (u=0; u < 23; u++) {
                            sread_bits(str, &bit, 32);
                        }
                        sread_bits(str, &bit, 10);
                        
                        // Player number
                        uint16_t player_count = sread_bits(str, &bit, 16);
                        printf("Players: %d\n", player_count);

                        // First player name
                        int p = 0;
                        for (p=0; p < 16; p++) {
                            uint16_t player_name_length = sread_bits(str, &bit, 4);
                            char *player_name = (char *)malloc(player_name_length+1);
                            printf("Player %d Name: %d ", p, player_name_length);
                            for (i=0; i < player_name_length; i++) {
                                uint16_t byte = sread_bits(str, &bit, 16);
                                player_name[i] = byte;
                                printf("%c", byte);
                            }
                            player_name[i] = '\0';
                            
                            
                            uint16_t unk0 = sread_bits(str, &bit, 16);
                            uint16_t unk1 = sread_bits(str, &bit, 16);
                            uint16_t pnum = sread_bits(str, &bit, 16);
                            uint16_t team = sread_bits(str, &bit, 8);
                            uint16_t pid  = sread_bits(str, &bit, 8);
                            
                            if (pid != 0xFF) {
                                Player *pn = game->getPlayer(pid);
                                if (pn != nullptr) {
                                    pn->name = player_name;
                                    pn->team = team;
                                }
                                
                                printf(" (%d %d %d %d %d)\n", unk0, unk1, pnum, pid, team);
                            }
                        }
                        
                        for (i=(bit+12); i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
                        continue;
                    } else if (message_type == _message_delta_object_deletion) {
                        int object_id = sread_bits(str, &bit, 9);
                        NetworkObject *object = game->getObject(object_id);
                        if (object != nullptr) {
                            object->table_id = -1;
                            object->tag_index = -1;
                        }
                        continue; //confirmed
                    } else if (message_type == _message_delta_super_ping_update) {
                        int count = sread_bits(str, &bit, 4);
                        //printf("=== PING ===\n");
                        int i;
                        for (i=0; i <= count; i++) {
                            int player   = sread_bits(str, &bit, 8);
                            int ping     = sread_bits(str, &bit, 32);
                            printf("Player %d: %dms\n", player, ping);
                        }
                        printf("--------\n");
                        continue;
                    } else if (message_type == _message_delta_weapon_start_reload) {
                        printf("WEAPON START RELOAD [UNKNOWN]\n");
                        //01010000000 1 0 110101 1001111000 0000000000000000
                        //            00101010000000001011000000000000010 00000011100010001100001000101010
                        
                        
                        //p1 [12] 0011
                        //01010000000 1 0 110101 0000100100 0000000000000000
                        //            01110010000000010100100000000000101 01011011110000111001110011111000
                        
                        //p3 [14] 0111
                        //01010000000 1 0 110101 1011100100 0000000000000000
                        //            01010010000000010001000000000000110 10101111001010110100000010110110
                        
                        
                        
                        
                            /*
                             weapon (0xc7) reloading
                             01010000000 1 0 110101 1110001100000000000000000 1 1 100001 00000000 001011 0000000000 0010 10011011000000001001111100000001
                             01010000000 1 0 110101 1110001100000000000000000 1 1 111110 00000000 110011 0000000000 1000 10011101100110010110011101101010
                             01010000000 1 0 110101 1110001100000000000000000 0 1 101110 00000000 101011 0000000000 0100 10100100101000100111011110010111
                             01010000000 1 0 110101 1110001100000000000000000 0 1 111010 00000000 111001 0000000000 0010 10100011110101001010011101111001
                             
                             c8
                             01010000000 1 0 110101 0001001100000000000000000 00001100 00000000 000100 0000000000 0110 10011110000100001100111000001010
                             */
                        
                        //
                        
                        int weapon = sread_bits(str, &bit, 10); // should be 25?
                        //int junk   = sread_bits(str, &bit, 14); //<--- whats junk?
                        
                        #ifdef VERBOSE
                        printf("weapon (0x%x) reloading\n", weapon);
                        int i;
                        for (i=0; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
#endif
                        
                        
                        continue;
                    } else if (message_type == _message_delta_weapon_cancel_reload) {
                        printf("WEAPON CANCEL RELOAD [UNKNOWN]\n");
                        int weapon = sread_bits(str, &bit, 10);
                        //int junk   = sread_bits(str, &bit, 14); //<--- whats junk?
                        
                        #ifdef VERBOSE
                        printf("weapon (0x%x) cancel reloading\n", weapon);
                        int i;
                        for (i=0; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
#endif
                        
                        
                        continue;
                    } else if (message_type == _message_delta_weapon_finish_reload) {
                        printf("WEAPON RELOAD [UNKNOWN]\n");
                        int weapon = sread_bits(str, &bit, 10);
                        //int junk   = sread_bits(str, &bit, 14); //<--- whats junk?
                        
                        #ifdef VERBOSE
                        printf("weapon (0x%x) finished reloading\n", weapon);
                        int i;
                        for (i=0; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
                        #endif
                        
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
                    } else if (message_type == _message_delta_local_player_update) {
                        printf("Locate update [UNKNOWN]\n");
                        int unknown0 = sread_bits(str, &bit, 11); //11010011110
                        uint32_t x   = sread_bits(str, &bit, 32);
                        uint32_t y   = sread_bits(str, &bit, 32);
                        uint32_t z   = sread_bits(str, &bit, 32);
                        int bitmask0 = sread_bits(str, &bit, 22);
                        int bitmask1 = sread_bits(str, &bit, 12);

                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        Player *p = game->getPlayer(p_no);
                        Biped *meb = game->getBiped(0);
                        if (meb != nullptr) {
                            meb->position->x = xf;
                            meb->position->y = yf;
                            meb->position->z = zf;
                            meb->alive = true;
                        }
                        continue;
                    } else if (message_type == _message_delta_team_change) {
                        int integer_small1  = sread_bits(str, &bit, 8);
                        int integer_small2  = sread_bits(str, &bit, 8);
                        
                        printf("Player %d changed to team %d\n", integer_small1, integer_small2);
                        continue;
                    } else if (message_type == _message_delta_player_spawn) { // max size: 222
                        int player_number  = sread_bits(str, &bit, 5);
                        int object_id      = sread_bits(str, &bit, 9);
                        
                        Biped *biped = game->getBiped(player_number);
                        NetworkObject *object = game->getObject(object_id);
                        if (biped) {
                            biped->alive = true;
                            biped->object_index = object_id;
                            biped->position->x = object->position.x;
                            biped->position->y = object->position.y;
                            biped->position->z = object->position.z;
                        }
                        
                        int integer_large   = sread_bits(str, &bit, 32);
                        int object_id2      = sread_bits(str, &bit, 9);
                        int integer_medium  = sread_bits(str, &bit, 16);
                        int object_id3      = sread_bits(str, &bit, 9);
                        int object_id4      = sread_bits(str, &bit, 9);
                        int object_id5      = sread_bits(str, &bit, 9);
                        int object_id6      = sread_bits(str, &bit, 9);
                        int integer_medium2 = sread_bits(str, &bit, 16);
                        int integer_medium3 = sread_bits(str, &bit, 16);
                        int integer_medium4 = sread_bits(str, &bit, 16);
                        
    
                        
                 
                        continue;
                    } else if (message_type == _message_delta_biped_new) {
                        int start_bit = bit+12;
                        int tag_index = sread_bits(str, &bit, 16); // definition_index
                        int table_id  = sread_bits(str, &bit, 16); //
                        int object_id = sread_bits(str, &bit, 10);
                        int unk0  = sread_bits(str, &bit, 32);
                        int unk1  = sread_bits(str, &bit, 32);
                        int unk2  = sread_bits(str, &bit, 32);
                        int unk3  = sread_bits(str, &bit, 32);
                        int unk4  = sread_bits(str, &bit, 20);
                        int unkf  = sread_bits(str, &bit, 12);
                        int unk5  = sread_bits(str, &bit, 32);
                        int unk6  = sread_bits(str, &bit, 32);
                        int unk7  = sread_bits(str, &bit, 32);
                        int unk8  = sread_bits(str, &bit, 32);
                        int unk9  = sread_bits(str, &bit, 32);
                        int unk10 = sread_bits(str, &bit, 32);
                        int unk11 = sread_bits(str, &bit, 32);
                        int unk12 = sread_bits(str, &bit, 32);
                        int unk13 = sread_bits(str, &bit, 32);
                        int unk15 = sread_bits(str, &bit, 6);
                        int unk16 = sread_bits(str, &bit, 6);
                        
                        int x = sread_bits(str, &bit, 32);
                        int y = sread_bits(str, &bit, 32);
                        int z = sread_bits(str, &bit, 32);
                        float xf = 0.0; memcpy(&xf, &x, sizeof(float));
                        float yf = 0.0; memcpy(&yf, &y, sizeof(float));
                        float zf = 0.0; memcpy(&zf, &z, sizeof(float));
                        
                        int unk17 = sread_bits(str, &bit, 5);
                        if (unk17 != 1) {
                            sread_bits(str, &bit, 48); // rotation or something...
                        }
                        int unk18 = sread_bits(str, &bit, 13);
                       
                        NetworkObject *object = game->getObject(object_id);
                        object->table_id = table_id;
                        object->tag_index = tag_index;
                        object->position.x = xf;
                        object->position.y = yf;
                        object->position.z = zf;
                        
                        //28
                        
                        #ifdef VERBOSE
                        printf("NEW BIPED (0x%x) of type 0x%x at %f %f %f (%d)\n", object_id, table_id, xf, yf, zf, bit-start_bit);
                        int i;
                        for (i=start_bit; i < (len-head)*8; i++) {
                            printf("%d", read_bits(1, buffer+head, i));
                        }
                        printf("\n");
                        #endif
                        continue; // wrong :9
                    } else if (message_type == _message_delta_remote_player_total_update_biped && bit == 7) {
                        int player_number       = sread_bits(str, &bit, 5); //BIPED NUMBER [probably 5]
                        int unknown0            = sread_bits(str, &bit, 10);
                        int unknown1            = sread_bits(str, &bit, 1);
                        int unk                 = sread_bits(str, &bit, 4);
                        int crouched            = sread_bits(str, &bit, 1);
                        int jumping             = sread_bits(str, &bit, 1);
                        int unk7                = sread_bits(str, &bit, 7);
                        int action              = sread_bits(str, &bit, 1);
                        
                
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
                        int ry   = sread_bits(str, &bit, 8);
                        int rx   = sread_bits(str, &bit, 8);
                        int right                = sread_bits(str, &bit, 1);
                        int left                 = sread_bits(str, &bit, 1);
                        int back                 = sread_bits(str, &bit, 1);
                        int forward              = sread_bits(str, &bit, 1);

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
                        
                        float rot_x = ((rx / 256.0) * M_PI * 2) + M_PI / 2;
                        float rot_y = (ry  / 256.0) * M_PI + M_PI/2;
                        
                        Biped *p = game->getBiped(player_number);
                        if (p != nullptr) {
                            NetworkObject *biped_object = game->getObject(p->object_index);
                            if (biped_object) {
                                biped_object->position.x = x;
                                biped_object->position.y = y;
                                biped_object->position.z = z;
                            }
                            
                            p->position->x = x;
                            p->position->y = y;
                            p->position->z = z;
                            p->controls.look_x = rot_x;
                            p->controls.look_y = rot_y;
                            
                            p->alive = true;
                            p->controls.movement.forwards   = forward;
                            p->controls.movement.backwards  = back;
                            p->controls.movement.left       = left;
                            p->controls.movement.right      = right;
                        }
                        
                        break;
                    } else {
                        printf("MISSING C %s\n", message_delta_packet_to_string_table[message_type]);

                        break;
                        break;
                    }
                }
                break;
            }
        }
        
        // AI Tick update
        // Find the closest player
        intel->tick(game, 0);
    
    
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
        
    //}
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
    while (true) {
        main();
    }
}

Client::Client(const char *name, const char *ip, short port, ai *intelligence) {
    intel = intelligence;
    
    // Setup general structures
    memcpy(this->name, name, strlen(name)+1);
    
    // Setup controls
    printf("setting up controls\n");
    this->map = intelligence->map;
    game = new Game(intelligence->map);
    
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
