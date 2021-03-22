%{
#include <string.h>
#include "pds.h"

static const unsigned char *InputBuffer = NULL;
static size_t InputSize = 0;
static size_t InputPos = 0;

static int  pdsNRD;
static unsigned  pdsPort;
static char pdsType[16];
static char pdsServer[1024];
static char pdsPath[1024];

static void init_internal(void);

#define YY_INPUT(buf, result, max_size) \
	{ \
		if (InputBuffer == NULL) { \
			result = YY_NULL; \
		} else if (InputPos >= InputSize) { \
		 	result = YY_NULL; \
		} else if (InputPos + max_size > InputSize) { \
			result = InputSize - InputPos; \
			memcpy(buf, InputBuffer + InputPos, result); \
			InputPos += result; \
		} else { \
			result = max_size; \
			memcpy(buf, InputBuffer + InputPos, result); \
			InputPos += result; \
		} \
	}
#define YY_DECL int nuspds_cfglex(const unsigned char *text, size_t textsize)
#define YY_NO_UNPUT
#define YY_NEVER_INTERACTIVE 1
#define STRING_SAVE(buf, size) \
	(strncpy((buf), yytext, size), (buf)[size - 1] = '\0')

#undef stdin
#define stdin NULL

%}

SP		[\ \t]
WORD		[_0-9A-Za-z]
W4		{WORD}{WORD}{WORD}{WORD}
W8		{W4}{W4}
HOSTNAME	([0-9A-Za-z][-0-9A-Za-z]*)
IP		{HOSTNAME}(\.{HOSTNAME})*
SPE		[\ \t\r\n]
PATHSEGM	([-_\.0-9A-Za-z~]+)
PATH		{PATHSEGM}(\/{PATHSEGM})*

%s typa typz srva hostz portz patha httpz hthostz hturlz pathz nrda nrdz err
%option noyywrap
%option prefix="nuspds_cfg"
%%
	BEGIN typa;
	InputBuffer = text;
	InputSize = textsize;
	init_internal();

<INITIAL,typa>{W8}\.{W4}.{WORD}+ {
	memcpy(pdsType + 0, yytext + 0, 8);
	memcpy(pdsType + 8, yytext + 9, 4);
	memcpy(pdsType + 12, yytext + 14, 4);
	BEGIN(typz);
	}
<typz>{SP}+ {
	BEGIN(srva);
	}
<srva>{IP} {
	STRING_SAVE(pdsServer, sizeof pdsServer);
	BEGIN(hostz);
	}
<hostz>:[0-9]+ {
	pdsPort = strtoul(yytext + 1, NULL, 0);
	BEGIN(portz);
	}
<hostz,portz>{SP}+ {
	BEGIN(patha);
	}
<patha>{PATH} {
	pdsPath[0] = '/';
	STRING_SAVE(pdsPath + 1, sizeof pdsPath - 1);
	BEGIN(pathz);
	}
<srva>http:\/\/ {
	BEGIN(httpz);
	}
<httpz>{IP} {
	STRING_SAVE(pdsServer, sizeof pdsServer);
	BEGIN(hthostz);
	}
<hthostz>:[0-9]+ {
	pdsPort = strtoul(yytext + 1, NULL, 0);
	BEGIN(hturlz);
	}
<patha,hthostz,hturlz>\/{PATH} {
	STRING_SAVE(pdsPath, sizeof pdsPath);
	BEGIN(pathz);
	}
<pathz>{SP}+ {
	BEGIN(nrda);
	}
<nrda>[0-9]+ {
	pdsNRD = atoi(yytext);
	BEGIN(nrdz);
	}
<pathz,nrdz>{SP}*(#[^\n]*)?\n {
	nuspds_dsfound(pdsNRD, pdsType, pdsServer, pdsPort, pdsPath);
	BEGIN(typa);
	init_internal();
	}
<err>\n {
	BEGIN(typa);
	init_internal();
	}
. {
	BEGIN(err);
	}
%%

	static void
init_internal(void)
{
	pdsNRD = 99;
	pdsPort = 8090;
	pdsPath[0] = '\0';
	pdsServer[0] = '\0';
}
