/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   bgcyberdisto_png;
    const int            bgcyberdisto_pngSize = 243589;

    extern const char*   cross_png;
    const int            cross_pngSize = 3643;

    extern const char*   disk_png;
    const int            disk_pngSize = 3320;

    extern const char*   extraterra_png;
    const int            extraterra_pngSize = 20317;

    extern const char*   logo_png;
    const int            logo_pngSize = 247747;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
