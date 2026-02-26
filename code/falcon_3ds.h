#ifndef FALCON_3DS_H
#define FALCON_3DS_H

/*
 * 3-DS / Visa Secure fields for Falcon DBTRAN25
 *
 * Requirement 148-25_BPD – Fields:
 *
 *  - consumerAuthenticationScore  (Visa F34 / Dataset 56 / Tag DF1F -> V_VCAS_SCORE)
 *  - cavvResult                   (Visa F44.13 -> V_CAVVRSCODE)                [ALREADY IMPLEMENTED]
 *  - secondFactorAuthCode         (Visa F126.9 Usage 3 Pos 2 -> C_CAVVAUTHRS2)
 *  - userData03                   (Visa F34 / Dataset 56 / Tag 9F1F -> V_IPADDRS)
 *  - cavvKeyIndicator             (Visa F126.9 Usage 2/3 Pos 3 -> C_CAVVKEYIND) [ALREADY IMPLEMENTED]
 *  - eciIndicator                 (Visa F60.8 -> POSCDIM / MCDIM_*; pad to 2 digits)
 *  - userData07                   (Visa F126.8 (XID/TAVV) -> I_SETTXNID)
 *  - authSecondaryVerify          (Derived from ECI + CAVV Auth Result (C_CAVVAUTHRS))
 *
 * This header declares the FF* builders to be used in falcon2cv.c
 * when building the DBTRAN25 record sent to Falcon.
 */

#include <stddef.h>

/* Forward declarations from existing codebase */
typedef struct FBFR FBFR;
typedef struct DBTRAN25_Context_s DBTRAN25_Context_t;

/* New / updated FF* builders */

/* Requirement field: consumerAuthenticationScore */
int FFconsumerAuthenticationScore(FBFR *p_fb,
                                  char *out,
                                  size_t len,
                                  DBTRAN25_Context_t *p_ctx);

/* Requirement field: secondFactorAuthCode */
int FFsecondFactorAuthCode(FBFR *p_fb,
                           char *out,
                           size_t len,
                           DBTRAN25_Context_t *p_ctx);

/* Requirement field: userData03 (consumer device IP address) */
int FFuserData03(FBFR *p_fb,
                 char *out,
                 size_t len,
                 DBTRAN25_Context_t *p_ctx);

/* Requirement field: userData07 (XID/TAVV from F126.8) */
int FFuserData07(FBFR *p_fb,
                 char *out,
                 size_t len,
                 DBTRAN25_Context_t *p_ctx);

/* Requirement field: authSecondaryVerify (derived V / I / blank) */
int FFauthSecondaryVerify(FBFR *p_fb,
                          char *out,
                          size_t len,
                          DBTRAN25_Context_t *p_ctx);

/*
 * Requirement field: eciIndicator
 *
 * This is an UPDATED implementation – replace the existing FFeciIndicator()
 * in falcon2cv.c with this one to ensure the value is ALWAYS 2 digits ("0X").
 */
int FFeciIndicator(FBFR *p_fb,
                   char *out,
                   size_t len,
                   DBTRAN25_Context_t *p_ctx);

#endif /* FALCON_3DS_H */
