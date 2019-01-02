#ifndef __MG_HTTP_CLIENT_H__
#define __MG_HTTP_CLIENT_H__

class MgHttpClient
{
public:
	typedef bool(*PReqCallBack)( const char* szBuffer, int nlen, const char* szFileName );
private:
	bool				m_bExit;			//每次收到请求后关闭本次连接,重置标记;
	PReqCallBack		m_pPReqCallBack;	//回调函数;
	struct mg_mgr*		m_pMGR;				//Mongoose event manager pointer;

	int					m_nFileName;		//FileName length;
	char*				m_pszFileName;		//url返回的内容保存起来的文件名;
	//以下NONBLOCK使用;
	int					m_nURL;				//URL LENGTH;
	char*				m_pszURL;			//url;
	int					m_nExtra_headers;	//extra_headers length;
	char*				m_pszextra_headers; //extra_headers;
	int					m_nPost_data;		//post_data length;
	char*				m_pszPost_data;		//post_data;
public:
	MgHttpClient();
	~MgHttpClient();

	bool isFinish() const { return m_bExit; }
	bool sendReq( const char* szURL, PReqCallBack pReqCallBack, const char* szFileName, const char *extra_headers, const char *post_data, bool bNONBLOCK=true);


private:
	void clear();
	void release();

	void setFileName( const char* szFileName );
	void setURL( const char* szURL );
	void set_extra_headers( const char *extra_headers );
	void set_post_data( const char *post_data );

	bool isValild( const char* szParam );

private:
	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

	//thread function;
	static void* process( void* pData );
};

#endif