/**
 * @file ARNETWORK_OSSPECIFIC_Error.c
 * @brief informations about libARNetwork errors
 * @date 04/25/2013
 * @author frederic.dhaeyer@parrot.com
 */

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h>

//
// To generate the function using emacs regexp syntax
// - remove all cases (except for default)
// - copy the content (no braces) of eARNETWORK_OSSPECIFIC_ERROR enum
// - replace rexexp (M-x replace-regexp) with the following parameters
//   - Source: [\ \t]*\([A-Z_]*\)[-0-9\ \t,=]*/\*\*<\ \(.*\)\ \*/
//   - Dest  : case \1:^Jreturn "\2";^J;break;
// - Reindent all lines in file
//
// Note: ^J character is the newline character, C-q C-j in emacs
//
char* ARNETWORK_OSSPECIFIC_Error_ToString (eARNETWORK_OSSPECIFIC_ERROR error)
{
    switch (error)
    {
    case ARNETWORK_OSSPECIFIC_OK:
        return "No error";
        break;
    case ARNETWORK_OSSPECIFIC_ERROR:
        return "Unknown generic error";
        break;
    case ARNETWORK_OSSPECIFIC_ERROR_ALLOC:
        return "Memory allocation error";
        break;
    case ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER:
        return "Bad parameters";
        break;
    case ARNETWORK_OSSPECIFIC_ERROR_MANAGER:
        return "Unknown ARNETWORK_Manager error";
        break;
    default:
        return "Unknown error";
        break;
    }
    return "Unknown error";
}
