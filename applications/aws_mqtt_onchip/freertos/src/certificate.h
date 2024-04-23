/*
 * certificate.h
 *
 */

#ifndef CERTIFICATE_H_
#define CERTIFICATE_H_

/* server certificate */
#define ROOT_CA                                          \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n"


/* client certificate */
#define CLIENT_CERT                                   \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n"


/* client private key */
#define PRIVATE_KEY                                   \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"-----END RSA PRIVATE KEY-----\n"

#endif /* CERTIFICATE_H_ */
