#pragma once
#ifdef Deprecated_iYIhv7irlXIcY
#include "curl/curl.h"
namespace Common_functions
{
	class Libcurl_custom
	{
        /// <summary>
        /// Struct as modules
        /// initializer and deconstructor for setting enviorment
        /// so declare struct at program lifecyle
        /// </summary>
    private:
        bool Libcurl_has_int = false;//only runs if you are using this code correctly
        std::string remote_user_agent ="";//I was planning to fetch from internet api and use this instead of the default one by checking if this was empty
        std::string default_user_agent()
        {
            if (remote_user_agent.empty())
            {
                return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36 Edg/86.0.622.51";
            }
            else
            {
                return remote_user_agent;
            }
        }

        static size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }
    public:
        std::string HttpGetString(std::string url)
        {
            if (Libcurl_has_int == true)
            {

                CURL* easyhandle = curl_easy_init();
                std::string readBuffer;

                curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());
                curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 0L);//for debug, for example set to one will output more rather than none ( set to 0 ) to the console
                curl_easy_setopt(easyhandle, CURLOPT_USERAGENT, default_user_agent());

                curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &readBuffer);


#ifdef SKIP_PEER_VERIFICATION
                /*
                 * If you want to connect to a site who isn't using a certificate that is
                 * signed by one of the certs in the CA bundle you have, you can skip the
                 * verification of the server's certificate. This makes the connection
                 * A LOT LESS SECURE.
                 *
                 * If you have a CA cert for the server stored someplace else than in the
                 * default bundle, then the CURLOPT_CAPATH option might come handy for
                 * you.
                 */
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
                /*
                 * If the site you're connecting to uses a different host name that what
                 * they have mentioned in their server certificate's commonName (or
                 * subjectAltName) fields, libcurl will refuse to connect. You can skip
                 * this check, but this will make the connection less secure.
                 */
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

                curl_easy_perform(easyhandle);
                /* always cleanup */
                curl_easy_cleanup(easyhandle);
                return readBuffer;
            }
        }
    private:
        void apx001()
        {
            curl_global_init(CURL_GLOBAL_DEFAULT);
            Libcurl_has_int = true;
        }
    public:
        Libcurl_custom()
        {
            apx001();
        }
        ~Libcurl_custom()
        {
            curl_global_cleanup();
        }
	};
}
#endif