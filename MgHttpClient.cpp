#include "MgHttpClient.h"


#include "mongoose/mongoose.h"

MgHttpClient::MgHttpClient():
m_bExit( false ),
m_pPReqCallBack( NULL ),
m_pMGR( (struct mg_mgr*)malloc(sizeof(struct mg_mgr)) ),
m_nFileName( 0 ),
m_pszFileName( NULL ),
m_nURL( 0 ),
m_pszURL( NULL ),
m_nExtra_headers( 0 ),
m_pszextra_headers( NULL ),
m_nPost_data( 0 ),
m_pszPost_data( NULL )
{
	mg_mgr_init(m_pMGR, NULL);
}

MgHttpClient::~MgHttpClient()
{
	mg_mgr_free(m_pMGR);
	if ( NULL != m_pMGR ) {
		free( m_pMGR );
		m_pMGR = NULL;
	}
	release();
}

bool MgHttpClient::sendReq(const char* szURL, PReqCallBack pReqCallBack, const char* szFileName, const char *extra_headers, const char *post_data, bool bNONBLOCK)
{
	if ( !isValild( szURL ) )	return false;

	clear();
	setFileName(szFileName);

	m_pPReqCallBack = pReqCallBack;

	if ( bNONBLOCK )
	{
		setURL(szURL);
		set_extra_headers(extra_headers);
		set_post_data(post_data);

		mg_start_thread( process, this );
	}
	else
	{
		mg_connection *nc = mg_connect_http( m_pMGR, ev_handler, szURL, extra_headers, post_data );
		if ( NULL == nc )
			return false;

		nc->user_data = this;

		while ( !m_bExit )
			mg_mgr_poll( m_pMGR, 500 );
	}

	return true;
}

void MgHttpClient::clear()
{
	m_bExit = false;
	m_pPReqCallBack = NULL;

	if (m_nFileName > 0)
		memset(m_pszFileName, 0, m_nFileName);
	if (m_nURL > 0)
		memset(m_pszURL, 0, m_nURL);
	if (m_nExtra_headers > 0)
		memset(m_pszextra_headers, 0, m_nExtra_headers);
	if (m_nPost_data > 0)
		memset(m_pszPost_data, 0, m_nPost_data);
}

void MgHttpClient::release()
{
	if (NULL != m_pszFileName) {
		free(m_pszFileName);
		m_pszFileName = NULL;
	}
	if (NULL != m_pszURL) {
		free(m_pszURL);
		m_pszURL = NULL;
	}
	if (NULL != m_pszextra_headers) {
		free(m_pszextra_headers);
		m_pszextra_headers = NULL;
	}
	if (NULL != m_pszPost_data) {
		free(m_pszPost_data);
		m_pszPost_data = NULL;
	}
}

void MgHttpClient::setFileName(const char* szFileName)
{
	if ( !isValild(szFileName) ) return;
		
	int nlen = strlen(szFileName);
	if ( nlen > m_nFileName )
	{
		if (m_nFileName > 0)
			free( m_pszFileName );
		m_pszFileName = (char*)malloc(nlen + 1);
	}
	
	memcpy(m_pszFileName, szFileName, nlen);
	m_pszFileName[nlen] = '\0';
}

void MgHttpClient::setURL(const char* szURL)
{
	if ( !isValild(szURL) ) return;

	int nlen = strlen(szURL);
	if (nlen > m_nURL)
	{
		if (m_nURL > 0)
			free(m_pszURL);
		m_pszURL = (char*)malloc(nlen + 1);
	}

	memcpy(m_pszURL, szURL, nlen);
	m_pszURL[nlen] = '\0';
}

void MgHttpClient::set_extra_headers(const char *extra_headers)
{
	if ( !isValild(extra_headers) ) return;

	int nlen = strlen(extra_headers);
	if (nlen > m_nExtra_headers)
	{
		if (m_nExtra_headers > 0)
			free(m_pszextra_headers);
		m_pszextra_headers = (char*)malloc(nlen + 1);
	}

	memcpy(m_pszextra_headers, extra_headers, nlen);
	m_pszextra_headers[nlen] = '\0';
}

void MgHttpClient::set_post_data(const char *post_data)
{
	if ( !isValild( post_data) ) return;

	int nlen = strlen(post_data);
	if (nlen > m_nPost_data)
	{
		if (m_nPost_data > 0)
			free(m_pszPost_data);
		m_pszPost_data = (char*)malloc(nlen + 1);
	}

	memcpy(m_pszPost_data, post_data, nlen);
	m_pszPost_data[nlen] = '\0';
}

bool MgHttpClient::isValild(const char* szParam)
{
	return ( (NULL != szParam) && ('\0' != szParam[0]) );
}

//客户端的网络请求响应;
void MgHttpClient::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	http_message *hm = (struct http_message *)ev_data;
	int connect_status;

	MgHttpClient* pThis = (MgHttpClient*)nc->user_data;

	switch (ev)
	{
	case MG_EV_CONNECT:
		connect_status = *(int *)ev_data;
		if (connect_status != 0) {
			printf("Error connecting to server, error code: %d\n", connect_status);
			pThis->m_bExit = true;
		}
		break;
	case MG_EV_HTTP_REPLY:
		{
			printf("Got reply:\n%.\n", (int)hm->body.len);

			nc->flags |= MG_F_CLOSE_IMMEDIATELY;
			// 回调处理;
			if ( NULL != pThis ) {
				if ( NULL != pThis->m_pPReqCallBack )
					pThis->m_pPReqCallBack( hm->body.p, hm->body.len, pThis->m_pszFileName );
				pThis->m_bExit = true;// 每次收到请求后关闭本次连接，重置标记;
			}
		}
		break;
	case MG_EV_CLOSE:
		if ( ( NULL != pThis ) && !pThis->m_bExit) {
			printf("Server closed connection\n");
			pThis->m_bExit = true;
		}
		break;
	default:
		break;
	}
}

void* MgHttpClient::process(void* pData)
{
	if ( NULL == pData ) {
		return NULL;
	}
	MgHttpClient* pThis = (MgHttpClient*)pData;
	if ( NULL == pThis || ( NULL == pThis->m_pMGR ) ) {
		return NULL;
	}

	mg_connection *nc = mg_connect_http(pThis->m_pMGR, ev_handler, pThis->m_pszURL, pThis->m_pszextra_headers, pThis->m_pszPost_data);
	if ( NULL == nc )
		return NULL;

	nc->user_data = pThis;

	while ( !pThis->m_bExit )
		mg_mgr_poll( pThis->m_pMGR, 500 );

	return NULL;
}
