#if defined(ESP32)
#include "mimetable.h"

//#include "pgmspace.h"
//#include "WString.h"
#include <string>
#include <cstring>  // for strlen, strcmp etc.

namespace mime
{

static const char kHtmlSuffix[]  = ".html";
static const char kHtmSuffix[]  = ".htm";
static const char kTxtSuffix[]  = ".txt";
#ifndef MIMETYPE_MINIMAL
static const char kCssSuffix[]  = ".css";
static const char kJsSuffix[]  = ".js";
static const char kJsonSuffix[]  = ".json";
static const char kPngSuffix[]  = ".png";
static const char kGifSuffix[]  = ".gif";
static const char kJpgSuffix[]  = ".jpg";
static const char kJpegSuffix[]  = ".jpeg";
static const char kIcoSuffix[]  = ".ico";
static const char kSvgSuffix[]  = ".svg";
static const char kTtfSuffix[]  = ".ttf";
static const char kOtfSuffix[]  = ".otf";
static const char kWoffSuffix[]  = ".woff";
static const char kWoff2Suffix[]  = ".woff2";
static const char kEotSuffix[]  = ".eot";
static const char kSfntSuffix[]  = ".sfnt";
static const char kXmlSuffix[]  = ".xml";
static const char kPdfSuffix[]  = ".pdf";
static const char kZipSuffix[]  = ".zip";
static const char kAppcacheSuffix[]  = ".appcache";
#endif // MIMETYPE_MINIMAL
static const char kGzSuffix[]  = ".gz";
static const char kDefaultSuffix[]  = "";

static const char kHtml[]  = "text/html";
static const char kTxt[]  = "text/plain";
#ifndef MIMETYPE_MINIMAL
static const char kCss[]  = "text/css";
static const char kJs[]  = "application/javascript";
static const char kJson[]  = "application/json";
static const char kPng[]  = "image/png";
static const char kGif[]  = "image/gif";
static const char kJpg[]  = "image/jpeg";
static const char kJpeg[]  = "image/jpeg";
static const char kIco[]  = "image/x-icon";
static const char kSvg[]  = "image/svg+xml";
static const char kTtf[]  = "application/x-font-ttf";
static const char kOtf[]  = "application/x-font-opentype";
static const char kWoff[]  = "application/font-woff";
static const char kWoff2[]  = "application/font-woff2";
static const char kEot[]  = "application/vnd.ms-fontobject";
static const char kSfnt[]  = "application/font-sfnt";
static const char kXml[]  = "text/xml";
static const char kPdf[]  = "application/pdf";
static const char kZip[]  = "application/zip";
static const char kAppcache[]  = "text/cache-manifest";
#endif // MIMETYPE_MINIMAL
static const char kGz[]  = "application/x-gzip";
static const char kDefault[]  = "application/octet-stream";

const Entry mimeTable[maxType]  =
{
    { kHtmlSuffix, kHtml },
    { kHtmSuffix, kHtml },
    { kTxtSuffix, kTxt },
#ifndef MIMETYPE_MINIMAL
    { kCssSuffix, kCss },
    { kJsSuffix, kJs },
    { kJsonSuffix, kJson },
    { kPngSuffix, kPng },
    { kGifSuffix, kGif },
    { kJpgSuffix, kJpg },
    { kJpegSuffix, kJpeg },
    { kIcoSuffix, kIco },
    { kSvgSuffix, kSvg },
    { kTtfSuffix, kTtf },
    { kOtfSuffix, kOtf },
    { kWoffSuffix, kWoff },
    { kWoff2Suffix, kWoff2 },
    { kEotSuffix, kEot },
    { kSfntSuffix, kSfnt },
    { kXmlSuffix, kXml },
    { kPdfSuffix, kPdf },
    { kZipSuffix, kZip },
    { kAppcacheSuffix, kAppcache },
#endif // MIMETYPE_MINIMAL
    { kGzSuffix, kGz },
    { kDefaultSuffix, kDefault }
};

    bool endsWith(const std::string& str, const char* suffix) {
        size_t suffixSize = strlen(suffix);
        return str.size() >= suffixSize &&
            str.compare(str.size() - suffixSize, suffixSize, suffix) == 0;
    }

    const char* getContentType(const std::string& path) {
        
        for (size_t i = 0; i < maxType; i++) {
            if (endsWith(path, mimeTable[i].endsWith)) {
                return mimeTable[i].mimeType;
            }
        }
        // Fall-through and just return default type
        return "kDefault";
    }

}
#endif