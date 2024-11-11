//
//  config.hpp
//  file scanner
//
//  Created by Evan Carew on 9/28/24.
//

#ifndef config_hpp
#define config_hpp
#include <string>
#include <stdio.h>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace std;
#endif /* config_hpp */
void setupMenuDescriptions(po::options_description &, string &, string &, string &, string &);
void setupMenuDescriptionsC(po::options_description &, string &, string &, string &, string &, string &);
