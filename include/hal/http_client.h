#pragma once
#include <Arduino.h>

struct HttpResponse
{
    // HTTP status code, or a value <= 0 when the request could not be performed.
    int statusCode = -1;
    String body;
};

// Hardware abstraction for the HTTP transport used to call the Dolibarr API.
class IHttpClient
{
public:
    virtual ~IHttpClient() = default;

    virtual HttpResponse get(const String &url, const String &apiKey) = 0;
    virtual HttpResponse post(const String &url, const String &apiKey, const String &body) = 0;
};
