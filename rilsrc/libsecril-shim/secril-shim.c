#include "secril-shim.h"

/* A copy of the original RIL function table. */
static const RIL_RadioFunctions *origRilFunctions;

/* A copy of the ril environment passed to RIL_Init. */
static const struct RIL_Env *rilEnv;

static void onRequestDial(int request, void *data, size_t datalen, RIL_Token t) {
	RIL_Dial *dial = malloc(sizeof(RIL_Dial));
	RIL_UUS_Info *uusInfo = malloc(sizeof(RIL_UUS_Info));

	memcpy(dial, data, datalen);

	if (dial->uusInfo == NULL) {
		memset(uusInfo, 0, sizeof(RIL_UUS_Info));
		uusInfo->uusType = (RIL_UUS_Type) 0;
		uusInfo->uusDcs = (RIL_UUS_DCS) 0;
		uusInfo->uusData = NULL;
		uusInfo->uusLength = 0;
		dial->uusInfo = uusInfo;
	}

	origRilFunctions->onRequest(request, dial, sizeof(RIL_Dial), t);

	free(uusInfo);
	free(dial);
}

static void onRequestShim(int request, void *data, size_t datalen, RIL_Token t)
{
	switch (request) {
		/* The Samsung RIL crashes if uusInfo is NULL... */
		case RIL_REQUEST_DIAL:
			if (datalen == sizeof(RIL_Dial) && data != NULL) {
				onRequestDial(request, data, datalen, t);
				return;
			}
			break;
	}

	RLOGD("%s: got request %s: forwarded to RIL.\n", __func__, requestToString(request));
	origRilFunctions->onRequest(request, data, datalen, t);
}

static void onCompleteQueryAvailableNetworks(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
	/* Response is a char **, pointing to an array of char *'s */
	size_t numStrings = responselen / sizeof(char *);
	size_t numNeededStrings = numStrings - (numStrings / 5);
	size_t newResponseLen = numNeededStrings * sizeof(char *);

	void *newResponse = malloc(newResponseLen);

	/* Remove every 5th string (qan element) */
	char **p_cur = (char **) response;
	char **p_new = (char **) newResponse;
	size_t i, j;
	for (i = 0, j = 0; i < numStrings; ++i) {
		if ((i + 1) % 5 != 0) {
			p_new[j] = p_cur[i];
			++j;
		}
	}

	/* Send the fixed response to libril */
	rilEnv->OnRequestComplete(t, e, newResponse, newResponseLen);

	free(newResponse);
}

static void onRequestCompleteShim(RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
	int request;
	RequestInfo *pRI;

	pRI = (RequestInfo *)t;

	/* If pRI is null, this entire function is useless. */
	if (pRI == NULL)
		goto null_token_exit;

	request = pRI->pCI->requestNumber;

	switch (request) {
		case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
			/* Remove the extra (unused) element from the operator info, freaking out the framework.
			 * Formerly, this is know as the mQANElements override. */
			if (response != NULL && responselen != 0 && (responselen % sizeof(char *) == 0)) {
				onCompleteQueryAvailableNetworks(t, e, response, responselen);
				return;
			}
			break;
	}

	RLOGD("%s: got request %s: forwarded to libril.\n", __func__, requestToString(request));
null_token_exit:
	rilEnv->OnRequestComplete(t, e, response, responselen);
}

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	RIL_RadioFunctions const* (*origRilInit)(const struct RIL_Env *env, int argc, char **argv);
	static RIL_RadioFunctions shimmedFunctions;
	static struct RIL_Env shimmedEnv;
	void *origRil;

	/* Shim the RIL_Env passed to the real RIL, saving a copy of the original */
	rilEnv = env;
	shimmedEnv = *env;
	shimmedEnv.OnRequestComplete = onRequestCompleteShim;

	/* Open and Init the original RIL. */

	origRil = dlopen(RIL_LIB_PATH, RTLD_LOCAL);
	if (CC_UNLIKELY(!origRil)) {
		RLOGE("%s: failed to load '" RIL_LIB_PATH  "': %s\n", __func__, dlerror());
		return NULL;
	}

	origRilInit = dlsym(origRil, "RIL_Init");
	if (CC_UNLIKELY(!origRilInit)) {
		RLOGE("%s: couldn't find original RIL_Init!\n", __func__);
		goto fail_after_dlopen;
	}

	origRilFunctions = origRilInit(&shimmedEnv, argc, argv);
	if (CC_UNLIKELY(!origRilFunctions)) {
		RLOGE("%s: the original RIL_Init derped.\n", __func__);
		goto fail_after_dlopen;
	}

	/* Shim functions as needed. */
	shimmedFunctions = *origRilFunctions;
	shimmedFunctions.onRequest = onRequestShim;

	return &shimmedFunctions;

fail_after_dlopen:
	dlclose(origRil);
	return NULL;
}
