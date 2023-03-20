#include "http_client.h"

#include <cstring>

#include <tinyformat.h>

HTTPClient::HTTPClient(std::string url, std::string user, std::string passwd, bool no_proxy)
    : m_curl(curl_easy_init())
    , m_url(std::move(url))
    , m_user(std::move(user))
    , m_passwd(std::move(passwd))
    , m_no_proxy(no_proxy)
{
}

HTTPClient::~HTTPClient()
{
    curl_easy_cleanup(m_curl);
}

std::tuple<int, std::string> HTTPClient::Send(std::string const& buff)
{
    curl_easy_setopt(m_curl, CURLOPT_URL, m_url.c_str());

    curl_slist* header_list;
    header_list = curl_slist_append(nullptr, "Content-Type: application/json-rpc");
    header_list = curl_slist_append(header_list, "Accept: application/json");
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, buff.c_str());
    curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_user.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_passwd.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, buff.size());

    if (m_no_proxy) {
        CURLcode code = curl_easy_setopt(m_curl, CURLOPT_PROXY, "");
        if (code != CURLE_OK) {
            return std::make_tuple(code,
                    tinyformat::format(
                            "curl_easy_setopt CURLOPT_PROXY error: code=%d, %s", code, curl_easy_strerror(code)));
        }
    }

    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HTTPClient::RecvCallback);

    CURLcode code = curl_easy_perform(m_curl);

    curl_slist_free_all(header_list);

    if (code != CURLE_OK) {
        return std::make_tuple(
                code, tinyformat::format("curl returns error: code=%d, %s", code, curl_easy_strerror(code)));
    }

    return std::make_tuple(code, "");
}

Bytes HTTPClient::GetReceivedData() const
{
    return m_recv_data;
}

void HTTPClient::AppendRecvData(char const* ptr, size_t total)
{
    size_t offset = m_recv_data.size();
    m_recv_data.resize(offset + total);
    memcpy(m_recv_data.data() + offset, ptr, total);
}

size_t HTTPClient::RecvCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    HTTPClient* client = reinterpret_cast<HTTPClient*>(userdata);

    size_t total = size * nmemb;
    client->AppendRecvData(ptr, total);
    return total;
}

size_t HTTPClient::SendCallback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    HTTPClient* client = reinterpret_cast<HTTPClient*>(userdata);

    size_t bytes_to_copy = std::min(size * nitems, client->m_send_data.size() - client->m_send_data_offset);
    memcpy(buffer, client->m_send_data.data() + client->m_send_data_offset, bytes_to_copy);
    client->m_send_data_offset += bytes_to_copy;

    return bytes_to_copy;
}
