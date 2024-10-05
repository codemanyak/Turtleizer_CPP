#pragma once
#ifndef IMAGEENCODERS_H
#define IMAGEENCODERS_H
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
 * 2024-10-04   String types declared const where necessary for initialisation with literals
 * 2021-04-21   Converted into a class (h+cpp), UNICODE adaptation fixed
 * 2016-04-20   created by William Sherif (https://gist.github.com/superwills/2f98fc72f07e61f9c04e56036a29f4b3)
 */

#include <windows.h>
#include <gdiplus.h>
#include <cstdio>

#define DEBUG_PRINT 0

struct ImageEncoders
{
public:
    //enum ImageFormat { BMP, JPG, GIF, TIF, PNG };
    ImageEncoders();
    ~ImageEncoders();

    // Checks that extension ext is member of the filetypeslist
    // 
    // @param ext - An assumed file type extension
    // @param filetypesList - File types lists look like "*.jpg;*.jpeg;*.jfif"
    // @returns whether ext is in the list
    bool InFileTypesList(const TCHAR* ext, const TCHAR* filetypesList);

    // Retrieves the guid of the codec for the given file type extension ext
    // 
    // @param ext - a file type extension
    CLSID GetCLSIDForExtension(const TCHAR* ext);

    // Retrieves the guid of the codec from the given MIME type mimetype 
    CLSID GetCLSIDByMime(const TCHAR* mimetype);

    // Saves the given image to the file with given filename using the code
    // associated to filename's extension
    //
    // @param im - pointer to the Image to be saved
    // @param filename - the target file path
    static bool Save(Gdiplus::Image* im, TCHAR* filename);

private:
    static const TCHAR* STATUS_TEXTS[];
    UINT byteSize;  // byteSize of encoders on system
    UINT NumberOfEncoders; // number of encoders on system
    Gdiplus::ImageCodecInfo* imageCodecs;   // singleton

    // Adapter method showing a message box with text composed from format string and the
    // value arguments args, regarding the given options (like MB_OKCANCEL | MB_ICONWARNING)
    //
    // @param fmt - a format string as for printf or wprintf
    // @param title - the message box title string
    // @param options - a combination of the uType codes of the winapi function MessageBox
    // @param args - the values to be inserted into the format string fmt 
    static void showMessage(const TCHAR* fmt, const TCHAR* title, int options, va_list args);

    // Convenience wrapper for showMessage with error title and error icon
    static void error(const TCHAR* fmt, ...);

    // Convenience wrapper for showMessage with info title and info icon
    static void info(const TCHAR* fmt, ...);

    // Retrieves the appropriate text for the given GDI+ status value
    // 
    // @param status - a GDI+ status id
    // @returns the pointer to a verbose status description (in English)
    static const TCHAR* getStatusString(Gdiplus::Status status);

};

#endif /*IMAGEENCODERS_H*/