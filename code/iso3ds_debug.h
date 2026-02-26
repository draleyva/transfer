#ifndef ISO3DS_DEBUG_H
#define ISO3DS_DEBUG_H

/*
 * Debug helper for 3-DS / Visa Secure fields in the MAIN FB buffer.
 *
 * Requirement 148-25_BPD – Logical/Falcon-side fields and their sources:
 *
 *   - consumerAuthenticationScore   <- V_VCAS_SCORE
 *   - cavvResult                    <- V_CAVVRSCODE
 *   - secondFactorAuthCode          <- C_CAVVAUTHRS2
 *   - userData03                    <- V_IPADDRS
 *   - cavvKeyIndicator              <- C_CAVVKEYIND
 *   - eciIndicator                  <- V_VIC_ECI / C_POSCDIM
 *   - userData07                    <- I_SETTXNID
 *   - authSecondaryVerify           <- derived from ECI + C_CAVVAUTHRS
 *
 * This module prints ONLY the parsed values present in the main FB buffer.
 * It does NOT call Falcon builders or depend on Falcon code.
 */

typedef struct FBFR FBFR;

/*
 * Iso3DS_DebugDump
 *
 *  p_fb : pointer to the main FB buffer (after ISO 8583 parsing)
 *
 * Uses existing DBG_PRINTF() logging macros. Does not modify p_fb.
 */
void Iso3DS_DebugDump(FBFR *p_fb);

#endif /* ISO3DS_DEBUG_H */
