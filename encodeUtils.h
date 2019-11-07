#ifndef ENCODEUTILS_H_
#define ENCODEUTILS_H_

void urlencode(unsigned char * src, int src_len, unsigned char * dest, int dest_len);
unsigned char* urldecode(unsigned char* encd, unsigned char* decd);


#endif /* ENCODEUTILS_H_ */
