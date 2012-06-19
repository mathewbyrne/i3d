/* $Id: capture.h,v 1.1 2006/03/19 14:31:14 mbyrne Exp $
 * Alex Holkner, aholkner@cs.rmit.edu.au
 */

#ifndef CAPTURE_H
#define CAPTURE_H

/* Capture the current GL color buffer to the specified file, in
 * PNG format.  Returns 1 on success, 0 on failure.  Error messages
 * are written to stderr.  Assumes RGB 24-bit color buffer.  Example:
 *
 *   capture("screenshot.png");
 */
int capture(const char *filename);

/* A convenience function for capturing multiple frames in an 
 * animation.  Names each file as <prefix><frame>.png, where frame
 * is incremented each time the function is called.  For example,
 * calling:
 *
 *   captureFrame("frame");
 *
 * several times creates the files:
 *
 *   frame0001.png
 *   frame0002.png
 *   frame0003.png
 *   etc.
 */
int captureFrame(const char *prefix);

#endif
