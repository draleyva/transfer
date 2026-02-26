#include <string.h>
#include <stdio.h>

#include "falcon_3ds.h"

/*
 * These includes assume your existing layout:
 *  - F_pres / F_get / CF_get macros
 *  - FB field ids (V_VCAS_SCORE, C_CAVVAUTHRS2, V_IPADDRS, I_SETTXNID,
 *    C_CAVVAUTHRS, C_POSCDIM, C_CAVVKEYIND, etc.)
 *  - Macros FBISEC(), FBISMOTO(), MCHP_* and MCDIM_* used in the
 *    original FFeciIndicator implementation.
 *
 * Adjust header names if needed to match your tree.
 */
#include "tuxfbuf.h"
#include "visaxt.fd.h"
#include "cocrd.fd.h"
#include "falcon2cv.h"    /* for DBTRAN25_Context_t, FBISEC, FBISMOTO, etc. */

/* ------------------------------------------------------------------------- */
/* Local helpers                                                             */
/* ------------------------------------------------------------------------- */

/*
 * Left-justify src into out, padded with spaces to len.
 * Truncates src if longer than len.
 */
static void
copy_ljust_space(char *out, size_t len, const char *src)
{
    size_t i = 0;
    if (!out || !len)
        return;

    if (src) {
        for (; i < len && src[i] != '\0'; ++i) {
            out[i] = src[i];
        }
    }
    for (; i < len; ++i) {
        out[i] = ' ';
    }
}

/* ------------------------------------------------------------------------- */
/* Requirement field: consumerAuthenticationScore                            */
/* Visa F34 / Dataset 56 / Tag DF1F -> V_VCAS_SCORE -> DBTRAN25.consumer...  */
/* ------------------------------------------------------------------------- */

int
FFconsumerAuthenticationScore(FBFR *p_fb,
                              char *out,
                              size_t len,
                              DBTRAN25_Context_t *p_ctx)
{
    (void)p_ctx;  /* not used for this field */

    short score = 0;
    char  tmp[8];

    if (!out || len == 0)
        return FAIL;

    /* Default: blanks when the field is not present / not applicable */
    memset(out, ' ', len);

    if (!F_pres(p_fb, V_VCAS_SCORE, 0))
        return SUCCEED;

    if (SUCCEED != F_get(p_fb, V_VCAS_SCORE, 0, (char *)&score, 0L)) {
        DBG_PRINTF((dbg_syserr, "FFconsumerAuthenticationScore: "
                    "failed to get V_VCAS_SCORE from FB"));
        return FAIL;
    }

    /* Defensive range check – Visa BRD expects 0-999 */
    if (score < 0 || score > 999) {
        DBG_PRINTF((dbg_proginfo, "FFconsumerAuthenticationScore: "
                    "invalid score [%hd], sending blanks", score));
        return SUCCEED; /* keep blanks, do not fail whole mapping */
    }

    /* DBTRAN25 field length is 3; pad with leading zeros */
    snprintf(tmp, sizeof(tmp), "%03hd", score);
    memcpy(out, tmp, (len < 3 ? len : 3));

    return SUCCEED;
}

/* ------------------------------------------------------------------------- */
/* Requirement field: secondFactorAuthCode                                   */
/* Visa F126.9 Usage 3 Pos 2 -> C_CAVVAUTHRS2 -> DBTRAN25.secondFactor...    */
/* ------------------------------------------------------------------------- */

int
FFsecondFactorAuthCode(FBFR *p_fb,
                       char *out,
                       size_t len,
                       DBTRAN25_Context_t *p_ctx)
{
    (void)p_ctx;

    char buf[8];

    if (!out || len == 0)
        return FAIL;

    memset(out, ' ', len);

    if (!F_pres(p_fb, C_CAVVAUTHRS2, 0))
        return SUCCEED;

    if (SUCCEED != CF_get(p_fb, C_CAVVAUTHRS2, 0, buf, 0L, FLD_STRING)) {
        DBG_PRINTF((dbg_syserr, "FFsecondFactorAuthCode: "
                    "failed to get C_CAVVAUTHRS2 from FB"));
        return FAIL;
    }

    /*
     * Visa table defines codes up to 2 characters ("00", "01", ..., "92").
     * If for any reason we only get a single character, left-pad with '0'.
     */
    if (strlen(buf) == 1 && len >= 2) {
        out[0] = '0';
        out[1] = buf[0];
    } else {
        copy_ljust_space(out, len, buf);
    }

    return SUCCEED;
}

/* ------------------------------------------------------------------------- */
/* Requirement field: userData03                                             */
/* Visa F34 / Dataset 56 / Tag 9F1F -> V_IPADDRS -> DBTRAN25.userData03      */
/* ------------------------------------------------------------------------- */

int
FFuserData03(FBFR *p_fb,
             char *out,
             size_t len,
             DBTRAN25_Context_t *p_ctx)
{
    (void)p_ctx;

    char buf[64];

    if (!out || len == 0)
        return FAIL;

    memset(out, ' ', len);

    if (!F_pres(p_fb, V_IPADDRS, 0))
        return SUCCEED;

    if (SUCCEED != CF_get(p_fb, V_IPADDRS, 0, buf, 0L, FLD_STRING)) {
        DBG_PRINTF((dbg_syserr, "FFuserData03: failed to get V_IPADDRS"));
        return FAIL;
    }

    /*
     * BRD: Falcon userData03 length = 15, BPD accepts truncation for IPv6.
     * We simply left-justify the string, truncating if longer than len.
     */
    copy_ljust_space(out, len, buf);

    return SUCCEED;
}

/* ------------------------------------------------------------------------- */
/* Requirement field: userData07                                             */
/* Visa F126.8 (XID/TAVV) -> I_SETTXNID (40-char HEX) -> DBTRAN25.userData07 */
/* ------------------------------------------------------------------------- */

int
FFuserData07(FBFR *p_fb,
             char *out,
             size_t len,
             DBTRAN25_Context_t *p_ctx)
{
    (void)p_ctx;

    char buf[64];

    if (!out || len == 0)
        return FAIL;

    memset(out, ' ', len);

    if (!F_pres(p_fb, I_SETTXNID, 0))
        return SUCCEED;

    if (SUCCEED != CF_get(p_fb, I_SETTXNID, 0, buf, 0L, FLD_STRING)) {
        DBG_PRINTF((dbg_syserr, "FFuserData07: failed to get I_SETTXNID"));
        return FAIL;
    }

    /* I_SETTXNID is already HEX-encoded; just copy and pad */
    copy_ljust_space(out, len, buf);

    return SUCCEED;
}

/* ------------------------------------------------------------------------- */
/* Requirement field: authSecondaryVerify                                    */
/* Derived (ECI + CAVV Auth Result) -> DBTRAN25.authSecondaryVerify          */
/* Rules:                                                                    */
/*   if ECI in {5,6,7} and CAVVAUTHRS == '0' -> 'V'                          */
/*   if ECI in {5,6,7} and CAVVAUTHRS != '0' -> 'I'                          */
/*   else                                -> ' '                              */
/* ------------------------------------------------------------------------- */

int
FFauthSecondaryVerify(FBFR *p_fb,
                      char *out,
                      size_t len,
                      DBTRAN25_Context_t *p_ctx)
{
    char ars      = 0;
    char poscdim  = 0;
    char eci_digit = '0';

    if (!out || len == 0)
        return FAIL;

    memset(out, ' ', len);

    /* Get CAVV Authentication Result (position 1) if present */
    if (F_pres(p_fb, C_CAVVAUTHRS, 0)) {
        if (SUCCEED != F_get(p_fb, C_CAVVAUTHRS, 0, &ars, 0L)) {
            DBG_PRINTF((dbg_syserr, "FFauthSecondaryVerify: "
                        "failed to get C_CAVVAUTHRS"));
            return FAIL;
        }
    }

    /*
     * ECI comes from F60.8 and is mapped to internal POSCDIM / poscdim.
     * We rely on the same mapping logic as FFeciIndicator().
     */

    if (F_pres(p_fb, C_POSCDIM, 0)) {
        if (SUCCEED != F_get(p_fb, C_POSCDIM, 0, &poscdim, 0L)) {
            DBG_PRINTF((dbg_syserr, "FFauthSecondaryVerify: "
                        "failed to get C_POSCDIM"));
            return FAIL;
        }
    }

    if (!FBISEC(p_fb)) {
        eci_digit = '0';
    } else if (FBISMOTO(p_fb)) {
        eci_digit = '1';
    } else if (MCHP_STAND == p_ctx->poschp ||
               MCHP_PRESSTAND == p_ctx->poschp) {
        eci_digit = '2';
    } else if (MCHP_INST == p_ctx->poschp) {
        eci_digit = '3';
    } else if (MCDIM_3DSEC == poscdim) {
        eci_digit = '5';
    } else if (MCDIM_3DSECMRCH == poscdim) {
        eci_digit = '6';
    } else if (MCDIM_ESEC == poscdim) {
        eci_digit = '7';
    } else if (MCDIM_ENOSEC == poscdim) {
        eci_digit = '8';
    }

    /* Apply BRD rule only when ECI is 5/6/7 */
    if (eci_digit == '5' || eci_digit == '6' || eci_digit == '7') {
        if (ars == '0') {
            out[0] = 'V';
        } else if (ars != 0) {
            out[0] = 'I';
        } else {
            out[0] = ' '; /* CAVV auth result missing */
        }
    } else {
        out[0] = ' ';     /* ECI not in {5,6,7} */
    }

    return SUCCEED;
}

/* ------------------------------------------------------------------------- */
/* Requirement field: eciIndicator (CORRECTED IMPLEMENTATION)               */
/* Visa F60.8 -> V_VIC_ECI / POSCDIM -> DBTRAN25.eciIndicator               */
/*                                                                         */
/* Priority:                                                                */
/*   1) Use raw Visa ECI from V_VIC_ECI (Field 60.8) if present/valid.     */
/*   2) Otherwise fall back to historical logic using FBISEC/FBISMOTO,     */
/*      poschp and poscdim.                                                */
/*                                                                         */
/* Output must always be 2 digits: "0X" (BRD RQ3).                          */
/* ------------------------------------------------------------------------- */

ctxprivate int
FFeciIndicator(FBFR *p_fb,
               char *out,
               size_t len,
               DBTRAN25_Context_t *p_ctx)
{
    char  code = '0';
    char  eci_raw[8];
    int   have_raw_eci = 0;
    size_t i;

    if (!out || len < 2)
        return FAIL;

    memset(eci_raw, 0, sizeof(eci_raw));

    /*
     * Step 1: Try to derive from raw Visa ECI (Field 60.8) stored as V_VIC_ECI.
     * Typically this is a 2-digit string "00".."07", etc.
     */
    if (F_pres(p_fb, V_VIC_ECI, 0) &&
        SUCCEED == CF_get(p_fb, V_VIC_ECI, 0, eci_raw, 0L, FLD_STRING))
    {
        /* Expecting at least 2 characters "0X" */
        if (eci_raw[0] >= '0' && eci_raw[0] <= '9' &&
            eci_raw[1] >= '0' && eci_raw[1] <= '9')
        {
            code = eci_raw[1];      /* keep *second* digit, e.g. "05" -> '5' */
            have_raw_eci = 1;
        }
    }

    /*
     * Step 2: If we do NOT have a clean raw ECI, fall back to the original
     * mapping logic based on the internal flags and POS condition codes.
     *
     * This preserves backward-compatible behaviour and keeps you safe if
     * some flows don't populate V_VIC_ECI.
     */
    if (!have_raw_eci)
    {
        if (!FBISEC(p_fb)) {
            code = '0';
        } else if (FBISMOTO(p_fb)) {
            code = '1';
        } else if (MCHP_STAND == p_ctx->poschp ||
                   MCHP_PRESSTAND == p_ctx->poschp) {
            code = '2';
        } else if (MCHP_INST == p_ctx->poschp) {
            code = '3';
        } else if (MCDIM_3DSEC == p_ctx->poscdim) {
            code = '5';
        } else if (MCDIM_3DSECMRCH == p_ctx->poscdim) {
            code = '6';
        } else if (MCDIM_ESEC == p_ctx->poscdim) {
            code = '7';
        } else if (MCDIM_ENOSEC == p_ctx->poscdim) {
            code = '8';
        }
    }

    /*
     * Step 3: BRD RQ3 – eciIndicator must always be 2 characters "0X".
     * We always left-pad with '0', regardless of where 'code' came from.
     */
    out[0] = '0';
    out[1] = code;

    /* If length > 2 in DBTRAN25 layout, pad the rest with spaces */
    for (i = 2; i < len; ++i) {
        out[i] = ' ';
    }

    return SUCCEED;
}

