// Adapter: the ESP32 HTTP client behind the IHttpClient interface.
#include "hal/firmware/adapters.h"

#include <HTTPClient.h>

namespace
{
    class HttpClientTransport : public IHttpClient
    {
    public:
        // Each request opens its own HTTPClient, sends the API key header, and closes the connection after the reply.
        HttpResponse get(const String &url, const String &apiKey) override
        {
            HTTPClient http;
            begin(http, url);
            http.addHeader("DOLAPIKEY", apiKey);
            return perform(http, http.GET());
        }

        HttpResponse post(const String &url, const String &apiKey, const String &body) override
        {
            HTTPClient http;
            begin(http, url);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("DOLAPIKEY", apiKey);
            return perform(http, http.POST(body));
        }

    private:
        // The loop is blocked during a request, so a silent server must not stall it for long
        static constexpr uint16_t TIMEOUT_MS = 5000;

        static void begin(HTTPClient &http, const String &url)
        {
            http.begin(url);
            http.setTimeout(TIMEOUT_MS);
        }

        static HttpResponse perform(HTTPClient &http, int code)
        {
            HttpResponse response;
            response.statusCode = code;
            if (code > 0)
                response.body = http.getString();
            http.end();
            return response;
        }
    };
}

IHttpClient *httpClientAdapter()
{
    static HttpClientTransport client;
    return &client;
}
