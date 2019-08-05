#include <ncurses.h>
#include <poll.h>
#include <thread>
#include <sys/stat.h> // checking if file/dir exist
#include <fstream>
#include <string>

#include "opendlv-standard-message-set.hpp"
#include "automower_hrp.json"

auto 
