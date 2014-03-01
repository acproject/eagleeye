#include "Print.h"
#include <algorithm>
#include <cstdlib>
#include <cctype> // for toupper
#include <string>
#include <iostream>

#if defined WIN32
# include <windows.h>
#endif

#ifndef _MSC_VER
# define COMMON_LVB_UNDERSCORE    0
# define COMMON_LVB_REVERSE_VIDEO 0
#endif

namespace eagleeye
{
DWORD convertAttributesColor (int attribute, int fg, int bg=0)
{
	static DWORD w_attributes[7]  = { 0,                        // TT_RESET
		FOREGROUND_INTENSITY ,    // TT_BRIGHT
		0,                        // TT_DIM
		COMMON_LVB_UNDERSCORE,    // TT_UNDERLINE
		0,                        // TT_BLINK
		COMMON_LVB_REVERSE_VIDEO, // TT_REVERSE
		0                         // TT_HIDDEN
	};
	static DWORD w_fg_colors[8]  = { 0,                                                  // TT_BLACK
		FOREGROUND_RED,                                     // TT_RED
		FOREGROUND_GREEN ,                                  // TT_GREEN
		FOREGROUND_GREEN | FOREGROUND_RED ,                 // TT_YELLOW
		FOREGROUND_BLUE ,                                   // TT_BLUE
		FOREGROUND_RED | FOREGROUND_BLUE ,                  // TT_MAGENTA
		FOREGROUND_GREEN | FOREGROUND_BLUE,                 // TT_CYAN
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED // TT_WHITE
	};
	static DWORD w_bg_colors[8]  = { 0,                                                  // TT_BLACK
		BACKGROUND_RED,                                     // TT_RED
		BACKGROUND_GREEN ,                                  // TT_GREEN
		BACKGROUND_GREEN | BACKGROUND_BLUE ,                // TT_YELLOW
		BACKGROUND_BLUE ,                                   // TT_BLUE
		BACKGROUND_RED | BACKGROUND_BLUE ,                  // TT_MAGENTA
		BACKGROUND_GREEN | BACKGROUND_BLUE,                 // TT_CYAN
		BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED // TT_WHITE
	};

	return w_attributes[attribute] | w_fg_colors[fg] | w_bg_colors[bg];
}

////////////////////////////////////////////////////////////////////////////////
void change_text_color (FILE *stream, int attribute, int fg, int bg)
{
#ifdef WIN32
	HANDLE h = GetStdHandle ((stream == stdout) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	SetConsoleTextAttribute (h, WORD(convertAttributesColor (attribute, fg, bg)));
#else
	char command[13];
	// Command is the control command to the terminal
	sprintf (command, "%c[%d;%d;%dm", 0x1B, attribute, fg + 30, bg + 40);
	fprintf (stream, "%s", command);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void change_text_color (FILE *stream, int attribute, int fg)
{
#ifdef WIN32
	HANDLE h = GetStdHandle ((stream == stdout) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	SetConsoleTextAttribute (h, WORD(convertAttributesColor (attribute, fg)));
#else
	char command[13];
	// Command is the control command to the terminal
	sprintf (command, "%c[%d;%dm", 0x1B, attribute, fg + 30);
	fprintf (stream, "%s", command);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void reset_text_color (FILE *stream)
{
#ifdef WIN32
	HANDLE h = GetStdHandle ((stream == stdout) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	SetConsoleTextAttribute (h, WORD(convertAttributesColor (0, TT_WHITE, TT_BLACK)));
#else
	char command[13];
	// Command is the control command to the terminal
	sprintf (command, "%c[0;m", 0x1B);
	fprintf (stream, "%s", command);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void print_color (FILE *stream, int attr, int fg, const char *format, ...)
{
	change_text_color (stream, attr, fg);
	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
void print_info (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_INFO)) return; 

	reset_text_color (stdout);

	va_list ap;

	va_start (ap, format);
	vfprintf (stdout, format, ap);
	va_end (ap);
}

////////////////////////////////////////////////////////////////////////////////
void print_info (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_INFO)) return;

	reset_text_color (stream);

	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);
}

////////////////////////////////////////////////////////////////////////////////
void print_highlight (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ALWAYS)) return;

	change_text_color (stdout, TT_BRIGHT, TT_GREEN);
	fprintf (stdout, "> ");
	reset_text_color (stdout);

	va_list ap;

	va_start (ap, format);
	vfprintf (stdout, format, ap);
	va_end (ap);
}

////////////////////////////////////////////////////////////////////////////////
void print_highlight (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ALWAYS)) return;

	change_text_color (stream, TT_BRIGHT, TT_GREEN);
	fprintf (stream, "> ");
	reset_text_color (stream);

	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);
}

////////////////////////////////////////////////////////////////////////////////
void print_error (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ERROR)) return;

	change_text_color (stderr, TT_BRIGHT, TT_RED);
	va_list ap;

	va_start (ap, format);
	vfprintf (stderr, format, ap);
	va_end (ap);

	reset_text_color (stderr);
}

////////////////////////////////////////////////////////////////////////////////
void print_error (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ERROR)) return;

	change_text_color (stream, TT_BRIGHT, TT_RED);
	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
void print_warn (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_WARN)) return;

	change_text_color (stderr, TT_BRIGHT, TT_YELLOW);
	va_list ap;

	va_start (ap, format);
	vfprintf (stderr, format, ap);
	va_end (ap);

	reset_text_color (stderr);
}

////////////////////////////////////////////////////////////////////////////////
void print_warn (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_WARN)) return;

	change_text_color (stream, TT_BRIGHT, TT_YELLOW);
	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
void print_value (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ALWAYS)) return;

	change_text_color (stdout, TT_RESET, TT_CYAN);
	va_list ap;

	va_start (ap, format);
	vfprintf (stdout, format, ap);
	va_end (ap);

	reset_text_color (stdout);
}

////////////////////////////////////////////////////////////////////////////////
void print_value (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_ALWAYS)) return;

	change_text_color (stream, TT_RESET, TT_CYAN);
	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
void print_debug (const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_DEBUG)) return;

	change_text_color (stdout, TT_RESET, TT_GREEN);
	va_list ap;

	va_start (ap, format);
	vfprintf (stdout, format, ap);
	va_end (ap);

	reset_text_color (stdout);
}

////////////////////////////////////////////////////////////////////////////////
void print_debug (FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (L_DEBUG)) return;

	change_text_color (stream, TT_RESET, TT_GREEN);
	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
static bool s_need_verbosity_init = false;
static VERBOSITY_LEVEL s_verbosity_level = L_VERBOSE;
////////////////////////////////////////////////////////////////////////////////
void setVerbosityLevel (VERBOSITY_LEVEL level)
{
	if (s_need_verbosity_init) initVerbosityLevel ();
	s_verbosity_level = level;
}

////////////////////////////////////////////////////////////////////////////////
VERBOSITY_LEVEL getVerbosityLevel ()
{
	if (s_need_verbosity_init) initVerbosityLevel ();
	return s_verbosity_level;
}

////////////////////////////////////////////////////////////////////////////////
bool isVerbosityLevelEnabled (VERBOSITY_LEVEL level)
{
	if (s_need_verbosity_init) initVerbosityLevel ();
	return level <= s_verbosity_level;  
}

////////////////////////////////////////////////////////////////////////////////
bool initVerbosityLevel ()
{
	s_verbosity_level = L_INFO; // Default value
	char* vi_verbosity_level = getenv ( "VI_VERBOSITY_LEVEL");
	if (vi_verbosity_level)
	{
		std::string s_vi_verbosity_level (vi_verbosity_level);
		std::transform (s_vi_verbosity_level.begin (), s_vi_verbosity_level.end (), s_vi_verbosity_level.begin (), toupper);

		if (s_vi_verbosity_level.find ("ALWAYS") != std::string::npos)          s_verbosity_level = L_ALWAYS;
		else if (s_vi_verbosity_level.find ("ERROR") != std::string::npos)      s_verbosity_level = L_ERROR;
		else if (s_vi_verbosity_level.find ("WARN") != std::string::npos)       s_verbosity_level = L_WARN;
		else if (s_vi_verbosity_level.find ("INFO") != std::string::npos)       s_verbosity_level = L_INFO;
		else if (s_vi_verbosity_level.find ("DEBUG") != std::string::npos)      s_verbosity_level = L_DEBUG;
		else if (s_vi_verbosity_level.find ("VERBOSE") != std::string::npos)    s_verbosity_level = L_VERBOSE;
		else std::cout << "Warning: invalid VI_VERBOSITY_LEVEL set (" << s_vi_verbosity_level << ")" << std::endl;
	}

	s_need_verbosity_init = false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void print (VERBOSITY_LEVEL level, FILE *stream, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (level)) return;
	switch (level)
	{
	case L_DEBUG:
		change_text_color (stream, TT_RESET, TT_GREEN);
		break;
	case L_WARN:
		change_text_color (stream, TT_BRIGHT, TT_YELLOW);
		break;
	case L_ERROR:
		change_text_color (stream, TT_BRIGHT, TT_RED);
		break;
	case L_ALWAYS:
	case L_INFO:
	case L_VERBOSE:
		change_text_color(stream,TT_BRIGHT,TT_WHITE);
		break;
	default:
		break;
	}

	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

////////////////////////////////////////////////////////////////////////////////
void print (VERBOSITY_LEVEL level, const char *format, ...)
{
	if (!isVerbosityLevelEnabled (level)) return;
	FILE *stream = (level == L_WARN || level == L_ERROR) ? stderr : stdout;
	switch (level)
	{
	case L_DEBUG:
		change_text_color (stream, TT_RESET, TT_GREEN);
		break;
	case L_WARN:
		change_text_color (stream, TT_BRIGHT, TT_YELLOW);
		break;
	case L_ERROR:
		change_text_color (stream, TT_BRIGHT, TT_RED);
		break;
	case L_ALWAYS:
	case L_INFO:
	case L_VERBOSE:
		change_text_color(stream,TT_BRIGHT,TT_WHITE);
		break;
	default:
		break;
	}

	va_list ap;

	va_start (ap, format);
	vfprintf (stream, format, ap);
	va_end (ap);

	reset_text_color (stream);
}

void print_elapsed_time()
{
	static bool first_call = true;
	static unsigned long last_time = 0;

	if (first_call)
	{
		//record time
		last_time = clock();
		first_call = false;
	}
	else
	{
		unsigned long now_time = clock();
		float elapsed_time = (float)(now_time - last_time) / CLOCKS_PER_SEC;

		//display time
		change_text_color (stdout, TT_BRIGHT, TT_YELLOW);

		fprintf(stdout,"elapsed time %f s \n",elapsed_time);
		first_call = true;

		reset_text_color (stdout);
	}
}
}
