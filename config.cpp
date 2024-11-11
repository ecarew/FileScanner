//
//  config.cpp
//  file scanner
//
//  Created by Evan Carew on 9/28/24.
//

#include "config.hpp"

//options that will be
// allowed only on command line
void setupMenuDescriptions(po::options_description &gen, string &cf, string &pd, string &lr, string &sp){
    gen.add_options()
    ("version,v", "print version string")
    ("help,h", "produce help message")
    ("config,c", po::value<string>(&cf)->default_value("scanner.cfg"),
     "name of a file of a configuration.")
    ("dbfile,db", po::value<string>(&pd)->default_value("persist_db"),
     "name of sqlite3 persistent file object db.")
    ("initdb", "initialize persistent file for initial use? (y/n).")
    ("lreport,l", po::value<string>(&lr)->default_value("n"),
     "Long report format output.")
    ("remove,r", "remove over stale archive files.")
    ("archive,a", "Archive files in encrypted tar files. Password from config file\nor command line used to encrypt with openssl tools command line.")
 //   ("scanpath,p", po::value<string>(&sp),
 //    "Path to recursively scan")
    ;
}
//options that will be
// allowed both on command line and in
// config file
void setupMenuDescriptionsC(po::options_description &conf, string &pd, string &sr, string &lr, string &sp, string &pw){
    conf.add_options()
    ("dbfile,db", po::value<string>(&pd)->default_value("persist_db"),
     "name of sqlite3 persistent file object db.")
    ("sreport,s", po::value<string>(&sr)->default_value("n"),
    "Short report format output y/n")
    ("lreport,l", po::value<string>(&lr)->default_value("n"),
     "Long report format output.")
    ("passwd,e", po::value<string>(&pw)->default_value("base"), "The password used to encrypt the tar archive files for stale directories.")
    ("scanpath,p", po::value<string>(&sp), "Path to recursively scan")
    ;
}
