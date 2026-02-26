#include "falcon_3ds.h"

/* ... inside DBTRAN25_layout[] initializer ... */

/* consumerAuthenticationScore (requirement) */
{ "consumerAuthenticationScore", 255, 3,  FFconsumerAuthenticationScore },

/* cavvResult (requirement) – already implemented, keep existing FFcavvResult */
{ "cavvResult",                 267, 1,  FFcavvResult },

/* secondFactorAuthCode (requirement) */
{ "secondFactorAuthCode",       382, 2,  FFsecondFactorAuthCode },

/* userData03 (requirement) – consumer device IP address */
{ "userData03",                 512, 15, FFuserData03 },

/* cavvKeyIndicator (requirement) – already implemented, keep existing FFcavvKeyIndicator */
{ "cavvKeyIndicator",           760, 2,  FFcavvKeyIndicator },

/* eciIndicator (requirement) – REPLACE builder with updated FFeciIndicator */
{ "eciIndicator",               772, 2,  FFeciIndicator },

/* userData07 (requirement) – XID/TAVV */
{ "userData07",                 796, 40, FFuserData07 },

/* authSecondaryVerify (requirement) – derived V / I / blank */
{ "authSecondaryVerify",       309, 1,  FFauthSecondaryVerify },
