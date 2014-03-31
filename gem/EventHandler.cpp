/*
 * File:   EventHandler.cpp
 * Author: Richard Greene
 * Implements a handler that detects events on file descriptors.
 * Created on March 31, 2014, 1:49 PM
 */

#include <EventHandler.h>

// needs:
// constructor that initializes file descriptors (incl interrupt handlers and FIFOs for status & errors)
// needed for various events,
// and lists of subscribers
// destructor that closes all files (and perh removes temp ones used for pipes?)
// method for subscribers to add their subscriptions
// method that starts handling, with epoll in loop & 10ms sleep, that calls all the subscribers for each event
// UTs, eg to test that if status is written twice, we always read the most recent one