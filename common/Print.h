#ifndef _TERMINAL_TOOLS_PRINT_H_
#define _TERMINAL_TOOLS_PRINT_H_

#include "EagleeyeMacro.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>


#define EAGLEEYE_ALWAYS(...)  eagleeye::print (eagleeye::L_ALWAYS, __VA_ARGS__)
#define EAGLEEYE_ERROR(...)   eagleeye::print (eagleeye::L_ERROR, __VA_ARGS__)
#define EAGLEEYE_WARN(...)    eagleeye::print (eagleeye::L_WARN, __VA_ARGS__)
#define EAGLEEYE_INFO(...)    eagleeye::print (eagleeye::L_INFO, __VA_ARGS__)
#define EAGLEEYE_DEBUG(...)   eagleeye::print (eagleeye::L_DEBUG, __VA_ARGS__)
#define EAGLEEYE_VERBOSE(...) eagleeye::print (eagleeye::L_VERBOSE, __VA_ARGS__)

namespace eagleeye
{
	enum TT_ATTIBUTES
	{
		TT_RESET     = 0,
		TT_BRIGHT    = 1,
		TT_DIM       = 2,
		TT_UNDERLINE = 3,
		TT_BLINK     = 4,
		TT_REVERSE   = 7,
		TT_HIDDEN    = 8
	};

	enum TT_COLORS
	{
		TT_BLACK,
		TT_RED,
		TT_GREEN,
		TT_YELLOW,
		TT_BLUE,
		TT_MAGENTA,
		TT_CYAN,
		TT_WHITE
	};

	enum VERBOSITY_LEVEL
	{
		L_ALWAYS,
		L_ERROR,
		L_WARN,
		L_INFO,
		L_DEBUG,
		L_VERBOSE
	};

	/** set the verbosity level */
	EAGLEEYE_API void setVerbosityLevel (VERBOSITY_LEVEL level);

	/** get the verbosity level. */
	EAGLEEYE_API VERBOSITY_LEVEL 	getVerbosityLevel ();

	/** initialize verbosity level. */
	EAGLEEYE_API bool initVerbosityLevel ();

	/** is verbosity level enabled? */
	EAGLEEYE_API bool isVerbosityLevelEnabled (VERBOSITY_LEVEL severity);

	/** \brief Change the text color (on either stdout or stderr) with an attr:fg:bg
	* \param stream the output stream (stdout, stderr, etc)
	* \param attribute the text attribute
	* \param fg the foreground color
	* \param bg the background color
	*/
	EAGLEEYE_API void change_text_color (FILE *stream, int attribute, int fg, int bg);

	/** \brief Change the text color (on either stdout or stderr) with an attr:fg
	* \param stream the output stream (stdout, stderr, etc)
	* \param attribute the text attribute
	* \param fg the foreground color
	*/
	EAGLEEYE_API void change_text_color (FILE *stream, int attribute, int fg);

	/** \brief Reset the text color (on either stdout or stderr) to its original state
	* \param stream the output stream (stdout, stderr, etc)
	*/
	EAGLEEYE_API void reset_text_color (FILE *stream);

	/** \brief Print a message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param attr the text attribute
	* \param fg the foreground color
	* \param format the message
	*/
	EAGLEEYE_API void print_color (FILE *stream, int attr, int fg, const char *format, ...);

	/** \brief Print an info message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_info  (const char *format, ...);

	/** \brief Print an info message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_info  (FILE *stream, const char *format, ...);

	/** \brief Print a highlighted info message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_highlight  (const char *format, ...);

	/** \brief Print a highlighted info message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_highlight  (FILE *stream, const char *format, ...);

	/** \brief Print an error message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_error (const char *format, ...);

	/** \brief Print an error message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_error (FILE *stream, const char *format, ...);

	/** \brief Print a warning message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_warn (const char *format, ...);

	/** \brief Print a warning message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_warn (FILE *stream, const char *format, ...);

	/** \brief Print a debug message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_debug (const char *format, ...);

	/** \brief Print a debug message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_debug (FILE *stream, const char *format, ...);


	/** \brief Print a value message on stream with colors
	* \param format the message
	*/
	EAGLEEYE_API void print_value (const char *format, ...);

	/** \brief Print a value message on stream with colors
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print_value (FILE *stream, const char *format, ...);

	/**
	 *	\brief Print elapsed time
	 *	\param stream the output stream (stdout,stderr,etc)
	 *	\note we have to call twice.
	 */
	EAGLEEYE_API void print_elapsed_time();

	/** \brief Print a message on stream
	* \param level the verbosity level
	* \param stream the output stream (stdout, stderr, etc)
	* \param format the message
	*/
	EAGLEEYE_API void print (VERBOSITY_LEVEL level, FILE *stream, const char *format, ...);

	/** \brief Print a message
	* \param level the verbosity level
	* \param format the message
	*/
	EAGLEEYE_API void print (VERBOSITY_LEVEL level, const char *format, ...);
}
#endif