/*-----------------------------------------------------------------------
 * 
 * File		: falcon2cv.h
 *
 * Author	: Ruslans Vasiljevs
 *
 * Created	: 09/05/2022
 *
 * Purpose	: FALCON2 conversion functions declarations
 *
 *-----------------------------------------------------------------------
 * Copyright (c) FIS
 *-----------------------------------------------------------------------*/

#ifndef FALCON2CV_H
#define FALCON2CV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <portable.h>
#include <fml.h>

/* ===== Macros ===== */
#define BUF_SIZE						2048
#define FALCON2_HEADER_LEN				52
#define FALCON2_HEADER_RESERVED_1_LEN	17
#define DEF_FILLER_SiZE					1
#define ERROR_CODE_POS					25
#define ERROR_CODE_LEN					4
#define DECISION_COUNT_POS				423
#define DECISION_COUNT_LEN				2
#define DECISION_CODE_LEN				32
#define DECISION_TYPE_LEN				32
#define ISCHIPCRD(x)					(SVCC1_INTLCHIP == (x) || SVCC1_LOCLCHIP == (x))
#define ISMAGCRD(x)						(SVCC1_INTL == (x) || SVCC1_LOCL == (x))
#define memstrscpy(dst, src, max)		memcpy(dst, src, strlen(src)>(max)?(max):strlen(src))
#define FOREX_REFCODE_BUFFSIZE_FALCON	33
#define GENSUBST_PTRN_BUFFSIZE_FALCON	128

/* ===== Type Definitions ===== */
typedef struct DBTRAN25_Context
{
    short   txncode;
    short	fncode;
    char	timestamp[UTCTIMESTAMPLEN+1];
    char	local_timestamp[UTCTIMESTAMPLEN+1];
    char	poscdim;
    char	poschic;
    char	poscham;
    char	poschac;
    char	posoe;
    char	poschp;
    char	poscp;
    double	transactionAmount;
    char	transactionCurrencyCode[3+1];
    short	mcc;
    char	posEntryMode;
    char	authPostFlag;
    FBFR	*adddata_fb;
} DBTRAN25_Context_t;

typedef struct Falcon_msglayout
{
    char *fieldname;
    int pos;
    size_t len;
    int (*p_fb2rawfn)(FBFR *, char *, size_t, DBTRAN25_Context_t *);
} Falcon_msglayout_t;

typedef struct masked_fields
{
    char *name;
    char *(*p_maskfn)(const char *, size_t len);
} masked_fields_t;

typedef struct 
{
    char    wallet_list[800];
    char    wallet_list_toktmp[800];
    char    * listWallet[50];
    char	wallet_list_toktmp_completed[2048];
    char	* listWalletCompleted[50];
    char	tknrequestorid_toktmp[2048];
    char	* listTknRequestorId[50];
    int     sizeListWallet;
    int		sizeListWalletCompleted;
} wallet_list_t;

/* ===== Public Function Declarations ===== */

/**
 * @brief Initialize FALCON2 conversion module
 * @param subsect Configuration subsection name
 * @return SUCCEED / FAIL
 */
ctxpublic int falcon2_init(char *subsect);

/**
 * @brief Uninitialize and free FALCON2 conversion resources
 */
ctxpublic void falcon2_uninit(void);

/**
 * @brief FALCON2 service: convert native message to FML
 * @param p_svc Tuxedo service info
 */
ctxpublic void FALCON2CVIN(TPSVCINFO *p_svc);

/**
 * @brief FALCON2 service: convert FML to native message
 * @param p_svc Tuxedo service info
 */
ctxpublic void FALCON2CVOUT(TPSVCINFO *p_svc);

/**
 * @brief Convert FALCON2 input (linked function)
 * @param p_fb Fielded buffer to convert
 * @return SUCCEED / FAIL
 */
ctxpublic int falcon2cvin(FBFR *p_fb);

/**
 * @brief Convert FALCON2 output (linked function)
 * @param p_fb Fielded buffer to convert
 * @return SUCCEED / FAIL
 */
ctxpublic int falcon2cvout(FBFR *p_fb);

/**
 * @brief Map Falcon decision code to response code
 * @param code Falcon decision code
 * @param actioncode Output action code
 * @param rspcode Output response code
 * @return Result string or NULL
 */
ctxpublic char* falconcode2rsp(char* code, char* actioncode, char* rspcode);

#ifdef __cplusplus
}
#endif

#endif /* FALCON2CV_H */