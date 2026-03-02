/*-----------------------------------------------------------------------
 * 
 * File		: falcon2cv.c
 *
 * Author	: Ruslans Vasiljevs
 *
 * Created	: 09/05/2022
 *
 * Purpose	: FALCON2 conversion functions implementation
 *
 *-----------------------------------------------------------------------
 * Copyright (c) FIS
 *-----------------------------------------------------------------------*/

#include <portable.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <slfdbg.h>
#include <sldbg.h>
#include <slntp.h>
#include <slnfb.h>
#include <slcfp.h>
#include <slstring.h>
#include <sldtm.h>
#include <slmap.h>
#include <slstrip.h>
#include <slregex.h>

#include <coctry.h>
#include <cocurr.h>

#include <coint.fd.h>
#include <cocrd.fd.h>
#include <token.fd.h>
#include <visa.fd.h>
#include <emv.fd.h>
#include <cointxt.fd.h>
#include <cust_prod.fd.h>

#include <coencpan.h>
#include <cocbfdef.h>
#include <comsgtyp.h>
#include <emv.h>
#include <videfs.h>

#include <enc.h>
#include <forex.h>
#include <diges.h>
#include <fds.h>
#include <falcon2cv.h>
#include <falcon2.h>
#include <visa_common_defs.h>

#include <bpd_prod.fd.h>

#include <dbtokenrh.h>
#include <bpddbtkndeviceprovrh.h>

#include <slstrmsc.h>
#include <comsc.h>

/* ===== Static Global Variables ===== */
ctxprivate char M_forex_ref_dflt[FOREX_REFCODE_BUFFSIZE_FALCON] = {EOS};
ctxprivate char M_forex_ref_fmt[GENSUBST_PTRN_BUFFSIZE_FALCON]  = {EOS};
ctxprivate int 	M_filler_size = DEF_FILLER_SiZE;
ctxprivate char	M_code2rsp[CTX_FILENAME_MAX+1] = {EOS};
ctxprivate short M_gmtoffset = -4;
ctxprivate char	M_ehsource[10+1] = "CORTEX";
ctxprivate char	M_ehdest[10+1] = "FALCON";
ctxprivate char	M_workflow[16+1] = {EOS};
ctxprivate char	M_clientid[16+1] = {EOS};
ctxprivate char	M_onusafes[CTX_FILENAME_MAX] = {EOS};
ctxprivate char	M_unionafes[CTX_FILENAME_MAX] = {EOS};
ctxprivate char	*M_re_code2rsp = NULL;
ctxprivate char M_tmp_wallet_list[800];
ctxprivate wallet_list_t  M_wallet_list;
ctxprivate int M_gettokenadd = 0;
ctxprivate int M_desfalconmsc = 0;

/* ===== Mapping Tables ===== */
ctxprivate map_tbl_t map_tkntype[] =
{
    {TKN_TYPE_CRD_ON_FILE,	"C",	STR},
    {TKN_TYPE_SECURE_ELEM,	"S",	STR},
    {TKN_TYPE_CLOUD_BASED,	"H",	STR},
    {NULL}
};

ctxprivate map_tbl_t map_rspcode[] =
{
    {"000",	"A",	STR},
    {"006",	"B",	STR},
    {"001",	"I",	STR},
    {"108",	"R",	STR},
    {"0..",	"A",	REGEX},
    {"1..",	"D",	REGEX},
    {"2..",	"P",	REGEX},
    {"...",	"D",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_txncode[] =
{
    {"00",	"M",	STR},
    {"01",	"C",	STR},
    {"09",	"B",	STR},
    {"21",	"J",	STR},
    {"30",	"I",	STR},
    {"31",	"I",	STR},
    {"96",	"A",	STR},
    {"..",	"O",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_cvvres[] =
{
    {"N",	"I",	STR},
    {"n",	"I",	STR},
    {"Y",	"V",	STR},
    {"y",	"V",	STR},
    {".",	" ",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_poscdim[] =
{
    {"E",	"C",	STR},
    {"G",	"C",	STR},
    {"S",	"E",	STR},
    {"T",	"E",	STR},
    {"V",	"E",	STR},
    {"A",	"E",	STR},
    {"B",	"E",	STR},
    {"C",	"E",	STR},
    {"1",	"K",	STR},
    {"2",	"S",	STR},
    {"8",	"T",	STR},
    {"9",	"U",	STR},
    {"5",	"V",	STR},
    {"6",	"K",	STR},
    {"7",	"D",	STR},
    {"Y",	"V",	STR},
    {"3",	"X",	STR},
    {"4",	"Y",	STR},
    {".",	" ",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_avsrsp[] =
{
    {"A",	"A",	STR},
    {"N",	"N",	STR},
    {"R",	"R",	STR},
    {"S",	"I",	STR},
    {"W",	"W",	STR},
    {"X",	"Y",	STR},
    {".",	" ",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_poschic[] =
{
    {"5",	"C",	STR},
    {"7",	"D",	STR},
    {"A",	"D",	STR},
    {"B",	"D",	STR},
    {"6",	"K",	STR},
    {"2",	"M",	STR},
    {"9",	"M",	STR},
    {"8",	"N",	STR},
    {"C",	"N",	STR},
    {"D",	"N",	STR},
    {".",	"O",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_reasoncode[] =
{
    {"4021", "3",	STR},
    {"4005", "4",	STR},
    {"4020", "5",	STR},
    {"4007", "6",	STR},
    {"4006", "7",	STR},
    {"4000", "8",	STR},
    {"4362", "9",	STR},
    {"....", " ",	REGEX},
    {NULL}
};

ctxprivate map_tbl_t map_carc[] =
{
    {"2",	"I",	STR},
    {"1",	"V",	STR},
    {".",	" ",	REGEX},
    {NULL}
};

/* ===== Condition Definitions ===== */
ctxprivate fld_cond_t Brejexpdate = { COND_FBOOL, NULL, "I_REJREASON == 700 || I_REJREASON == 708 || I_REJREASON == 705 || I_REJREASON == 723" };
ctxprivate fld_cond_t Brejexpmismatch = { COND_FBOOL, NULL, "I_REJREASON == 708 || I_REJREASON == 705 || I_REJREASON == 723" };
ctxprivate fld_cond_t Brejcvv = { COND_FBOOL, NULL, "I_REJREASON == 602 || I_REJREASON == 698 || I_REJREASON == 703" };
ctxprivate fld_cond_t Brejbadatc = { COND_FBOOL, NULL, "I_REJREASON == 757" };
ctxprivate fld_cond_t Badencrdexp = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '01'" };
ctxprivate fld_cond_t Badeninsuff = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '16'" };
ctxprivate fld_cond_t Badenexclim = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '21'" };
ctxprivate fld_cond_t Badenexcfrq = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '23'" };
ctxprivate fld_cond_t Badennoacty = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '14'" };
ctxprivate fld_cond_t Badenpinreq = { COND_FBOOL, NULL, "C_ACTIONCODE == '1' && C_RSPCODE == '17'" };
ctxprivate fld_cond_t Bisfraud = { COND_FBOOL, NULL, "C_ACTIONCODE == '2' && (C_RSPCODE == '09' || C_RSPCODE == '08' || C_RSPCODE == '02')" };
ctxprivate fld_cond_t Bcirrus = { COND_FBOOL, NULL, "I_SCHEME=='CIRR'" };
ctxprivate fld_cond_t Bpinveri = { COND_FBOOL, NULL, "(I_PINCHKRES == 'N') || (I_PINCHKRES == 'n')" };
ctxprivate fld_cond_t Bpinverv = { COND_FBOOL, NULL, "(I_PINCHKRES == 'Y') || (I_PINCHKRES == 'y')" };
ctxprivate fld_cond_t Bpinverx = { COND_FBOOL, NULL, "(I_PINCHKRES == 'U') || (I_PINCHKRES == 'u') || ((C_POSCHAM == '1') && !(I_PINCHKRES))" };
ctxprivate fld_cond_t Becomm = { COND_FBOOL, NULL, "E_ECOMM_IND %% '21.'" };
ctxprivate fld_cond_t Bmasterpass = { COND_FBOOL, NULL, "E_ECOMM_IND %% '22.'" };
ctxprivate fld_cond_t Bdsrp = { COND_FBOOL, NULL, "E_ECOMM_IND %% '24[123]'" };
ctxprivate fld_cond_t Bcvv2pres = { COND_FBOOL, NULL, "I_CRDCVC" };
ctxprivate fld_cond_t Bcvv2syserr = { COND_FBOOL, NULL, "I_CVV2RES == 'S' || I_CVV2RES == 's'" };
ctxprivate fld_cond_t Bcvv2encerr = { COND_FBOOL, NULL, "I_CVV2RES == 'E' || I_CVV2RES == 'e'" };
ctxprivate fld_cond_t Bcvv2match = { COND_FBOOL, NULL, "I_CVV2RES == 'Y' || I_CVV2RES == 'y'" };
ctxprivate fld_cond_t Bcvv2notmatch = { COND_FBOOL, NULL, "I_CVV2RES == 'N' || I_CVV2RES == 'n'" };
ctxprivate fld_cond_t Bcvv2notperf = { COND_FBOOL, NULL, "I_CVV2RES == 'P' || I_CVV2RES == 'p'" };
ctxprivate fld_cond_t Bisattend = { COND_FBOOL, NULL, "C_POSOE == '1' || C_POSOE == '3'" };
ctxprivate fld_cond_t Bcmpladv = { COND_FBOOL, NULL, "C_MSGCLS == '2' && C_MSGFN == '2' && C_AUTHLIFE > 0" };
ctxprivate fld_cond_t Biscof = { COND_FBOOL, NULL, "C_POSCDIM == 'H' || C_POSCC_89 == 8" };

ctxprivate masked_fields_t M_masked_flds[] =
{
    { "pan",		maskPAN },
    { "cardExpireDate",	maskExpiry },
    { "expandedBIN",	maskBIN },
    {NULL}
};

/* ===== Private Helper Functions ===== */
ctxprivate int rex_group(char *rx, char *str, regmatch_t *garray, int max_groups);
ctxprivate char* regmatch_group_get(char *str, regmatch_t *regmatch, int group);
ctxprivate int	build_hdr(FBFR *p_fb, char *p_buf, long *p_hdrlen);
ctxprivate int	update_hdr_set_datalen(FBFR *p_fb, char *p_buf, long datalen);
ctxprivate int	parse_hdr(char *p_natmsg, FBFR *p_fb, long *p_hdrlen, long *p_datalen);
ctxprivate int	build_data(FBFR *p_fb, char *p_buf, long *p_len);
ctxprivate int	parse_data(char *p_natmsg, long len, FBFR *p_fb);
ctxprivate void falcon_msg_dump(char *p_buf, Falcon_msglayout_t *p_msglayout);
ctxprivate char* dbg_falcon_mask_field(const char *field, char *data, size_t len);
ctxprivate int posEntryModeFallback(FBFR *p_fb, char chic, char cdim, char crdtype, char *res);
ctxprivate ctxbool lookupInList(char *str, char* list);
ctxprivate int cardAipInd(FBFR *p_fb, FLDID fld, char *ind);
ctxprivate char *maskPAN(const char *, size_t len);
ctxprivate char *maskExpiry(const char *, size_t len);
ctxprivate char *maskBIN(const char *, size_t len);
ctxprivate int getTokenInfo(FBFR *p_fb, char *tokenrequestorid);
ctxprivate int getTknRequestorIdList(void);
ctxprivate int completeListRequestorId(char **p_walletListComp, char **p_tknreqidListComp, char wallet[], char resultmsc[]);
ctxprivate int buildTknrequestoridTag(char walletitem[], char tag[]);
ctxprivate int findListTknRequestorid(char tokenrequestorid[], char wallet[]);
ctxprivate int get_datetimexmit_with_gmtoffset(FBFR *p_fb, long *p_date, long *p_time, double gmtoffset);
ctxprivate int timestamp2datetime_with_gmtoffset(char *timestamp, long *p_date, long *p_time, double gmtoffset);
ctxprivate int build_hdr_prov(FBFR *p_fb, char *p_buf, long *p_hdrlen, char subtxncode[]);
ctxprivate int build_data_prov(FBFR *p_fb, char *p_buf, long *p_len, char subtxncode[]);

/* ===== Falcon Field Building Functions ===== */
ctxprivate int FFworkflow(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFrecordType(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFdataSpecificationVersion(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFclientIdFromHeader(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFrecordCreationDate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFrecordCreationTime(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFrecordCreationMilliseconds(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFgmtOffset(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcustomerIdFromHeader(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcustomerAcctNumber(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFexternalTransactionId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFpan(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthPostFlag(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardSeqNum(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardExpireDate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFexpandedBIN(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenizationIndicator(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenExpirationDate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcavvResult(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionDate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionTime(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionAmount(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionCurrencyCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionCurrencyConversionRate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthDecisionCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionType(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmcc(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantPostalCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantCountryCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFpinVerifyCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvvVerifyCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposEntryMode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFpostDate(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFidMethod(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFexternalScore1(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcustomerPresent(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFatmOwner(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenRequestorId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantName(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantCity(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantState(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFrealtimeRequest(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipStatic(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipDynamic(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipVerify(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipRisk(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipIssuerAuthentication(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardAipCombined(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardDailyLimitCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFavailableBalance(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFatmProcessingCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvv2Present(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvv2Response(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFavsResponse(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionCategory(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFacquirerId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFacquirerCountry(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFterminalId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFterminalType(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFterminalEntryCapability(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposConditionCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFatmNetworkId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthExpireDateVerify(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthResponseCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthReversalReason(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthCardIssuer(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFterminalVerificationResults(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardVerificationResults(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcryptogramValid(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFatcCard(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenAssuranceLevel(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcavvKeyIndicator(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFeciIndicator(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFprocessorAuthReasonCode(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFstandinAdvice(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcashbackAmount(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFavsRequest(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvrOfflinePinVerificationPerformed(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvrOfflinePinVerificationFailed(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcvrPinTryLimitExceeded(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposUnattended(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposOffPremises(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposCardCapture(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFauthId(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData02(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);

/* Provisioning functions */
ctxprivate int FFexternalTransactionId_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionType_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFcardExpireDate_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenizationIndicator_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenRequestorId_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData03_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData05_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData06_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData07_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserIndicator02_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserIndicator04_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserIndicator05_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserIndicator06_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserIndicator07_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionAmount_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFsegmentId4_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionCurrencyConversionRate_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantId_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFacquirerCountry_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFacquirerId_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmcc_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtransactionCurrencyCode_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantName_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposEntryMode_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFposUnattended_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFexternalScore1_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFterminalType_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFmerchantCountryCode_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFtokenId_prov(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);

/* OTP provisioning functions */
ctxprivate int FFrecordType_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFdataSpecificationVersion_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFvalidity_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFentityType_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFextSource_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFnotificationName_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFnotificationStatus_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFscore1_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFscore2_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFscore3_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData01_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData02_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData03_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData04_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData05_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData06_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData07_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData08_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData09_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData10_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData12_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData14_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData15_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData16_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData17_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData18_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData27_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData28_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData29_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFuserData30_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);
ctxprivate int FFexternalTransactionId_prov_otp(FBFR *p_fb, char *out, size_t len, DBTRAN25_Context_t *p_ctx);

/* Message layouts */
ctxprivate Falcon_msglayout_t DBTRAN25_layout[] =
{
    { "workflow",				0,	16,	FFworkflow },
    { "recordType",				16,	8,	FFrecordType },
    { "dataSpecificationVersion",		24,	5,	FFdataSpecificationVersion },
    { "clientIdFromHeader",			29,	16,	FFclientIdFromHeader },
    { "recordCreationDate",			45,	8,	FFrecordCreationDate },
    { "recordCreationTime",			53,	6,	FFrecordCreationTime },
    { "recordCreationMilliseconds",		59,	3,	FFrecordCreationMilliseconds },
    { "gmtOffset",				62,	6,	FFgmtOffset },
    { "customerIdFromHeader",		68,	20,	FFcustomerIdFromHeader },
    { "customerAcctNumber",			88,	40,	FFcustomerAcctNumber },
    { "externalTransactionId",		128,	32,	FFexternalTransactionId },
    { "pan",				160,	19,	FFpan },
    { "authPostFlag",			179,	1,	FFauthPostFlag },
    { "cardPostalCode",			180,	9,	NULL },
    { "cardSeqNum",				189,	3,	FFcardSeqNum },
    { "openDate",				192,	8,	NULL },
    { "plasticIssueDate",			200,	8,	NULL },
    { "plasticIssueType",			208,	1,	NULL },
    { "acctExpireDate",			209,	8,	NULL },
    { "cardExpireDate",			217,	8,	FFcardExpireDate },
    { "expandedBIN",			225,	10,	FFexpandedBIN },
    { "dailyCashLimit",			235,	10,	NULL },
    { "tokenizationIndicator",		245,	1,	FFtokenizationIndicator },
    { "tokenExpirationDate",		246,	8,	FFtokenExpirationDate },
    { "consumerAuthenticationScore",	254,	3,	NULL },
    { "incomeOrCashBack",			257,	10,	NULL },
    { "cavvResult",				267,	1,	FFcavvResult },
    { "peerGrouping",			268,	1,	NULL },
    { "transactionDate",			269,	8,	FFtransactionDate },
    { "transactionTime",			277,	6,	FFtransactionTime },
    { "transactionAmount",			283,	13,	FFtransactionAmount },
    { "transactionCurrencyCode",		296,	3,	FFtransactionCurrencyCode },
    { "transactionCurrencyConversionRate",	299,	13,	FFtransactionCurrencyConversionRate },
    { "authDecisionCode",			312,	1,	FFauthDecisionCode },
    { "transactionType",			313,	1,	FFtransactionType },
    { "mcc",				314,	4,	FFmcc },
    { "merchantPostalCode",			318,	9,	FFmerchantPostalCode },
    { "merchantCountryCode",		327,	3,	FFmerchantCountryCode },
    { "pinVerifyCode",			330,	1,	FFpinVerifyCode },
    { "cvvVerifyCode",			331,	1,	FFcvvVerifyCode },
    { "posEntryMode",			332,	1,	FFposEntryMode },
    { "postDate",				333,	8,	FFpostDate },
    { "authPostMiscIndicator",		341,	1,	NULL },
    { "mismatchIndicator",			342,	1,	NULL },
    { "caseCreationIndicator",		343,	1,	NULL },
    { "userIndicator01",			344,	1,	NULL },
    { "userIndicator02",			345,	1,	NULL },
    { "userData01",				346,	10,	NULL },
    { "userData02",				356,	10,	FFuserData02 },
    { "onUsMerchantId",			366,	10,	NULL },
    { "merchantDataProvided",		376,	1,	NULL },
    { "idMethod",				377,	1,	FFidMethod },
    { "externalScore1",			378,	4,	FFexternalScore1 },
    { "externalScore2",			382,	4,	NULL },
    { "externalScore3",			386,	4,	NULL },
    { "customerPresent",			390,	1,	FFcustomerPresent },
    { "atmOwner",				391,	1,	FFatmOwner },
    { "randomDigits",			392,	2,	NULL },
    { "portfolio",				394,	14,	NULL },
    { "tokenRequestorId",			408,	14,	FFtokenRequestorId },
    { "acquirerBin",			422,	6,	NULL },
    { "merchantName",			428,	40,	FFmerchantName },
    { "merchantCity",			468,	30,	FFmerchantCity },
    { "merchantState",			498,	3,	FFmerchantState },
    { "caseSuppressionIndicator",		501,	1,	NULL },
    { "userIndicator03",			502,	5,	NULL },
    { "userIndicator04",			507,	5,	NULL },
    { "userData03",				512,	15,	NULL },
    { "userData04",				527,	20,	NULL },
    { "userData05",				547,	40,	NULL },
    { "realtimeRequest",			587,	1,	FFrealtimeRequest },
    { "padResponse",			588,	1,	NULL },
    { "padActionExpireDate",		589,	8,	NULL },
    { "tokenId",				597,	19,	FFtokenId },
    { "cardAipStatic",			616,	1,	FFcardAipStatic },
    { "cardAipDynamic",			617,	1,	FFcardAipDynamic },
    { "RESERVED_01",			618,	1,	NULL },
    { "cardAipVerify",			619,	1,	FFcardAipVerify },
    { "cardAipRisk",			620,	1,	FFcardAipRisk },
    { "cardAipIssuerAuthentication",	621,	1,	FFcardAipIssuerAuthentication },
    { "cardAipCombined",			622,	1,	FFcardAipCombined },
    { "cardDailyLimitCode",			623,	1,	FFcardDailyLimitCode },
    { "availableBalance",			624,	13,	FFavailableBalance },
    { "availableDailyCashLimit",		637,	13,	NULL },
    { "availableDailyMerchandiseLimit",	650,	13,	NULL },
    { "atmHostMcc",				663,	4,	NULL },
    { "atmProcessingCode",			667,	6,	FFatmProcessingCode },
    { "atmCameraPresent",			673,	1,	NULL },
    { "cardPinType",			674,	1,	NULL },
    { "cardMediaType",			675,	1,	NULL },
    { "cvv2Present",			676,	1,	FFcvv2Present },
    { "cvv2Response",			677,	1,	FFcvv2Response },
    { "avsResponse",			678,	1,	FFavsResponse },
    { "transactionCategory",		679,	1,	FFtransactionCategory },
    { "acquirerId",				680,	12,	FFacquirerId },
    { "acquirerCountry",			692,	3,	FFacquirerCountry },
    { "terminalId",				695,	16,	FFterminalId },
    { "terminalType",			711,	1,	FFterminalType },
    { "terminalEntryCapability",		712,	1,	FFterminalEntryCapability },
    { "posConditionCode",			713,	2,	FFposConditionCode },
    { "atmNetworkId",			715,	1,	FFatmNetworkId },
    { "RESERVED_02",			716,	1,	NULL },
    { "authExpireDateVerify",		717,	1,	FFauthExpireDateVerify },
    { "authSecondaryVerify",		718,	1,	NULL },
    { "authBeneficiary",			719,	1,	NULL },
    { "authResponseCode",			720,	1,	FFauthResponseCode },
    { "authReversalReason",			721,	1,	FFauthReversalReason },
    { "authCardIssuer",			722,	1,	FFauthCardIssuer },
    { "terminalVerificationResults",	723,	10,	FFterminalVerificationResults },
    { "cardVerificationResults",		733,	10,	FFcardVerificationResults },
    { "cryptogramValid",			743,	1,	FFcryptogramValid },
    { "atcCard",				744,	5,	FFatcCard },
    { "atcHost",				749,	5,	NULL },
    { "RESERVED_03",			754,	2,	NULL },
    { "tokenAssuranceLevel",		756,	2,	FFtokenAssuranceLevel },
    { "secondFactorAuthCode",		758,	2,	NULL },
    { "cavvKeyIndicator",			760,	2,	FFcavvKeyIndicator },
    { "recurringAuthExpireDate",		762,	8,	NULL },
    { "linkedAcctType",			770,	1,	NULL },
    { "cardIncentive",			771,	1,	NULL },
    { "eciIndicator",			772,	2,	FFeciIndicator },
    { "cardPinSetDate",			774,	8,	NULL },
    { "processorAuthReasonCode",		782,	5,	FFprocessorAuthReasonCode },
    { "standinAdvice",			787,	1,	FFstandinAdvice },
    { "merchantId",				788,	16,	FFmerchantId },
    { "cardOrder",				804,	1,	NULL },
    { "cashbackAmount",			805,	13,	FFcashbackAmount },
    { "userData06",				818,	13,	NULL },
    { "userData07",				831,	40,	NULL },
    { "paymentInstrumentId",		871,	30,	NULL },
    { "avsRequest",				901,	1,	FFavsRequest },
    { "cvrOfflinePinVerificationPerformed",	902,	1,	FFcvrOfflinePinVerificationPerformed },
    { "cvrOfflinePinVerificationFailed",	903,	1,	FFcvrOfflinePinVerificationFailed },
    { "cvrPinTryLimitExceeded",		904,	1,	FFcvrPinTryLimitExceeded },
    { "posUnattended",			905,	1,	FFposUnattended },
    { "posOffPremises",			906,	1,	FFposOffPremises },
    { "posCardCapture",			907,	1,	FFposCardCapture },
    { "posSecurity",			908,	1,	NULL },
    { "authId",				909,	6,	FFauthId },
    { "userData08",				915,	10,	NULL },
    { "userData09",				925,	10,	NULL },
    { "userIndicator05",			935,	1,	NULL },
    { "userIndicator06",			936,	1,	NULL },
    { "userIndicator07",			937,	5,	NULL },
    { "userIndicator08",			942,	5,	NULL },
    { "modelControl1",			947,	1,	NULL },
    { "modelControl2",			948,	1,	NULL },
    { "modelControl3",			949,	1,	NULL },
    { "modelControl4",			950,	1,	NULL },
    { "RESERVED_04",			951,	3,	NULL },
    { "segmentId1",				954,	6,	NULL },
    { "segmentId2",				960,	6,	NULL },
    { "segmentId3",				966,	6,	NULL },
    { "segmentId4",				972,	6,	NULL },
    {NULL}
};

ctxprivate Falcon_msglayout_t DBTRAN25_layout_prov[] =
{
    { "workflow",				0,	16,	FFworkflow },
    { "recordType",				16,	8,	FFrecordType },
    { "dataSpecificationVersion",		24,	5,	FFdataSpecificationVersion },
    { "clientIdFromHeader",			29,	16,	FFclientIdFromHeader },
    { "recordCreationDate",			45,	8,	FFrecordCreationDate },
    { "recordCreationTime",			53,	6,	FFrecordCreationTime },
    { "recordCreationMilliseconds",		59,	3,	FFrecordCreationMilliseconds },
    { "gmtOffset",				62,	6,	FFgmtOffset },
    { "customerIdFromHeader",		68,	20,	FFcustomerIdFromHeader },
    { "customerAcctNumber",			88,	40,	FFcustomerAcctNumber },
    { "externalTransactionId",		128,	32,	FFexternalTransactionId_prov },
    { "pan",				160,	19,	FFpan },
    { "authPostFlag",			179,	1,	FFauthPostFlag },
    { "cardPostalCode",			180,	9,	NULL },
    { "cardSeqNum",				189,	3,	FFcardSeqNum },
    { "openDate",				192,	8,	NULL },
    { "plasticIssueDate",			200,	8,	NULL },
    { "plasticIssueType",			208,	1,	NULL },
    { "acctExpireDate",			209,	8,	NULL },
    { "cardExpireDate",			217,	8,	FFcardExpireDate_prov },
    { "expandedBIN",			225,	10,	FFexpandedBIN },
    { "dailyCashLimit",			235,	10,	NULL },
    { "tokenizationIndicator",		245,	1,	FFtokenizationIndicator_prov },
    { "tokenExpirationDate",		246,	8,	FFtokenExpirationDate },
    { "consumerAuthenticationScore",	254,	3,	NULL },
    { "incomeOrCashBack",			257,	10,	NULL },
    { "cavvResult",				267,	1,	FFcavvResult },
    { "peerGrouping",			268,	1,	NULL },
    { "transactionDate",			269,	8,	FFtransactionDate },
    { "transactionTime",			277,	6,	FFtransactionTime },
    { "transactionAmount",			283,	13,	FFtransactionAmount_prov },
    { "transactionCurrencyCode",		296,	3,	FFtransactionCurrencyCode_prov },
    { "transactionCurrencyConversionRate",	299,	13,	FFtransactionCurrencyConversionRate_prov },
    { "authDecisionCode",			312,	1,	FFauthDecisionCode },
    { "transactionType",			313,	1,	FFtransactionType_prov },
    { "mcc",				314,	4,	FFmcc_prov },
    { "merchantPostalCode",			318,	9,	FFmerchantPostalCode },
    { "merchantCountryCode",		327,	3,	FFmerchantCountryCode_prov },
    { "pinVerifyCode",			330,	1,	FFpinVerifyCode },
    { "cvvVerifyCode",			331,	1,	FFcvvVerifyCode },
    { "posEntryMode",			332,	1,	FFposEntryMode_prov },
    { "postDate",				333,	8,	FFpostDate },
    { "authPostMiscIndicator",		341,	1,	NULL },
    { "mismatchIndicator",			342,	1,	NULL },
    { "caseCreationIndicator",		343,	1,	NULL },
    { "userIndicator01",			344,	1,	NULL },
    { "userIndicator02",			345,	1,	FFuserIndicator02_prov },
    { "userData01",				346,	10,	NULL },
    { "userData02",				356,	10,	NULL },
    { "onUsMerchantId",			366,	10,	NULL },
    { "merchantDataProvided",		376,	1,	NULL },
    { "idMethod",				377,	1,	FFidMethod },
    { "externalScore1",			378,	4,	FFexternalScore1_prov },
    { "externalScore2",			382,	4,	NULL },
    { "externalScore3",			386,	4,	NULL },
    { "customerPresent",			390,	1,	FFcustomerPresent },
    { "atmOwner",				391,	1,	FFatmOwner },
    { "randomDigits",			392,	2,	NULL },
    { "portfolio",				394,	14,	NULL },
    { "tokenRequestorId",			408,	14,	FFtokenRequestorId_prov },
    { "acquirerBin",			422,	6,	NULL },
    { "merchantName",			428,	40,	FFmerchantName_prov },
    { "merchantCity",			468,	30,	NULL },
    { "merchantState",			498,	3,	NULL },
    { "caseSuppressionIndicator",		501,	1,	NULL },
    { "userIndicator03",			502,	5,	NULL },
    { "userIndicator04",			507,	5,	FFuserIndicator04_prov },
    { "userData03",				512,	15,	FFuserData03_prov },
    { "userData04",				527,	20,	NULL },
    { "userData05",				547,	40,	FFuserData05_prov },
    { "realtimeRequest",			587,	1,	FFrealtimeRequest },
    { "padResponse",			588,	1,	NULL },
    { "padActionExpireDate",		589,	8,	NULL },
    { "tokenId",				597,	19,	FFtokenId_prov },
    { "cardAipStatic",			616,	1,	FFcardAipStatic },
    { "cardAipDynamic",			617,	1,	FFcardAipDynamic },
    { "RESERVED_01",			618,	1,	NULL },
    { "cardAipVerify",			619,	1,	FFcardAipVerify },
    { "cardAipRisk",			620,	1,	FFcardAipRisk },
    { "cardAipIssuerAuthentication",	621,	1,	FFcardAipIssuerAuthentication },
    { "cardAipCombined",			622,	1,	FFcardAipCombined },
    { "cardDailyLimitCode",			623,	1,	FFcardDailyLimitCode },
    { "availableBalance",			624,	13,	FFavailableBalance },
    { "availableDailyCashLimit",		637,	13,	NULL },
    { "availableDailyMerchandiseLimit",	650,	13,	NULL },
    { "atmHostMcc",				663,	4,	NULL },
    { "atmProcessingCode",			667,	6,	FFatmProcessingCode },
    { "atmCameraPresent",			673,	1,	NULL },
    { "cardPinType",			674,	1,	NULL },
    { "cardMediaType",			675,	1,	NULL },
    { "cvv2Present",			676,	1,	FFcvv2Present },
    { "cvv2Response",			677,	1,	FFcvv2Response },
    { "avsResponse",			678,	1,	FFavsResponse },
    { "transactionCategory",		679,	1,	FFtransactionCategory },
    { "acquirerId",				680,	12,	FFacquirerId_prov },
    { "acquirerCountry",			692,	3,	FFacquirerCountry_prov },
    { "terminalId",				695,	16,	FFterminalId },
    { "terminalType",			711,	1,	FFterminalType_prov },
    { "terminalEntryCapability",		712,	1,	FFterminalEntryCapability },
    { "posConditionCode",			713,	2,	FFposConditionCode },
    { "atmNetworkId",			715,	1,	FFatmNetworkId },
    { "RESERVED_02",			716,	1,	NULL },
    { "authExpireDateVerify",		717,	1,	FFauthExpireDateVerify},
    { "authSecondaryVerify",		718,	1,	NULL },
    { "authBeneficiary",			719,	1,	NULL },
    { "authResponseCode",			720,	1,	FFauthResponseCode },
    { "authReversalReason",			721,	1,	FFauthReversalReason },
    { "authCardIssuer",			722,	1,	FFauthCardIssuer },
    { "terminalVerificationResults",	723,	10,	FFterminalVerificationResults },
    { "cardVerificationResults",		733,	10,	FFcardVerificationResults },
    { "cryptogramValid",			743,	1,	FFcryptogramValid },
    { "atcCard",				744,	5,	FFatcCard },
    { "atcHost",				749,	5,	NULL },
    { "RESERVED_03",			754,	2,	NULL },
    { "tokenAssuranceLevel",		756,	2,	FFtokenAssuranceLevel },
    { "secondFactorAuthCode",		758,	2,	NULL },
    { "cavvKeyIndicator",			760,	2,	FFcavvKeyIndicator },
    { "recurringAuthExpireDate",		762,	8,	NULL },
    { "linkedAcctType",			770,	1,	NULL },
    { "cardIncentive",			771,	1,	NULL },
    { "eciIndicator",			772,	2,	FFeciIndicator },
    { "cardPinSetDate",			774,	8,	NULL },
    { "processorAuthReasonCode",		782,	5,	FFprocessorAuthReasonCode },
    { "standinAdvice",			787,	1,	FFstandinAdvice },
    { "merchantId",				788,	16,	FFmerchantId_prov },
    { "cardOrder",				804,	1,	NULL },
    { "cashbackAmount",			805,	13,	FFcashbackAmount },
    { "userData06",				818,	13,	FFuserData06_prov },
    { "userData07",				831,	40,	FFuserData07_prov },
    { "paymentInstrumentId",		871,	30,	NULL },
    { "avsRequest",				901,	1,	FFavsRequest },
    { "cvrOfflinePinVerificationPerformed",	902,	1,	FFcvrOfflinePinVerificationPerformed },
    { "cvrOfflinePinVerificationFailed",	903,	1,	FFcvrOfflinePinVerificationFailed },
    { "cvrPinTryLimitExceeded",		904,	1,	FFcvrPinTryLimitExceeded },
    { "posUnattended",			905,	1,	FFposUnattended_prov },
    { "posOffPremises",			906,	1,	FFposOffPremises },
    { "posCardCapture",			907,	1,	FFposCardCapture },
    { "posSecurity",			908,	1,	NULL },
    { "authId",				909,	6,	FFauthId },
    { "userData08",				915,	10,	NULL },
    { "userData09",				925,	10,	NULL },
    { "userIndicator05",			935,	1,	FFuserIndicator05_prov },
    { "userIndicator06",			936,	1,	FFuserIndicator06_prov },
    { "userIndicator07",			937,	5,	FFuserIndicator07_prov },
    { "userIndicator08",			942,	5,	NULL },
    { "modelControl1",			947,	1,	NULL },
    { "modelControl2",			948,	1,	NULL },
    { "modelControl3",			949,	1,	NULL },
    { "modelControl4",			950,	1,	NULL },
    { "RESERVED_04",			951,	3,	NULL },
    { "segmentId1",				954,	6,	NULL },
    { "segmentId2",				960,	6,	NULL },
    { "segmentId3",				966,	6,	NULL },
    { "segmentId4",				972,	6,	FFsegmentId4_prov },
    {NULL}
};

ctxprivate Falcon_msglayout_t DBTRAN25_layout_prov_otp[] =
{
    { "workflow",				0,	16,	FFworkflow },
    { "recordType",				16,	8,	FFrecordType_prov_otp },
    { "dataSpecificationVersion",		24,	5,	FFdataSpecificationVersion_prov_otp },
    { "clientIdFromHeader",			29,	16,	FFclientIdFromHeader },
    { "recordCreationDate",			45,	8,	FFrecordCreationDate },
    { "recordCreationTime",			53,	6,	FFrecordCreationTime },
    { "recordCreationMilliseconds",		59,	3,	FFrecordCreationMilliseconds },
    { "gmtOffset",				62,	6,	FFgmtOffset },
    { "customerIdFromHeader",		68,	20,	FFcustomerIdFromHeader },
    { "customerAcctNumber",			88,	40,	FFcustomerAcctNumber },
    { "externalTransactionId",		128,	32,	FFexternalTransactionId_prov_otp },
    { "serviceId",					160,	19,	FFpan },
    { "transactionDate",			179,	8,	FFtransactionDate },
    { "transactionTime",			187,	6,	FFtransactionTime },
    { "validity",					193,	4,	FFvalidity_prov_otp },
    { "entityType",					197,	4,	FFentityType_prov_otp },
    { "extSource",					201,	48,	FFextSource_prov_otp },
    { "notificationName",			249,	48,	FFnotificationName_prov_otp },
    { "notificationStatus",			297,	10,	FFnotificationStatus_prov_otp },
    { "score1",						307,	4,	FFscore1_prov_otp },
    { "score2",						311,	4,	FFscore2_prov_otp },
    { "score3",						315,	4,	FFscore3_prov_otp },
    { "userData01",					319,	4,	FFuserData01_prov_otp },
    { "userData02",					323,	4,	FFuserData02_prov_otp },
    { "userData03",					327,	4,	FFuserData03_prov_otp },
    { "userData04",					331,	4,	FFuserData04_prov_otp },
    { "userData05",					335,	4,	FFuserData05_prov_otp },
    { "userData06",					339,	8,	FFuserData06_prov_otp },
    { "userData07",					347,	8,	FFuserData07_prov_otp },
    { "userData08",					355,	8,	FFuserData08_prov_otp },
    { "userData09",					363,	8,	FFuserData09_prov_otp },
    { "userData10",					371,	8,	FFuserData10_prov_otp },
    { "userData11",					379,	8,	NULL },
    { "userData12",					387,	16,	FFuserData12_prov_otp },
    { "userData13",					403,	16,	NULL },
    { "userData14",					419,	16,	FFuserData14_prov_otp },
    { "userData15",					435,	16,	FFuserData15_prov_otp },
    { "userData16",					451,	16,	FFuserData16_prov_otp },
    { "userData17",					467,	16,	FFuserData17_prov_otp },
    { "userData18",					483,	32,	FFuserData18_prov_otp },
    { "userData19",					515,	32,	NULL },
    { "userData20",					547,	32,	NULL },
    { "userData21",					579,	32,	NULL },
    { "userData22",					611,	32,	NULL },
    { "userData23",					643,	32,	NULL },
    { "userData24",					675,	32,	NULL },
    { "userData25",					707,	32,	NULL },
    { "userData26",					739,	32,	NULL },
    { "userData27",					771,	32,	FFuserData27_prov_otp },
    { "userData28",					803,	64,	FFuserData28_prov_otp },
    { "userData29",					867,	64,	FFuserData29_prov_otp },
    { "userData30",					931,	64,	FFuserData30_prov_otp },
    { "userData31",					995,	64,	NULL },
    { "userData32",					1059,	64,	NULL },
    { "userData33",					1123,	255,	NULL },
    { "userData34",					1378,	255,	NULL },
    {NULL}
};

/* ===== IMPLEMENTATION - Main Module Functions ===== */

ctxpublic int falcon2_init(char *subsect)
{
    int ret = SUCCEED;
    char *tagnm = "FALCON2";
    FILE *fp = NULL;
    
    ctxprivate cfp_parm cfp[] =
    {
        {"CODE2RSP",		parm_string, sizeof(M_code2rsp),	TRUE,	M_code2rsp,		0},
        {"EH_SOURCE",		parm_string, sizeof(M_ehsource),	FALSE,	M_ehsource,		0},
        {"EH_DEST",		parm_string, sizeof(M_ehdest),		FALSE,	M_ehdest,		0},
        {"WORKFLOW",		parm_string, sizeof(M_workflow),	FALSE,	M_workflow,		0},
        {"CLIENTID",		parm_string, sizeof(M_clientid),	FALSE,	M_clientid,		0},
        {"ONUSAFES",		parm_string, sizeof(M_onusafes),	FALSE,	M_onusafes,		0},
        {"UNIONAFES",		parm_string, sizeof(M_unionafes),	FALSE,	M_unionafes,		0},
        {"GMTOFFSET",		parm_short,  0,				FALSE,	(char *)&M_gmtoffset,	0},
        {"GETTOKENADD", 	parm_int, 	0, 			TRUE,	(void *)&M_gettokenadd, 0},
        {"DESFALCONMSC", 	parm_int, 	0, 			TRUE,	(void *)&M_desfalconmsc, 0},
        {"WALLETLIST", 		parm_string, sizeof(M_tmp_wallet_list), TRUE,(void *)&M_tmp_wallet_list, 0},
        {0}
    };

    fp = cfp_open();
    if (fp)
    {
        ret = cfp_parse_nodebug(fp, cfp, tagnm, subsect);
        if (SUCCEED != ret)
        {
            DBG_PRINTF((dbg_syserr, "falcon2_init failed [%s/%s] (%s)",
                tagnm, subsect, cfp_errparm()));
        }
        else if (SUCCEED != rex_comp(&M_re_code2rsp, "^(.+):(.+)$", 0))
        {
            DBG_PRINTF((dbg_syserr, "compile REGEX failed"));
            ret = FAIL;
        }
        fclose(fp);
    }
    else
    {
        DBG_PRINTF((dbg_syserr, "cfp_open failed"));
        ret = FAIL;
    }
    
    if (SUCCEED == ret) 
    {
        DBG_PRINTF((dbg_progdetail, "getTknRequestorIdList..."));
        ret = getTknRequestorIdList();
    }
    
    if (SUCCEED == ret)
    {
        DBG_PRINTF((dbg_progdetail, "sizeListWalletCompleted[%d]", M_wallet_list.sizeListWalletCompleted));
        for (int i=0; i<M_wallet_list.sizeListWalletCompleted; i++)
        {
            DBG_PRINTF((dbg_progdetail, "id[%d] wallet[%s] tknrequestorid[%s]", i, M_wallet_list.listWalletCompleted[i], M_wallet_list.listTknRequestorId[i]));
        }
    }
    
    return ret;
}

ctxpublic void falcon2_uninit(void)
{
    if (NULL != M_re_code2rsp)
    {
        rex_free(M_re_code2rsp);
    }
}

ctxpublic void FALCON2CVIN(TPSVCINFO *p_svc)
{
    int		ret = SUCCEED;
    static FBFR	*p_fb;

    DBG_TIMER(dbg_proginfo, "FALCON2CVIN START");
    p_fb = MAKE_BIGGER(p_svc->data);
    ret = falcon2cvin(p_fb);
    DBG_TIMER(dbg_proginfo, "FALCON2CVIN END");
    DBG_FLUSH();
    ntp_return(SUCCEED == ret ? TPSUCCESS : TPFAIL, 0L, (char *)p_fb, 0L, 0L);
}

ctxpublic int falcon2cvin(FBFR *p_fb)
{
    int 		ret = SUCCEED;
    char 		*p_natmsg;
    char		*p_tmpbuf = NULL;
    FLDLEN		natmsglen;
    long		hdrlen = FALCON2_HEADER_LEN;
    long 		datalen = 0;

    if (NULL == (p_natmsg = F_find(p_fb, I_NATMSG, 0, &natmsglen ) ) )
        ret = FAIL;
    else if (!(p_tmpbuf = malloc((size_t)natmsglen)))
    {
        DBG_PRINTF((dbg_syserr, "malloc failed"));
        ret = FAIL;
    }
    else
    {
        memcpy(p_tmpbuf, p_natmsg, (size_t)natmsglen);
        if (DBG_GETLEV() > dbg_proginfo)
        {
            DBG_DUMP(dbg_progdetail, "IN MSG", p_tmpbuf, natmsglen);		
        }
    }
    
    if (SUCCEED == ret)
    {
        ret = parse_hdr(p_tmpbuf, p_fb, &hdrlen, &datalen);
    }

    if (SUCCEED == ret)
    {
        ret = parse_data(p_tmpbuf + hdrlen, datalen, p_fb);
    }

    if (FAIL != ret)
         DBG_DUMPFB(dbg_progdetail,"FALCON2CVIN returns:", p_fb);
    else			
         DBG_PRINTF((dbg_syserr, "FALCON2CVIN convert failed"));

    free(p_tmpbuf);

    return ret;
}

ctxpublic void FALCON2CVOUT(TPSVCINFO *p_svc)
{
     int	 	ret = SUCCEED;
     static FBFR	*p_fb;

    DBG_TIMER(dbg_proginfo, "FALCON2CVOUT START");
     p_fb = MAKE_BIGGER(p_svc->data);
    ret = falcon2cvout(p_fb);
    DBG_TIMER(dbg_proginfo, "FALCON2CVOUT END");
     DBG_FLUSH();
    ntp_return(SUCCEED == ret ? TPSUCCESS : TPFAIL, 0L, (char *)p_fb, 0L, 0L);
}

ctxpublic int falcon2cvout(FBFR *p_fb)
{
    char	buf[BUF_SIZE];
     long	datalen = 0;
    long	hdrlen = FALCON2_HEADER_LEN;
    char subtxncode[20];
     int	ret = SUCCEED;

    memset(buf, ' ', sizeof(buf));
    DBG_DUMPFB(dbg_progdetail,"FALCON2CVOUT converting:", p_fb);
    
    if  (F_pres(p_fb, D_WALLETPROVIDERID, 0))
    {
        DBG_PRINTF((dbg_progdetail,"converting provisioning..."));
        
        if (SUCCEED != CF_get(p_fb, N_SUB_TXNCODE, 0, (char *)&subtxncode, 0, FLD_STRING))
        {
            DBG_PRINTF((dbg_syserr, "Failed to get N_SUB_TXNCODE"));
            ret = FAIL;
        }
        else
        {
            DBG_PRINTF((dbg_syserr,"N_SUB_TXNCODE[%s]", subtxncode));

            if ( SUCCEED != (ret=build_hdr_prov(p_fb, buf, &hdrlen, subtxncode)) )
            {
                DBG_PRINTF((dbg_syserr,"build_hdr failed"));
            }
            
            DBG_PRINTF((dbg_progdetail,"calling build_data_prov ..."));
            
            if ( SUCCEED != (ret=build_data_prov(p_fb, buf + hdrlen, &datalen, subtxncode)) )
            {
                DBG_PRINTF((dbg_syserr,"build_data_prov failed"));
            }
            else
            {
                ret = update_hdr_set_datalen(p_fb, buf, datalen+FALCON2_HEADER_RESERVED_1_LEN);
            }
            
            if (SUCCEED == ret && DBG_GETLEV() > dbg_proginfo)
            {
                DBG_DUMP(dbg_progdetail,"Msg header:", buf, (size_t)hdrlen);
                DBG_DUMP(dbg_progdetail,"Full message:", buf, (size_t)datalen + hdrlen);
            }

            if (SUCCEED == ret && FAIL == F_chg(p_fb, I_NATMSG, 0, buf, (FLDLEN)datalen + hdrlen))
            {
                DBG_PRINTF((dbg_syserr,"Failed add I_NATMSG"));
                ret = FAIL;
            }
        }
    }
    else 
    {
        if ( SUCCEED != (ret=build_hdr(p_fb, buf, &hdrlen)) )
        {
            DBG_PRINTF((dbg_syserr,"build_hdr failed"));
        }

        if ( SUCCEED == ret && (FBISAUTH(p_fb) || FBISFIN(p_fb) || FBISREVCB(p_fb)) )
        {
            DBG_PRINTF((dbg_progdetail,"calling build_data..."));
            
            if ( SUCCEED != (ret=build_data(p_fb, buf + hdrlen, &datalen)) )
            {
                DBG_PRINTF((dbg_syserr,"build_data failed"));
            }
            else
            {
                ret = update_hdr_set_datalen(p_fb, buf, datalen+FALCON2_HEADER_RESERVED_1_LEN);
            }
        }
        
        if (SUCCEED == ret && DBG_GETLEV() > dbg_proginfo)
        {
            DBG_DUMP(dbg_progdetail,"Msg header:", buf, (size_t)hdrlen);
            DBG_DUMP(dbg_progdetail,"Full message:", buf, (size_t)datalen + hdrlen);
        }

        if (SUCCEED == ret && FAIL == F_chg(p_fb, I_NATMSG, 0, buf, (FLDLEN)datalen + hdrlen))
        {
            DBG_PRINTF((dbg_syserr,"Failed add I_NATMSG"));
            ret = FAIL;
        }
    }

    return ret;
}

ctxpublic char* falconcode2rsp(char* code, char* actioncode, char* rspcode)
{
    char *item;
    char *group;
    char *result = NULL;
    regmatch_t regmatch[2+1];
    char tmp_code2rsp[CTX_FILENAME_MAX+1] = {EOS};

    if (NULL == code || EOS == code[0])
    {
        return NULL;
    }

    strscpy(tmp_code2rsp, M_code2rsp);
    item = strtok(tmp_code2rsp,",");
    while(NULL != item)
    {
        if (SUCCEED == rex_group(M_re_code2rsp, item, regmatch, 2+1))
        {
            group = regmatch_group_get(item, regmatch, 1);
            if (0 == strcmp(stp_both(group), code))
            {
                result = regmatch_group_get(item, regmatch, 2);
            }
            if (NULL != result)
            {
                stp_both(result);
                if (NULL != actioncode)
                {
                    *actioncode = result[0];
                }
                if (NULL != rspcode)
                {
                    slstrncpy(rspcode, ++result, 2);
                }
                break;
            }
        }

        item = strtok(NULL, ",");
    }

    return result;
}

/* ... Implementation continues in separate sections ... */
/* Include remaining functions from original file (build_hdr, parse_hdr, build_data, parse_data, 
   all FF* field building functions, provisioning functions, helper functions, etc.) */
