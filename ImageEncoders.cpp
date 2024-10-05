#include "ImageEncoders.h"
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Project: Turtleizer_CPP (static C++ library for Windows)
 *
 * Object class, roughly representing sort of factory pattern for image codecs
 * in a winapi enivronment.
 *
 * @author William Sherif
 * @author Kay Gürtzig
 * Version: 11.0.1 (covering capabilities of Structorizer 3.30-12, functional GUI)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2024-10-04   wcstok signature adapted, STATUS_TEXTS initialisation updated
 * 2021-04-21   Converted into a class (h+cpp), UNICODE adaptation fixed
 * 2016-04-20   created by William Sherif (https://gist.github.com/superwills/2f98fc72f07e61f9c04e56036a29f4b3)
 */

const TCHAR* ImageEncoders::STATUS_TEXTS[] = {
  TEXT("Ok: The method call was successful."),
  TEXT("GenericError: There was an error on the method call, which is identified as something other than those defined by the other elements of this enumeration."),
  TEXT("InvalidParameter: One of the arguments passed to the method was not valid."),
  TEXT("OutOfMemory: The operating system is out of memory and could not allocate memory to process the method call. For an explanation of how constructors use the OutOfMemory status, see the Remarks section at the end of this topic."),
  TEXT("ObjectBusy: One of the arguments specified in the API call is already in use in another thread."),
  TEXT("InsufficientBuffer: A buffer specified as an argument in the API call is not large enough to hold the data to be received."),
  TEXT("NotImplemented: The method is not implemented."),
  TEXT("Win32Error: The method generated a Win32 error."),
  TEXT("WrongState: The object is in an invalid state to satisfy the API call. For example, calling Pen::GetColor from a pen that is not a single, solid color results in a WrongState status."),
  TEXT("Aborted: Indicates The method was aborted."),
  TEXT("FileNotFound: The specified image file or metafile cannot be found."),
  TEXT("ValueOverflow: The method performed an arithmetic operation that produced a numeric overflow."),
  TEXT("AccessDenied: A write operation is not allowed on the specified file."),
  TEXT("UnknownImageFormat: The specified image file format is not known."),
  TEXT("FontFamilyNotFound: The specified font family cannot be found. Either the font family name is incorrect or the font family is not installed."),
  TEXT("FontStyleNotFound: The specified style is not available for the specified font family."),
  TEXT("NotTrueTypeFont: The font retrieved from an HDC or LOGFONT is not a TrueType font and cannot be used with GDI+."),
  TEXT("UnsupportedGdiplusVersion: The version of GDI+ that is installed on the system is incompatible with the version with which the application was compiled."),
  TEXT("GdiplusNotInitialized: The GDI+ API is not in an initialized state. To function, all GDI+ objects require that GDI+ be in an initialized state. Initialize GDI+ by calling GdiplusStartup."),
  TEXT("PropertyNotFound: The specified property does not exist in the image."),
  TEXT("PropertyNotSupported: The specified property is not supported by the format of the image and, therefore, cannot be set."),
  TEXT("ProfileNotFound: The color profile required to save an image in CMYK format was not found."),
  TEXT("INVALID STATUS CODE")
};

ImageEncoders::ImageEncoders()
    : NumberOfEncoders(0)
    , byteSize(0)
    , imageCodecs(nullptr)
{
    // How many encoders do we have on the system?
    Gdiplus::GetImageEncodersSize(&NumberOfEncoders, &byteSize);
    if (!byteSize || !NumberOfEncoders)
    {
        error(TEXT("ERROR: There are no image encoders available, num=%d, size=%d"),
            NumberOfEncoders, byteSize);
        return;
    }

    // Allocate space to get the ImageCodeInfo descriptor for each codec.
    imageCodecs = (Gdiplus::ImageCodecInfo*)malloc(byteSize);
    if (imageCodecs != nullptr) {
        Gdiplus::GetImageEncoders(NumberOfEncoders, byteSize, imageCodecs);

#if DEBUG_PRINT
        wprintf(TEXT("CODECS:\n"));
        // Print the codecs we know
        for (int i = 0; i < NumberOfEncoders; i++)
        {
            wprintf(TEXT("  * Codec %d = Ext:%s Description:%s\n"),
                i, imageCodecs[i].FilenameExtension, imageCodecs[i].FormatDescription);
        }
#endif /*DEBUG_PRINT*/ 
    }
    else {
        NumberOfEncoders = 0;
#if DEBUG_PRINT
        wprintf(TEXT("Codecs could not be retrieved: Out of memory!\n"));
#endif /*DEBUG_PRINT*/ 
    }
}

ImageEncoders::~ImageEncoders()
{
    if (imageCodecs != nullptr) {
        free(imageCodecs);
    }
}

bool ImageEncoders::InFileTypesList(const TCHAR* ext, const TCHAR* filetypesList)
{
    const TCHAR* delimiters = TEXT("*.;");
#ifdef UNICODE
    TCHAR* wcstokBuf = NULL;
    TCHAR* dup = _wcsdup(filetypesList); // We have to form a writeable copy of the FileTypesList
#pragma warning(suppress : 4996)
    TCHAR* tok = wcstok(dup, delimiters, &wcstokBuf); // 1st call is on dup, subsequent on NULL.
#else
    TCHAR* dup = strdup(filetypesList); // We have to form a writeable copy of the FileTypesList
    TCHAR* tok = strtok(dup, delimiters); // 1st call is on dup, subsequent on NULL.
#endif /*UNICODE*/
    // So we want this call outside the while loop anyway.
    bool in = 0;
    do
    {
#ifdef UNICODE
        if (!_wcsicmp(ext, tok))
#else
        if (!_stricmp(ext, tok))
#endif /*UNICODE*/
        in = 1;
        else  // Pull the next token
             // wcstok/strtok retains state, so we pass NULL to use last tokenized string
#ifdef UNICODE
#pragma warning(suppress : 4996)
            tok = wcstok(NULL, delimiters, &wcstokBuf);
#else
            tok = strtok(NULL, delimiters);
#endif /*UNICODE*/
    } while (tok && !in);
    free(dup); // release the manipulatable duplicate.
    return in;
}

CLSID ImageEncoders::GetCLSIDForExtension(const TCHAR* ext)
{
    CLSID clsid = CLSID_NULL; // Start with assuming invalid clsid.

    // Use a case-insensitive comparison
    for (unsigned int i = 0; i < NumberOfEncoders; i++)
    {
        if (InFileTypesList(ext, imageCodecs[i].FilenameExtension))
        {
            wprintf(TEXT("%s is type %s\n"), ext, imageCodecs[i].FormatDescription);
            clsid = imageCodecs[i].Clsid; // Found a CLSID for this extension
            break;
        }
    }

    return clsid;
}

CLSID ImageEncoders::GetCLSIDByMime(const TCHAR* mimetype)
{
    CLSID clsid = CLSID_NULL;
    for (unsigned int i = 0; i < NumberOfEncoders; i++)
    {
        // Straight comparison with listed mime type.
        if (!_wcsicmp(mimetype, imageCodecs[i].MimeType))
        {
            clsid = imageCodecs[i].Clsid;
            break;
        }
    }
    return clsid;
}

bool ImageEncoders::Save(Gdiplus::Image* im, TCHAR* filename)
{
    ImageEncoders encoders;

    // Extract the extension
    TCHAR* dotLocation = wcsrchr(filename, TEXT('.'));
    if (dotLocation) // found.
    {
        CLSID clsid = encoders.GetCLSIDForExtension(dotLocation + 1);
        if (clsid != CLSID_NULL)
        {
            Gdiplus::Status status = im->Save(filename, &clsid);
            if (status != Gdiplus::Status::Ok)
            {
                error(TEXT("ImageEncoders::Save( %s ): Failed to save: %s"), filename, getStatusString(status));
                return 0;
            }
#if DEBUG_PRINT
            else {
                wprintf(TEXT("%s saved successfully\n"), filename);
            }
#endif /*DEBUG_PRINT*/
            return 1;
        }
    }

    error(TEXT("ImageEncoders::Save( %s ): Failed to save; invalid extension"), filename);
    return 0;
}


void ImageEncoders::showMessage(const TCHAR* fmt, const TCHAR* title, int options, va_list args)
{
	TCHAR buf[1024];
	int index = wvsprintf(buf, fmt, args);
	buf[index] = '\n';
	buf[index + 1] = 0;
	wprintf(buf);
	MessageBox(HWND_DESKTOP, buf, title, options);
}

void ImageEncoders::info(const TCHAR* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	showMessage(fmt, TEXT("Info"), MB_OK | MB_ICONINFORMATION, list);
}

void ImageEncoders::error(const TCHAR* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	showMessage(fmt, TEXT("Error"), MB_OK | MB_ICONERROR, list);
}

const TCHAR* ImageEncoders::getStatusString(Gdiplus::Status status)
{
    if (status < 0 || status > Gdiplus::Status::PropertyNotSupported + 1) // ProfileNotFound may not be there
        status = (Gdiplus::Status)(Gdiplus::Status::PropertyNotSupported + 2); // gives last error (INVALID STATUS CODE)

    return STATUS_TEXTS[status];
}
