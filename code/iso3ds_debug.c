#include "iso3ds_debug.h"

#include <string.h>
#include <stdio.h>

/*
 * Adjust these includes to your actual headers.
 *
 * They are needed for:
 *  - Field IDs: V_VCAS_SCORE, V_CAVVRSCODE, V_IPADDRS, V_VIC_ECI,
 *               C_CAVVAUTHRS, C_CAVVAUTHRS2, C_CAVVKEYIND, C_POSCDIM,
 *               I_SETTXNID, etc.
 *  - F_pres(), F_get(), CF_get()
 *  - DBG_PRINTF(), dbg_proginfo
 */
#include "tuxfbuf.h"      /* F_pres, F_get, CF_get, FLD_STRING, SUCCEED, etc. */
#include "visaxt.fd.h"    /* V_VCAS_SCORE, V_IPADDRS, V_VIC_ECI, ...        */
#include "cocrd.fd.h"     /* C_CAVVAUTHRS, C_CAVVAUTHRS2, C_CAVVKEYIND,
                             C_POSCDIM, I_SETTXNID, ...                     */
#include "mxuiso93.fd.h"  /* V_CAVVRSCODE (if that’s where it’s defined)    */
/* If DBG_PRINTF/dbg_proginfo live in a specific header, include it here:
 * #include "dbgprt.h"
 */

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

static void
buf_to_string(char *dst, size_t dstlen, const char *src, size_t srclen)
{
    size_t i;

    if (!dst || dstlen == 0) {
        return;
    }

    if (!src || srclen == 0) {
        dst[0] = '\0';
        return;
    }

    if (srclen >= dstlen) {
        srclen = dstlen - 1;
    }

    for (i = 0; i < srclen; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

/*
 * Read a string FB field (CF_get) into a local buffer safely.
 * Returns 1 if present and successfully read, 0 otherwise.
 */
static int
fb_get_string(FBFR *p_fb, int fldid, int occ, char *out, size_t outlen)
{
    char tmp[256];

    if (!out || outlen == 0) {
        return 0;
    }

    out[0] = '\0';

    if (!F_pres(p_fb, fldid, occ)) {
        return 0;
    }

    if (SUCCEED != CF_get(p_fb, fldid, occ, tmp, 0L, FLD_STRING)) {
        return 0;
    }

    buf_to_string(out, outlen, tmp, strlen(tmp));
    return 1;
}

/*
 * Read a short FB field (F_get) into *p_val.
 * Returns 1 if present and successfully read, 0 otherwise.
 */
static int
fb_get_short(FBFR *p_fb, int fldid, int occ, short *p_val)
{
    if (!p_val) {
        return 0;
    }

    *p_val = 0;

    if (!F_pres(p_fb, fldid, occ)) {
        return 0;
    }

    if (SUCCEED != F_get(p_fb, fldid, occ, (char *)p_val, 0L)) {
        return 0;
    }

    return 1;
}

/*
 * Read a char FB field (F_get) into *p_val.
 * Returns 1 if present and successfully read, 0 otherwise.
 */
static int
fb_get_char(FBFR *p_fb, int fldid, int occ, char *p_val)
{
    if (!p_val) {
        return 0;
    }

    *p_val = 0;

    if (!F_pres(p_fb, fldid, occ)) {
        return 0;
    }

    if (SUCCEED != F_get(p_fb, fldid, occ, p_val, 0L)) {
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
/* Main debug dump – uses DBG_PRINTF() only                                  */
/* ------------------------------------------------------------------------- */

void
Iso3DS_DebugDump(FBFR *p_fb)
{
    short s_val;
    char  c_val;
    char  str[256];

    char  eci_raw[32];
    char  ars_char = 0;
    char  auth_secondary = ' ';

    if (!p_fb) {
        DBG_PRINTF((dbg_proginfo,
                    "Iso3DS_DebugDump: p_fb is NULL – nothing to dump"));
        return;
    }

    DBG_PRINTF((dbg_proginfo,
                "================ ISO 3DS / Visa Secure debug ================"));

    /* ------------------------------------------------------------------ */
    /* consumerAuthenticationScore  (from V_VCAS_SCORE)                   */
    /* Visa: F34 / Dataset 56 / Tag DF1F (VCAS Score)                     */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[consumerAuthenticationScore]"));
    if (fb_get_short(p_fb, V_VCAS_SCORE, 0, &s_val)) {
        DBG_PRINTF((dbg_proginfo,
                    "  V_VCAS_SCORE (short)       : %hd", s_val));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  V_VCAS_SCORE               : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* cavvResult  (from V_CAVVRSCODE)                                   */
    /* Visa: F44.13 – CAVV Results Code                                   */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[cavvResult]"));
    if (fb_get_string(p_fb, V_CAVVRSCODE, 0, str, sizeof(str))) {
        DBG_PRINTF((dbg_proginfo,
                    "  V_CAVVRSCODE (string)      : '%s'", str));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  V_CAVVRSCODE               : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* secondFactorAuthCode  (from C_CAVVAUTHRS2)                          */
    /* Visa: F126.9 Usage 3 – Position 2 (Authentication Method)          */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[secondFactorAuthCode]"));
    if (fb_get_string(p_fb, C_CAVVAUTHRS2, 0, str, sizeof(str))) {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVAUTHRS2 (string)     : '%s'", str));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVAUTHRS2              : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* userData03  (from V_IPADDRS)                                      */
    /* Visa: F34 / Dataset 56 / Tag 9F1F – Consumer Device IP Address    */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[userData03]"));
    if (fb_get_string(p_fb, V_IPADDRS, 0, str, sizeof(str))) {
        DBG_PRINTF((dbg_proginfo,
                    "  V_IPADDRS (string)         : '%s'", str));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  V_IPADDRS                  : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* cavvKeyIndicator  (from C_CAVVKEYIND)                              */
    /* Visa: F126.9 Usage 2/3 – Position 3 (Key Indicator)                */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[cavvKeyIndicator]"));
    if (fb_get_short(p_fb, C_CAVVKEYIND, 0, &s_val)) {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVKEYIND (short)       : %hd", s_val));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVKEYIND               : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* eciIndicator  (from V_VIC_ECI / C_POSCDIM)                         */
    /* Visa: F60.8 – MOTO/ECI/Payment Indicator                            */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[eciIndicator]"));

    if (fb_get_string(p_fb, V_VIC_ECI, 0, eci_raw, sizeof(eci_raw))) {
        DBG_PRINTF((dbg_proginfo,
                    "  V_VIC_ECI (raw ECI)        : '%s'", eci_raw));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  V_VIC_ECI                  : (not present)"));
        eci_raw[0] = '\0';
    }

    if (fb_get_char(p_fb, C_POSCDIM, 0, &c_val)) {
        DBG_PRINTF((dbg_proginfo,
                    "  C_POSCDIM (raw)            : %d", (int)c_val));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  C_POSCDIM                  : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* userData07  (from I_SETTXNID)                                     */
    /* Visa: F126.8 – Transaction ID (XID/TAVV)                           */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[userData07]"));
    if (fb_get_string(p_fb, I_SETTXNID, 0, str, sizeof(str))) {
        DBG_PRINTF((dbg_proginfo,
                    "  I_SETTXNID (HEX string)    : '%s'", str));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  I_SETTXNID                 : (not present)"));
    }

    /* ------------------------------------------------------------------ */
    /* authSecondaryVerify  (derived, but we print only its inputs +      */
    /* computed value; FB is NOT modified)                                */
    /* BRD rule:                                                          */
    /*   if ECI in {05,06,07} and CAVVAUTHRS == '0' -> 'V'                */
    /*   if ECI in {05,06,07} and CAVVAUTHRS != '0' -> 'I'                */
    /*   else                                 -> ' '                      */
    /* ------------------------------------------------------------------ */
    DBG_PRINTF((dbg_proginfo, "[authSecondaryVerify]"));

    if (fb_get_char(p_fb, C_CAVVAUTHRS, 0, &ars_char)) {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVAUTHRS (char)        : '%c'", ars_char));
    } else {
        DBG_PRINTF((dbg_proginfo,
                    "  C_CAVVAUTHRS               : (not present)"));
        ars_char = 0;
    }

    /* Compute derived authSecondaryVerify just for debugging purposes */
    if (eci_raw[0] == '0' &&
        (eci_raw[1] == '5' || eci_raw[1] == '6' || eci_raw[1] == '7')) {
        /* ECI 05 / 06 / 07 */
        if (ars_char == '0') {
            auth_secondary = 'V';
        } else if (ars_char != 0) {
            auth_secondary = 'I';
        } else {
            auth_secondary = ' ';
        }
    } else {
        auth_secondary = ' ';
    }

    DBG_PRINTF((dbg_proginfo,
                "  Derived authSecondaryVerify: '%c' (from ECI='%s', ARS='%c')",
                auth_secondary,
                (eci_raw[0] ? eci_raw : "(not set)"),
                (ars_char ? ars_char : ' ')));

    DBG_PRINTF((dbg_proginfo,
                "============================================================"));
}
