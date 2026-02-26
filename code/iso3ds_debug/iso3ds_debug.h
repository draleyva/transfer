#ifndef ISO3DS_DEBUG_H
#define ISO3DS_DEBUG_H

/*
 * Debug helper for 3-DS / Visa Secure fields in the MAIN FB buffer.
 *
 * Requirement 148-25_BPD – Fields (logical / Falcon-side names):
 *
 *   - consumerAuthenticationScore   (from V_VCAS_SCORE)
 *   - cavvResult                    (from V_CAVVRSCODE)
 *   - secondFactorAuthCode          (from C_CAVVAUTHRS2)
 *   - userData03                    (from V_IPADDRS)
 *   - cavvKeyIndicator              (from C_CAVVKEYIND)
 *   - eciIndicator                  (from V_VIC_ECI / C_POSCDIM)
 *   - userData07                    (from I_SETTXNID)
 *   - authSecondaryVerify           (derived from V_VIC_ECI + C_CAVVAUTHRS)
 *
 * This module prints ONLY the parsed values present in the main FB buffer.
 * It does NOT call Falcon builders or depend on Falcon code.
 */

#include <stdio.h>   /* FILE */

typedef struct FBFR FBFR;

/*
 * Iso3DS_DebugDump
 *
 *  fp   : destination FILE* (stdout, stderr, or a log file)
 *  p_fb : pointer to the main FB buffer (after ISO 8583 parsing)
 *
 * The function does not modify p_fb.
 */
void Iso3DS_DebugDump(FILE *fp, FBFR *p_fb);

#endif /* ISO3DS_DEBUG_H */
