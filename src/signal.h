#pragma once

/*
 * TDS_IGNORE_SIGFPE instructs the signal subsystem to NOT listen for SIGFPE --
 * this lets the game work with a broken OpenAL (seems to be the common culprit right now),
 * however when SIGFPE happens it can be unpredictable and should be managed with caution.
 * Right now the release build will ignore SIGFPE but the debug will still catch it and bail.
 */

void tds_signal_init(void); /* Sets global handlers for signal handling. */
